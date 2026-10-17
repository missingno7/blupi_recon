#!/usr/bin/env python3
"""Search conservative VC5 source variants for one function in its real TU."""

from __future__ import annotations

import argparse
import concurrent.futures
import difflib
from functools import lru_cache
import hashlib
import json
import os
from pathlib import Path
import random
import re
import subprocess
import sys
from typing import Callable

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import asmscore  # noqa: E402
import msvc  # noqa: E402

TOKEN_MASK = re.compile(r'//[^\r\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'')
SIMPLE = r"[A-Za-z_]\w*(?:(?:\.|->)[A-Za-z_]\w*|\[[0-9]+\])*"
ATOM = r"(?:[A-Za-z_]\w*(?:(?:\.|->)[A-Za-z_]\w*)*|-?[0-9]+)"
CHAIN_EXPR = r"[A-Za-z_]\w*(?:(?:\.|->)[A-Za-z_]\w*|\[[A-Za-z_]\w*\])*"
SCALAR = r"(?:int|short|long|char|BOOL|BYTE|WORD|DWORD|float|double)"
LOCAL_TYPE = r"(?:const\s+)?(?:unsigned\s+|signed\s+)?[A-Za-z_]\w*(?:::[A-Za-z_]\w*)?(?:\s*\*+)?"


def mask(text: str) -> str:
    return TOKEN_MASK.sub(lambda m: re.sub(r"[^\r\n]", " ", m.group()), text)


def matching(text: str, start: int, left: str, right: str) -> int | None:
    depth = 0
    for pos in range(start, len(text)):
        if text[pos] == left:
            depth += 1
        elif text[pos] == right:
            depth -= 1
            if depth == 0:
                return pos
    return None


def symbol_parts(decorated: str) -> tuple[str, str | None]:
    match = re.match(r"\?([^@]+)(?:@([^@]+))?@?", decorated)
    if not match:
        raise ValueError(f"cannot derive C++ function name from {decorated!r}")
    return match.group(1), match.group(2)


def locate_body(source: str, decorated: str) -> tuple[int, int]:
    name, owner = symbol_parts(decorated)
    clean = mask(source)
    if owner:
        pattern = re.compile(r"\b" + re.escape(owner) + r"\s*::\s*" + re.escape(name) + r"\s*\(")
    else:
        pattern = re.compile(r"\b" + re.escape(name) + r"\s*\(")
    found = []
    for item in pattern.finditer(clean):
        open_paren = clean.find("(", item.start(), item.end())
        close_paren = matching(clean, open_paren, "(", ")")
        if close_paren is None:
            continue
        cursor = close_paren + 1
        while cursor < len(clean) and clean[cursor].isspace():
            cursor += 1
        # Ignore calls in control conditions and declarations without a body.
        line_start = clean.rfind("\n", 0, item.start()) + 1
        prefix = clean[line_start:item.start()]
        if re.search(r"\b(if|while|for|switch|return|case)\b", prefix):
            continue
        if cursor >= len(clean) or clean[cursor] != "{":
            continue
        close_body = matching(clean, cursor, "{", "}")
        if close_body is not None:
            found.append((cursor, close_body + 1))
    if len(found) != 1:
        raise ValueError(f"expected one definition for {decorated}, found {len(found)}")
    return found[0]


def _replace(source: str, start: int, end: int, value: str) -> str:
    return source[:start] + value + source[end:]


@lru_cache(maxsize=1)
def header_field_types() -> dict[str, set[str]]:
    result: dict[str, set[str]] = {}
    decl = re.compile(r"(?m)^[ \t]*(?P<t>(?:const\s+)?(?:unsigned\s+|signed\s+)?[A-Za-z_]\w*(?:::[A-Za-z_]\w*)?(?:\s*\*+)?)\s+(?P<names>[A-Za-z_]\w*(?:\s*\[[^\]\r\n]*\])?(?:\s*,\s*[A-Za-z_]\w*(?:\s*\[[^\]\r\n]*\])?)*)\s*;")
    for path in (ROOT / "src").glob("*.h"):
        text = mask(path.read_bytes().decode("cp1252"))
        for item in decl.finditer(text):
            typ = re.sub(r"\s+", " ", item.group("t")).strip()
            for name in re.findall(r"[A-Za-z_]\w*", item.group("names")):
                result.setdefault(name, set()).add(typ)
    return result


def infer_type(expression: str, local_types: dict[str, str]) -> str | None:
    fields = header_field_types()
    segments = re.split(r"(?:->|\.)", expression)
    names = [re.match(r"[A-Za-z_]\w*", part) for part in segments]
    if not names or any(name is None for name in names):
        return None
    base = names[0].group()
    current = local_types.get(base)
    if current is None:
        options = fields.get(base, set())
        current = next(iter(options)) if len(options) == 1 else None
    for name in names[1:]:
        options = fields.get(name.group(), set())
        current = next(iter(options)) if len(options) == 1 else None
    if current is None:
        return None
    return current.replace("const ", "").strip()


def _match_if_blocks(text: str):
    clean = mask(text)
    for item in re.finditer(r"\bif\s*\(", clean):
        p = clean.find("(", item.start(), item.end())
        q = matching(clean, p, "(", ")")
        if q is None:
            continue
        first = q + 1
        while first < len(clean) and clean[first].isspace():
            first += 1
        if first >= len(clean) or clean[first] != "{":
            continue
        first_end = matching(clean, first, "{", "}")
        if first_end is None:
            continue
        after = first_end + 1
        while after < len(clean) and clean[after].isspace():
            after += 1
        if not clean.startswith("else", after) or (after + 4 < len(clean) and (clean[after + 4].isalnum() or clean[after + 4] == "_")):
            continue
        second = after + 4
        while second < len(clean) and clean[second].isspace():
            second += 1
        if clean.startswith("if", second):
            continue
        if second >= len(clean) or clean[second] != "{":
            continue
        second_end = matching(clean, second, "{", "}")
        if second_end is not None:
            yield item.start(), p, q, first, first_end, after, second, second_end


def mutations(body: str, preserve_lines: bool = False) -> list[tuple[str, str]]:
    made: list[tuple[str, str]] = []

    def add(label: str, changed: str) -> None:
        if changed != body and (not preserve_lines or changed.count("\n") == body.count("\n")):
            made.append((label, changed))

    # Swap adjacent plain assignments to distinct fields/locals with no data dependency.
    lhs_rx = SIMPLE
    assignment_line = re.compile(r"^(?P<i>[ \t]*)(?P<l>" + lhs_rx + r")\s*=\s*(?P<r>[^;\r\n]+);(?P<t>[ \t]*(?://[^\r\n]*)?)$")
    lines = body.splitlines(keepends=True)
    offsets = [0]
    for line in lines:
        offsets.append(offsets[-1] + len(line))
    for index, (line_a, line_b) in enumerate(zip(lines, lines[1:])):
        a_text, b_text = line_a.rstrip("\r\n"), line_b.rstrip("\r\n")
        a_match, b_match = assignment_line.fullmatch(a_text), assignment_line.fullmatch(b_text)
        if not a_match or not b_match:
            continue
        la, lb, ra, rb = (a_match.group("l"), b_match.group("l"),
                          a_match.group("r"), b_match.group("r"))
        roots = {re.split(r"\.|->|\[", lhs.lstrip("*"))[0] for lhs in (la, lb)}
        pure = all(not re.search(r"\w+\s*\(|\+\+|--|(?<![<>=!])=(?!=)", rhs) for rhs in (ra, rb))
        if la != lb and pure and not (set(re.findall(r"[A-Za-z_]\w*", ra)) | set(re.findall(r"[A-Za-z_]\w*", rb))) & roots:
            swapped = b_text + line_b[len(b_text):] + a_text + line_a[len(a_text):]
            start, end = offsets[index], offsets[index + 2]
            add("swap-independent-assignments", body[:start] + swapped + body[end:])

    # Invert a braced if/else while keeping both lexical scopes intact.
    for start, p, q, first, first_end, after, second, second_end in _match_if_blocks(body):
        cond = body[p + 1:q]
        gap = body[q + 1:first]
        between = body[first_end + 1:after]
        gap2 = body[after + 4:second]
        value = "if (!({})){}{}{}else{}{}".format(cond, gap, body[second:second_end + 1], between, gap2,
                                                   body[first:first_end + 1])
        add("invert-if-else", body[:start] + value + body[second_end + 1:])

    # Reverse relational operand order for side-effect-free scalar atoms.
    cmp_rx = re.compile(r"(?<![<>=!])(?P<a>" + ATOM + r")\s*(?P<op><=|>=|<|>)\s*(?P<b>" + ATOM + r")(?![=])")
    clean = mask(body)
    for m in cmp_rx.finditer(clean):
        left_context = clean[:m.start()].rstrip()
        right_context = clean[m.end():].lstrip()
        if (left_context and left_context[-1] in "+-*/%&|^<>=!") or (
                right_context and right_context[0] in "+-*/%&|^<>=!"):
            continue
        op = {"<": ">", ">": "<", "<=": ">=", ">=": "<="}[m.group("op")]
        add("flip-comparison-operands", body[:m.start()] + f"{m.group('b')}{op}{m.group('a')}" + body[m.end():])

    # Expand/contract arithmetic compound assignments on simple scalar lvalues.
    compound = re.compile(r"(?m)^(?P<i>[ \t]*)(?P<l>" + SIMPLE + r")\s*(?P<op><<|>>|[+\-*/%&|^])=\s*(?P<r>[^;\r\n]+);(?P<t>[^\r\n]*)$")
    for m in compound.finditer(body):
        lhs, rhs, op = m.group("l"), m.group("r").strip(), m.group("op")
        if not re.search(r"\b" + re.escape(lhs.split(".")[0]) + r"\b", rhs) and not re.search(r"\w\s*\(", rhs):
            add("expand-compound-assignment", body[:m.start()] + f"{m.group('i')}{lhs} = {lhs} {op} ({rhs});{m.group('t')}" + body[m.end():])
    expand = re.compile(r"(?m)^(?P<i>[ \t]*)(?P<l>" + SIMPLE + r")\s*=\s*(?P=l)\s*(?P<op><<|>>|[+\-*/%&|^])\s*(?P<r>[^;\r\n]+);(?P<t>[^\r\n]*)$")
    for m in expand.finditer(body):
        if not re.search(r"\b" + re.escape(m.group("l").split(".")[0]) + r"\b", m.group("r")):
            add("contract-compound-assignment", body[:m.start()] + f"{m.group('i')}{m.group('l')} {m.group('op')}= {m.group('r').strip()};{m.group('t')}" + body[m.end():])

    # Split or merge simple scalar declarations; transformations stay on the same source line.
    decl = re.compile(r"(?m)^(?P<i>[ \t]*)(?P<t>" + SCALAR + r")\s+(?P<a>[A-Za-z_]\w*)\s*=\s*(?P<av>" + ATOM + r")\s*,\s*(?P<b>[A-Za-z_]\w*)\s*=\s*(?P<bv>" + ATOM + r");(?P<tail>[^\r\n]*)$")
    for m in decl.finditer(body):
        add("split-declaration", body[:m.start()] + f"{m.group('i')}{m.group('t')} {m.group('a')} = {m.group('av')}; {m.group('t')} {m.group('b')} = {m.group('bv')};{m.group('tail')}" + body[m.end():])
    merge = re.compile(r"(?m)^(?P<i>[ \t]*)(?P<t>" + SCALAR + r")\s+(?P<a>[A-Za-z_]\w*)\s*=\s*(?P<av>" + ATOM + r");\r?\n(?P=i)(?P=t)\s+(?P<b>[A-Za-z_]\w*)\s*=\s*(?P<bv>" + ATOM + r");")
    for m in merge.finditer(body):
        add("merge-declarations", body[:m.start()] + f"{m.group('i')}{m.group('t')} {m.group('a')} = {m.group('av')},\n{m.group('i')}    {m.group('b')} = {m.group('bv')};" + body[m.end():])
    init = re.compile(r"(?m)^(?P<i>[ \t]*)(?P<t>" + SCALAR + r")\s+(?P<n>[A-Za-z_]\w*)\s*=\s*(?P<e>" + ATOM + r");")
    for m in init.finditer(body):
        add("split-initialization", body[:m.start()] + f"{m.group('i')}{m.group('t')} {m.group('n')};\n{m.group('i')}{m.group('n')} = {m.group('e')};" + body[m.end():])
    uninit = re.compile(r"(?m)^(?P<i>[ \t]*)(?P<t>" + SCALAR + r")\s+(?P<n>[A-Za-z_]\w*);\r?\n(?P=i)(?P=n)\s*=\s*(?P<e>" + ATOM + r");")
    for m in uninit.finditer(body):
        add("merge-initialization", body[:m.start()] + f"{m.group('i')}{m.group('t')} {m.group('n')} = {m.group('e')};" + body[m.end():])

    # ?: assignment and a matching braced if/else assignment.
    ternary = re.compile(r"(?m)^(?P<i>[ \t]*)(?P<l>" + SIMPLE + r")\s*=\s*(?P<c>" + ATOM + r")\s*\?\s*(?P<a>[^?:;\r\n]+)\s*:\s*(?P<b>[^;\r\n]+);(?P<t>[^\r\n]*)$")
    for m in ternary.finditer(body):
        add("ternary-to-if-else", body[:m.start()] + f"{m.group('i')}if ({m.group('c')}) {{ {m.group('l')} = {m.group('a').strip()}; }} else {{ {m.group('l')} = {m.group('b').strip()}; }}{m.group('t')}" + body[m.end():])
    for start, p, q, first, first_end, after, second, second_end in _match_if_blocks(body):
        left = re.fullmatch(r"\s*(" + SIMPLE + r")\s*=\s*([^;{}]+);\s*", body[first + 1:first_end])
        right = re.fullmatch(r"\s*(" + SIMPLE + r")\s*=\s*([^;{}]+);\s*", body[second + 1:second_end])
        cond = body[p + 1:q].strip()
        if left and right and left.group(1) == right.group(1) and re.fullmatch(ATOM, cond):
            add("if-else-to-ternary", body[:start] + f"{left.group(1)} = {cond} ? {left.group(2).strip()} : {right.group(2).strip()};" + body[second_end + 1:])

    # for <-> while for braced loops without continue (which changes increment behavior).
    clean = mask(body)
    for m in re.finditer(r"\bfor\s*\(", clean):
        p = clean.find("(", m.start(), m.end())
        q = matching(clean, p, "(", ")")
        if q is None:
            continue
        parts = body[p + 1:q].split(";")
        if len(parts) != 3 or not all(part.strip() for part in parts[1:]):
            continue
        brace = q + 1
        while brace < len(clean) and clean[brace].isspace():
            brace += 1
        end = matching(clean, brace, "{", "}") if brace < len(clean) and clean[brace] == "{" else None
        if end is None:
            continue
        loop_body = body[brace + 1:end]
        if re.search(r"\bcontinue\b", mask(loop_body)):
            continue
        changed = "{ " + parts[0].strip() + "; while (" + parts[1].strip() + ") {" + loop_body + " " + parts[2].strip() + "; } }"
        add("for-to-while", body[:m.start()] + changed + body[end + 1:])
    while_rx = re.compile(r"\bwhile\s*\(([^()]*)\)\s*(\{)")
    for m in while_rx.finditer(clean):
        end = matching(clean, m.start(2), "{", "}")
        if end is not None:
            add("while-to-for", body[:m.start()] + "for (; " + m.group(1).strip() + "; ) " + body[m.start(2):end + 1] + body[end + 1:])

    local_decl = re.compile(r"(?m)^[ \t]*(?P<t>" + LOCAL_TYPE + r")\s+(?P<n>[A-Za-z_]\w*)\s*(?:\[[^\]\r\n]*\])?\s*(?==|;|,)")
    types = {m.group("n"): re.sub(r"\s+", " ", m.group("t")).strip()
             for m in local_decl.finditer(mask(body))
             if m.group("t") not in {"if", "else", "return", "case", "while", "for"}}

    # Small flat switches and scalar comparison chains can change form.
    clean = mask(body)
    for m in re.finditer(r"\bswitch\s*\(\s*(" + SIMPLE + r")\s*\)\s*\{", clean):
        brace = clean.find("{", m.start(), m.end())
        end = matching(clean, brace, "{", "}")
        if end is None:
            continue
        inner = body[brace + 1:end]
        cases = list(re.finditer(r"(?m)\s*(case\s+[-A-Za-z_0-9]+|default)\s*:", mask(inner)))
        if not 2 <= len(cases) <= 6:
            continue
        parts = []
        ok = True
        for idx, case in enumerate(cases):
            finish = cases[idx + 1].start() if idx + 1 < len(cases) else len(inner)
            content = inner[case.end():finish].strip()
            if "{" in mask(content) or not re.search(r"\bbreak\s*;\s*$", mask(content)):
                ok = False
                break
            content = re.sub(r"\bbreak\s*;\s*$", "", content).strip()
            if re.search(r"\b(case|default|continue|goto)\b", mask(content)) or re.search(r"\b" + SCALAR + r"\s+\w+", mask(content)):
                ok = False
                break
            if re.search(r"\bbreak\s*;", mask(content)):
                ok = False
                break
            label = case.group(1)
            parts.append((label, content))
        if any(label == "default" for label, _ in parts[:-1]):
            continue
        if not ok:
            continue
        expr = m.group(1)
        output = ""
        for label, content in parts:
            if label == "default":
                output += " else { " + content + " }"
            else:
                value = label.split(None, 1)[1]
                output += ("if" if not output else " else if") + f" ({expr} == {value}) {{ {content} }}"
        add("switch-to-if-chain", body[:m.start()] + output + body[end + 1:])

    chain_head = re.compile(r"if\s*\(\s*(?P<x>" + SIMPLE + r")\s*==\s*(?P<v>" + ATOM + r")\s*\)\s*\{")
    for first in chain_head.finditer(clean):
        expression = first.group("x")
        value_type = infer_type(expression, types)
        if not value_type or not re.search(r"int|long|short|char|bool|byte|word|dword", value_type, re.I):
            continue
        branches = []
        cursor = first.start()
        head = first
        valid = True
        while True:
            if head.group("x") != expression:
                valid = False
                break
            brace = clean.find("{", head.start(), head.end())
            end = matching(clean, brace, "{", "}")
            if end is None:
                valid = False
                break
            content = body[brace + 1:end]
            if re.search(r"\b(break|goto|case|default)\b", mask(content)):
                valid = False
                break
            branches.append((head.group("v"), content.strip()))
            cursor = end + 1
            while cursor < len(clean) and clean[cursor].isspace():
                cursor += 1
            if not clean.startswith("else", cursor):
                break
            cursor += 4
            while cursor < len(clean) and clean[cursor].isspace():
                cursor += 1
            if clean.startswith("if", cursor):
                head = chain_head.match(clean, cursor)
                if head is None:
                    valid = False
                    break
                continue
            if cursor < len(clean) and clean[cursor] == "{":
                end = matching(clean, cursor, "{", "}")
                if end is None:
                    valid = False
                    break
                content = body[cursor + 1:end]
                if re.search(r"\b(break|goto|case|default)\b", mask(content)):
                    valid = False
                    break
                branches.append(("default", content.strip()))
                cursor = end + 1
            break
        if not valid or len([label for label, _ in branches if label != "default"]) < 2:
            continue
        output = "switch (" + expression + ") {"
        for label, content in branches:
            output += f" {label if label == 'default' else 'case ' + label}: {{ {content} }} break;"
        output += " }"
        add("if-chain-to-switch", body[:first.start()] + output + body[cursor:])
    # Reassociate only integer chains; floating-point addition is not associative.
    chain = re.compile(r"(?<![\w)])(?P<a>" + ATOM + r")\s*(?P<op>[+|])\s*(?P<b>" + ATOM + r")\s*(?P=op)\s*(?P<c>" + ATOM + r")")
    for m in chain.finditer(mask(body)):
        terms = (m.group("a"), m.group("b"), m.group("c"))
        integer = all(term.lstrip("-").isdigit() or
                      bool(re.search(r"int|long|short|char|bool|byte|word", infer_type(term, types) or "", re.I))
                      for term in terms)
        if integer:
            add("reassociate-chain", body[:m.start()] + f"{terms[0]} {m.group('op')} ({terms[1]} {m.group('op')} {terms[2]})" + body[m.end():])

    # Introduce/remove typed temporaries only when local or header declarations prove the type.
    assignments = re.compile(r"(?m)^(?P<i>[ \t]*)(?:(?P<decl>" + SCALAR + r")\s+)?(?P<l>" + SIMPLE + r")\s*=\s*(?P<rhs>[^;\r\n]+);(?P<t>[^\r\n]*)$")
    for m in assignments.finditer(body):
        rhs = m.group("rhs").strip()
        if re.search(r"\w+\s*\(|\+\+|--|(?<![<>=!])=(?!=)", rhs):
            continue
        expressions = [item.group() for item in re.finditer(CHAIN_EXPR, mask(rhs))]
        for expression in sorted(set(expressions), key=len, reverse=True):
            if sum(item.group() == expression for item in re.finditer(CHAIN_EXPR, mask(rhs))) < 2:
                continue
            typ = infer_type(expression, types)
            if not typ or "volatile" in typ:
                continue
            name = "permuteValue"
            while re.search(r"\b" + name + r"\b", body):
                name += "X"
            rhs2 = re.sub(r"(?<![A-Za-z_0-9])" + re.escape(expression) + r"(?![A-Za-z_0-9])", name, rhs)
            eol = "\r\n" if "\r\n" in body else "\n"
            destination = (m.group("decl") + " " if m.group("decl") else "")
            add("introduce-temporary", body[:m.start()] + f"{m.group('i')}{typ} {name} = {expression};{eol}{m.group('i')}{destination}{m.group('l')} = {rhs2};{m.group('t')}" + body[m.end():])

    temp_rx = re.compile(r"(?m)^(?P<i>[ \t]*)(?P<t>" + LOCAL_TYPE + r")\s+(?P<n>[A-Za-z_]\w*)\s*=\s*(?P<e>" + CHAIN_EXPR + r"|-?[0-9]+);(?P<nl>\r?\n)")
    for m in temp_rx.finditer(body):
        rest = body[m.end():]
        if len(re.findall(r"\b" + re.escape(m.group("n")) + r"\b", mask(rest))) != 1:
            continue
        code_lines = [line for line in rest.splitlines() if mask(line).strip()]
        if not code_lines or not re.match(r"\s*(?:return\s+|" + SIMPLE + r"\s*=)", mask(code_lines[0])):
            continue
        if re.search(r"\w+\s*\(|\+\+|--", mask(code_lines[0])):
            continue
        expression_type = "int" if m.group("e").lstrip("-").isdigit() else infer_type(m.group("e"), types)
        if expression_type is None or m.group("t").replace("const ", "").strip() != expression_type:
            continue
        use = re.search(r"\b" + re.escape(m.group("n")) + r"\b", mask(rest))
        if use:
            changed_rest = _replace(rest, use.start(), use.end(), m.group("e"))
            add("remove-temporary", body[:m.start()] + changed_rest)

    # Move trivial local scalar declarations across a single braced if boundary.
    clean = mask(body)
    for m in re.finditer(r"\bif\s*\([^\n]*?\)\s*\{", clean):
        brace = clean.find("{", m.start(), m.end())
        end = matching(clean, brace, "{", "}")
        if end is None:
            continue
        inner = body[brace + 1:end]
        decl_rx = re.compile(r"(?m)^(?P<i>[ \t]*)(?P<t>" + SCALAR + r")\s+(?P<n>[A-Za-z_]\w*)\s*=\s*(?P<e>" + ATOM + r");")
        for d in decl_rx.finditer(inner):
            if len(re.findall(r"\b" + re.escape(d.group("n")) + r"\b", mask(inner))) < 2:
                continue
            outside = body[:brace] + body[end + 1:]
            if re.search(r"\b" + re.escape(d.group("n")) + r"\b", mask(outside)):
                continue
            inner2 = inner[:d.start()] + f"{d.group('i')}{d.group('n')} = {d.group('e')};" + inner[d.end():]
            start = m.start()
            changed = body[:start] + d.group("t") + " " + d.group("n") + ";\n" + body[start:brace + 1] + inner2 + body[end:]
            add("move-declaration-out-of-block", changed)
    # Remove a declaration before an if and put it inside at the original assignment point.
    scope = re.compile(r"(?m)^(?P<i>[ \t]*)(?P<t>" + SCALAR + r")\s+(?P<n>[A-Za-z_]\w*);\r?\n(?P<if>[ \t]*if\s*\([^\n]*?\)\s*\{)(?P<nl>\r?\n)?(?P<indent>[ \t]*)(?P=n)\s*=\s*(?P<e>" + ATOM + r");")
    for m in scope.finditer(body):
        tail = body[m.end():]
        if re.search(r"\b" + re.escape(m.group("n")) + r"\b", mask(tail)):
            continue
        changed = body[:m.start()] + m.group("i") + "\n" + m.group("if") + (m.group("nl") or "") + m.group("indent") + m.group("t") + " " + m.group("n") + " = " + m.group("e") + ";" + body[m.end():]
        add("move-declaration-into-block", changed)

    return made


def load_manifest() -> dict:
    return json.loads((ROOT / "recon.json").read_text(encoding="utf-8"))


def compile_and_score(source: Path, object_path: Path, function: str, rva: int,
                      unit_stem: str, unit: dict, manifest: dict, worker: Path) -> dict:
    tool = manifest["toolchain"]
    tree = msvc.resolve_tree(tool["tree"]).resolve()
    compiler_bin, vc_root, shared_bin, default_includes = msvc.tool_layout(tree, None)
    worker.mkdir(parents=True, exist_ok=True)
    temp = worker / "tmp"
    temp.mkdir(parents=True, exist_ok=True)
    object_path.parent.mkdir(parents=True, exist_ok=True)
    include_dirs = [source.parent, ROOT / "src", *[(ROOT / p).resolve() if not Path(p).is_absolute() else Path(p)
                                                       for p in tool.get("include_dirs", [])]]
    command = [str(compiler_bin / "CL.EXE"), "/nologo", "/c", str(source), f"/Fo{object_path}"]
    command += [f"/I{path.resolve()}" for path in include_dirs]
    command += [f"/D{definition}" for definition in tool.get("defines", [])]
    command += [f"/D{definition}" for definition in unit.get("defines", [])]
    command += tool.get("flags", [])
    path_dirs = [compiler_bin]
    if shared_bin and shared_bin.is_dir():
        path_dirs.append(shared_bin)
    path_dirs.extend([Path(os.environ.get("SystemRoot", r"C:\Windows")) / "System32",
                      Path(os.environ.get("SystemRoot", r"C:\Windows"))])
    env = {"PATH": os.pathsep.join(map(str, path_dirs)),
           "INCLUDE": os.pathsep.join(map(str, default_includes)),
           "LIB": str(vc_root / "LIB"), "TMP": str(temp), "TEMP": str(temp),
           "SystemRoot": os.environ.get("SystemRoot", r"C:\Windows")}
    run = subprocess.run(command, cwd=ROOT, env=env, stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT, text=True, encoding="mbcs", errors="replace")
    (worker / "compile.log").write_text(subprocess.list2cmdline(command) + "\n" + run.stdout, encoding="utf-8")
    if run.returncode:
        return {"compile": "failed", "returncode": run.returncode, "log": str(worker / "compile.log")}
    try:
        score = asmscore.score_function(object_path, function, rva)
    except Exception as exc:
        return {"compile": "ok", "score_error": str(exc), "log": str(worker / "compile.log")}
    score.update({"compile": "ok", "object": str(object_path)})
    return score


def digest(text: str) -> str:
    return hashlib.sha256(text.encode("cp1252", "replace")).hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tu", required=True, metavar="src/X.cpp=CANDIDATE.cpp")
    parser.add_argument("--function", required=True, help="decorated COFF function symbol")
    parser.add_argument("--rva", required=True, type=lambda value: int(value, 0))
    parser.add_argument("--budget", type=int, default=80, help="total compiles, including baseline")
    parser.add_argument("--jobs", type=int, default=max(1, os.cpu_count() or 1))
    parser.add_argument("--seed", type=int, default=1)
    args = parser.parse_args()
    if args.budget < 1 or args.jobs < 1:
        parser.error("--budget and --jobs must be positive")
    if "=" not in args.tu:
        parser.error("--tu must be src/X.cpp=CANDIDATE.cpp")
    canonical, candidate = args.tu.split("=", 1)
    canonical_path = Path(canonical)
    candidate_path = Path(candidate)
    if not candidate_path.is_absolute():
        candidate_path = ROOT / candidate_path
    candidate_path = candidate_path.resolve()
    manifest = load_manifest()
    units = {item["source"] for item in manifest["translation_units"]}
    if canonical not in units or not candidate_path.is_file():
        parser.error(f"unknown TU or missing candidate: {canonical}={candidate_path}")
    source = candidate_path.read_bytes().decode("cp1252")
    body_start, body_end = locate_body(source, args.function)
    original_body = source[body_start:body_end]
    preserve_lines = "__LINE__" in source
    unit_stem = canonical_path.stem
    unit_config = next(item for item in manifest["translation_units"] if item["source"] == canonical)
    slug = re.sub(r"[^A-Za-z0-9_.-]+", "_", args.function).strip("._")
    output_root = ROOT / "build" / "permute" / slug
    variants_root = ROOT / "scratch" / "permute" / slug / "variants"
    output_root.mkdir(parents=True, exist_ok=True)
    variants_root.mkdir(parents=True, exist_ok=True)
    rng = random.Random(args.seed)

    def materialize(body: str, index: int) -> tuple[Path, str]:
        full = source[:body_start] + body + source[body_end:]
        variant_dir = variants_root / f"v{index:05d}"
        variant_dir.mkdir(parents=True, exist_ok=True)
        path = variant_dir / f"{unit_stem}.cpp"
        path.write_bytes(full.encode("cp1252"))
        return path, full

    baseline_path, baseline_text = materialize(original_body, 0)
    baseline_worker = output_root / "workers" / "w00000"
    baseline = compile_and_score(baseline_path, baseline_worker / f"00_{unit_stem}.obj",
                                 args.function, args.rva, unit_stem, unit_config, manifest, baseline_worker)
    scored: list[dict] = []
    base_entry = {"body": original_body, "text": baseline_text, "score": baseline,
                  "path": baseline_path, "mutations": []}
    if baseline.get("compile") == "ok" and "score" in baseline:
        scored.append(base_entry)
    attempts = 1
    beam_width = max(2, min(6, args.jobs))
    beam = [base_entry] if scored else []
    compiled_hashes = {digest(original_body)}
    expanded_hashes: set[str] = set()
    pending: list[tuple[dict, str, list[str]]] = []
    serial = 1

    def random_child(parent: dict) -> tuple[str, list[str]] | None:
        current = parent["body"]
        labels = []
        count = rng.choice((1, 1, 1, 2, 2, 3))
        for _ in range(count):
            options = mutations(current, preserve_lines)
            options = [item for item in options if digest(item[1]) not in compiled_hashes]
            if not options:
                break
            label, current = rng.choice(options)
            labels.append(label)
        if current == parent["body"]:
            return None
        return current, labels

    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        while attempts < args.budget and beam:
            for parent in beam:
                parent_hash = digest(parent["body"])
                if parent_hash in expanded_hashes:
                    continue
                options = mutations(parent["body"], preserve_lines)
                rng.shuffle(options)
                for label, body in options:
                    key = digest(body)
                    if key not in compiled_hashes and all(digest(queued[1]) != key for queued in pending):
                        pending.append((parent, body, [label]))
                expanded_hashes.add(parent_hash)
            batch = []
            generation = min(args.jobs, args.budget - attempts)
            while pending and len(batch) < generation:
                parent, body, labels = pending.pop(0)
                key = digest(body)
                if key in compiled_hashes:
                    continue
                compiled_hashes.add(key)
                index = serial
                serial += 1
                variant_path, full = materialize(body, index)
                worker = output_root / "workers" / f"w{index:05d}"
                obj = worker / f"{index:05d}_{unit_stem}.obj"
                batch.append((index, parent, body, full, labels, variant_path, worker, obj))
            while len(batch) < generation and not pending:
                parent = rng.choice(beam)
                chosen = random_child(parent)
                if chosen is None:
                    break
                body, labels = chosen
                key = digest(body)
                if key in compiled_hashes:
                    continue
                compiled_hashes.add(key)
                index = serial
                serial += 1
                variant_path, full = materialize(body, index)
                worker = output_root / "workers" / f"w{index:05d}"
                obj = worker / f"{index:05d}_{unit_stem}.obj"
                batch.append((index, parent, body, full, labels, variant_path, worker, obj))
            if not batch:
                break
            futures = [pool.submit(compile_and_score, item[5], item[7], args.function,
                                   args.rva, unit_stem, unit_config, manifest, item[6]) for item in batch]
            for item, future in zip(batch, futures):
                result = future.result()
                attempts += 1
                if result.get("compile") == "ok" and "score" in result:
                    entry = {"body": item[2], "text": item[3], "score": result,
                             "path": item[5], "mutations": item[4]}
                    scored.append(entry)
                    print(f"compile {attempts}/{args.budget}: score={result['score']} status={result['status']} "
                          f"prefix={result['identical_prefix']} via {','.join(item[4])}")
                else:
                    print(f"compile {attempts}/{args.budget}: {result.get('compile')} "
                          f"{result.get('score_error', '')}")
            ranked = sorted(scored, key=lambda item: (item["score"]["score"], -item["score"]["exact_matching_instructions"]))
            unique = []
            hashes = set()
            for item in ranked:
                key = digest(item["body"])
                if key not in hashes:
                    hashes.add(key)
                    unique.append(item)
                if len(unique) >= beam_width:
                    break
            beam = unique

    ranked = sorted(scored, key=lambda item: (item["score"]["score"], -item["score"]["exact_matching_instructions"]))
    best = ranked[:beam_width]
    for index, item in enumerate(best, 1):
        diff = difflib.unified_diff(source.splitlines(keepends=True), item["text"].splitlines(keepends=True),
                                    fromfile="candidate.cpp", tofile=f"best-{index:02d}.cpp")
        (output_root / f"best-{index:02d}.diff").write_text("".join(diff), encoding="utf-8")
    summary = {"tu": canonical, "candidate": str(candidate_path), "function": args.function,
               "rva": f"0x{args.rva:08x}", "seed": args.seed, "budget": args.budget,
               "compiles_attempted": attempts, "compiled_and_scored": len(scored),
               "baseline": baseline, "best": [{k: v for k, v in item["score"].items() if k not in ("relocations",)}
                                                | {"mutations": item["mutations"], "source": str(item["path"])}
                                                for item in best]}
    (output_root / "summary.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    if ranked:
        top = ranked[0]["score"]
        print(f"best: {args.function} RVA {args.rva:#x} score={top['score']} status={top['status']} "
              f"exact={top['exact_matching_instructions']}/{top['oracle_instructions']} "
              f"prefix={top['identical_prefix']} compiles={attempts}")
        print(f"results: {output_root}")
        return 0
    print(f"no scoreable variants; baseline compile result: {baseline}")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
