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
import time
from typing import Callable

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import asmscore  # noqa: E402

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


@lru_cache(maxsize=2)
def call_signature_text(translation_unit: str) -> str:
    chunks = [translation_unit]
    chunks.extend(path.read_bytes().decode("cp1252", "replace") for path in (ROOT / "src").glob("*.h"))
    return mask("\n".join(chunks))


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


def mutations(body: str, preserve_lines: bool = False, translation_unit: str = "") -> list[tuple[str, str]]:
    made: list[tuple[str, str]] = []

    def add(label: str, changed: str) -> None:
        if changed != body and (not preserve_lines or changed.count("\n") == body.count("\n")):
            made.append((label, changed))

    def pure_value(value: str) -> bool:
        return not re.search(r"\w\s*\(|\+\+|--|(?<![<>=!])=(?!=)|\bthrow\b", value)

    def local_name() -> str:
        name = "permuteValue"
        while re.search(r"\b" + name + r"\b", body):
            name += "X"
        return name

    # Swap adjacent plain assignments to distinct fields/locals with no data dependency.
    lhs_rx = SIMPLE
    assignment_line = re.compile(r"^(?P<i>[ \t]*)(?P<l>" + lhs_rx + r")\s*=\s*(?P<r>[^;\r\n]+);(?P<t>[ \t]*(?://[^\r\n]*)?)$")
    lines = body.splitlines(keepends=True)
    offsets = [0]
    for line in lines:
        offsets.append(offsets[-1] + len(line))
    assignment_matches = []
    for index, line in enumerate(lines):
        match = assignment_line.fullmatch(line.rstrip("\r\n"))
        if match and pure_value(match.group("r")):
            lhs = match.group("l")
            # Multi-part lvalues can alias through pointers or overlapping fields.
            if re.fullmatch(r"[A-Za-z_]\w*", lhs):
                reads = set(re.findall(r"[A-Za-z_]\w*", match.group("r")))
                assignment_matches.append((index, match, {lhs}, reads))
    # Move an assignment across up to four plain assignments when every crossed
    # statement is independent by identifier read/write sets.
    for left_index, (line_index, first, first_writes, first_reads) in enumerate(assignment_matches):
        for next_index in range(left_index + 1, min(left_index + 5, len(assignment_matches))):
            next_line_index, second, _second_writes, _second_reads = assignment_matches[next_index]
            between = assignment_matches[left_index:next_index + 1]
            if next_line_index - line_index > 4 or len(between) != next_line_index - line_index + 1:
                continue
            group = [(item[2], item[3]) for item in between]
            if any(w1 & (w2 | r2) or w2 & r1 for pos, (w1, r1) in enumerate(group)
                   for w2, r2 in group[pos + 1:]):
                continue
            moved = lines.copy()
            moved.insert(line_index, moved.pop(next_line_index))
            add("move-independent-assignment", "".join(moved))

    for index, (line_a, line_b) in enumerate(zip(lines, lines[1:])):
        a_text, b_text = line_a.rstrip("\r\n"), line_b.rstrip("\r\n")
        a_match, b_match = assignment_line.fullmatch(a_text), assignment_line.fullmatch(b_text)
        if not a_match or not b_match:
            continue
        la, lb, ra, rb = (a_match.group("l"), b_match.group("l"),
                          a_match.group("r"), b_match.group("r"))
        roots = {re.split(r"\.|->|\[", lhs.lstrip("*"))[0] for lhs in (la, lb)}
        pure = all(pure_value(rhs) for rhs in (ra, rb))
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

    # Combine/split adjacent side-effect-free guards with identical exits.
    guard_pair = re.compile(r"(?m)^(?P<i>[ \t]*)if\s*\((?P<a>[^()\r\n]+)\)\s*(?P<r>return(?:\s+[^;\r\n]+)?;)\s*\r?\n(?P=i)if\s*\((?P<b>[^()\r\n]+)\)\s*(?P=r)\s*$")
    for m in guard_pair.finditer(body):
        if pure_value(m.group("a")) and pure_value(m.group("b")):
            add("merge-guard-returns", body[:m.start()] +
                f"{m.group('i')}if (({m.group('a').strip()}) || ({m.group('b').strip()})) {m.group('r')}" + body[m.end():])
    guard_or = re.compile(r"(?m)^(?P<i>[ \t]*)if\s*\((?P<a>[^|()\r\n]+)\s*\|\|\s*(?P<b>[^|()\r\n]+)\)\s*(?P<r>return(?:\s+[^;\r\n]+)?;)\s*$")
    for m in guard_or.finditer(body):
        if pure_value(m.group("a")) and pure_value(m.group("b")):
            add("split-guard-return", body[:m.start()] +
                f"{m.group('i')}if ({m.group('a').strip()}) {m.group('r')}\n{m.group('i')}if ({m.group('b').strip()}) {m.group('r')}" + body[m.end():])

    # A top-level void early return is equivalent to guarding the remaining
    # straight-line tail, provided that tail has no alternate exit or label.
    guard_void = re.compile(r"(?m)^(?P<i>[ \t]*)if\s*\((?P<c>[^()\r\n]+)\)\s*return\s*;[ \t]*\r?\n")
    clean = mask(body)
    for m in guard_void.finditer(clean):
        depth = body[:m.start()].count("{") - body[:m.start()].count("}")
        tail_start = m.end()
        tail_end = body.rfind("}")
        if depth != 1 or tail_end < tail_start:
            continue
        tail = body[tail_start:tail_end]
        if re.search(r"\b(return|goto|break|continue|case|default)\b|^[ \t]*[A-Za-z_]\w*\s*:", mask(tail), re.M):
            continue
        add("early-return-to-fallthrough-block", body[:m.start()] +
            f"{m.group('i')}if (!({m.group('c').strip()})) {{{tail}{m.group('i')}}}" + body[tail_end:])

    # Fold or expand one nested braced if into a short-circuit condition.
    clean = mask(body)
    nested = re.compile(r"\bif\s*\((?P<a>[^()\r\n]+)\)\s*\{\s*if\s*\((?P<b>[^()\r\n]+)\)\s*\{")
    for m in nested.finditer(clean):
        if not pure_value(m.group("a")) or not pure_value(m.group("b")):
            continue
        first_brace = clean.find("{", m.start(), m.end())
        inner_if = clean.find("if", first_brace + 1, m.end())
        inner_brace = clean.find("{", inner_if, m.end())
        inner_end = matching(clean, inner_brace, "{", "}")
        outer_end = matching(clean, first_brace, "{", "}")
        if inner_end is None or outer_end is None:
            continue
        between = body[inner_end + 1:outer_end]
        if mask(between).strip():
            continue
        inside = body[inner_brace + 1:inner_end]
        changed = f"if (({m.group('a').strip()}) && ({m.group('b').strip()})) {{{inside}}}"
        add("nested-if-to-and", body[:m.start()] + changed + body[outer_end + 1:])
    and_if = re.compile(r"\bif\s*\(\s*(?P<a>[^()&|\r\n]+)\s*&&\s*(?P<b>[^()&|\r\n]+)\s*\)\s*\{")
    for m in and_if.finditer(clean):
        if not pure_value(m.group("a")) or not pure_value(m.group("b")):
            continue
        brace = clean.find("{", m.start(), m.end())
        end = matching(clean, brace, "{", "}")
        if end is not None:
            inside = body[brace + 1:end]
            changed = f"if ({m.group('a').strip()}) {{ if ({m.group('b').strip()}) {{{inside}}} }}"
            add("and-to-nested-if", body[:m.start()] + changed + body[end + 1:])

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

    # BLUPI-REG-02: merge duplicate scalar index locals only when both are
    # initialized to the same pure value in adjacent declarations and neither
    # is subsequently written. This is deliberately a narrow identity rewrite.
    initialized = list(re.finditer(r"(?m)^(?P<i>[ \t]*)(?P<t>" + SCALAR + r")\s+(?P<n>[A-Za-z_]\w*)\s*=\s*(?P<e>" + ATOM + r");[ \t]*$", body))
    for left, right in zip(initialized, initialized[1:]):
        if left.group("t") != right.group("t") or left.group("e") != right.group("e") or not pure_value(left.group("e")):
            continue
        if mask(body[left.end():right.start()]).strip():
            continue
        a, b = left.group("n"), right.group("n")
        if a == b or not re.search(r"\[\s*" + re.escape(a) + r"\s*\]", clean) or not re.search(r"\[\s*" + re.escape(b) + r"\s*\]", clean):
            continue
        writes_a = list(re.finditer(r"(?<![=!<>])\b" + re.escape(a) + r"\b\s*(?:=|\+=|-=|\+\+|--)", clean))
        writes_b = list(re.finditer(r"(?<![=!<>])\b" + re.escape(b) + r"\b\s*(?:=|\+=|-=|\+\+|--)", clean))
        if len(writes_a) != 1 or len(writes_b) != 1:
            continue
        if left.start() > writes_a[0].start() or right.start() > writes_b[0].start():
            continue
        changed = body[:right.start()] + body[right.end():]
        changed = re.sub(r"\b" + re.escape(b) + r"\b", a, changed)
        add("merge-index-locals", changed)

    # Split a shared index identity in one block after its fresh assignment.
    # The original local is retained because later code may use it elsewhere.
    clean = mask(body)
    uninit_index = re.compile(r"(?m)^(?P<i>[ \t]*)(?P<t>" + SCALAR + r")\s+(?P<n>[A-Za-z_]\w*)\s*;[ \t]*$")
    for declaration in uninit_index.finditer(body):
        name, typ = declaration.group("n"), declaration.group("t")
        if not re.search(r"\[\s*" + re.escape(name) + r"\s*\]", clean):
            continue
        for block in re.finditer(r"\bif\s*\([^\n]*?\)\s*\{", clean):
            brace = clean.find("{", block.start(), block.end())
            end = matching(clean, brace, "{", "}")
            if end is None or declaration.start() > block.start():
                continue
            inner = body[brace + 1:end]
            assigned = re.search(r"(?m)^(?P<i>[ \t]*)" + re.escape(name) + r"\s*=\s*(?P<e>" + ATOM + r");[ \t]*\r?\n", inner)
            if not assigned or not pure_value(assigned.group("e")):
                continue
            rest = inner[assigned.end():]
            masked_rest = mask(rest)
            if re.search(r"\b" + re.escape(name) + r"\b\s*(?:=|\+=|-=|\+\+|--)", masked_rest):
                continue
            indices = list(re.finditer(r"\[\s*" + re.escape(name) + r"\s*\]", masked_rest))
            if not indices:
                continue
            residue = re.sub(r"\[\s*" + re.escape(name) + r"\s*\]", "[]", masked_rest)
            if re.search(r"\b" + re.escape(name) + r"\b", residue):
                continue
            new_name = "permuteIndex"
            while re.search(r"\b" + new_name + r"\b", body):
                new_name += "X"
            rewritten = re.sub(r"\[\s*" + re.escape(name) + r"\s*\]", "[" + new_name + "]", rest)
            inner2 = (inner[:assigned.end()] + assigned.group("i") + typ + " " + new_name + " = " + name + ";\n" + rewritten)
            add("split-index-local", body[:brace + 1] + inner2 + body[end:])

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

    # Equality tests on the same stable scalar are mutually exclusive, so
    # branch order can vary while each branch body stays attached to its value.
    for first in chain_head.finditer(clean):
        expression = first.group("x")
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
            branches.append((head.group("v"), body[brace:end + 1]))
            cursor = end + 1
            while cursor < len(clean) and clean[cursor].isspace():
                cursor += 1
            if not clean.startswith("else", cursor):
                break
            cursor += 4
            while cursor < len(clean) and clean[cursor].isspace():
                cursor += 1
            if not clean.startswith("if", cursor):
                valid = False
                break
            head = chain_head.match(clean, cursor)
            if head is None:
                valid = False
                break
        if not valid or len(branches) < 2:
            continue
        for index in range(len(branches) - 1):
            if branches[index][0] == branches[index + 1][0]:
                continue
            ordered = branches.copy()
            ordered[index], ordered[index + 1] = ordered[index + 1], ordered[index]
            output = ""
            for branch_index, (value, content) in enumerate(ordered):
                output += ("if" if branch_index == 0 else " else if") + f" ({expression} == {value}) " + content
            add("reorder-exclusive-else-if", body[:first.start()] + output + body[cursor:])
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

    # Name a scalar call result before assigning it. This keeps the call count
    # and order unchanged and gives VC5 a source-level local allocation choice.
    result_assign = re.compile(r"(?m)^(?P<i>[ \t]*)(?:(?P<decl>" + SCALAR + r")\s+)?(?P<l>[A-Za-z_]\w*)\s*=\s*(?P<rhs>[A-Za-z_]\w*\s*\([^;\r\n]*\));(?P<t>[^\r\n]*)$")
    for m in result_assign.finditer(body):
        typ = m.group("decl") or types.get(m.group("l"))
        if not typ:
            continue
        name = local_name()
        declaration = (m.group("decl") + " " if m.group("decl") else "")
        eol = "\r\n" if "\r\n" in body else "\n"
        changed = (f"{m.group('i')}{typ} {name} = {m.group('rhs').strip()};{eol}"
                   f"{m.group('i')}{declaration}{m.group('l')} = {name};{m.group('t')}")
        add("name-call-result", body[:m.start()] + changed + body[m.end():])

    # Materialize an inner call result when all its sibling arguments are
    # integer/boolean literals. This prevents a call-order or aliasing change.
    if translation_unit:
        signature_text = call_signature_text(translation_unit)
        nested_call = re.compile(r"(?m)^(?P<i>[ \t]*)(?P<ret>return\s+)?(?P<outer>[A-Za-z_]\w*)\s*\((?P<args>[^;\r\n]*)\);(?P<t>[^\r\n]*)$")
        literal = re.compile(r"\s*(?:-?\d+|0x[0-9a-fA-F]+|NULL|TRUE|FALSE)\s*")
        for m in nested_call.finditer(body):
            args = m.group("args")
            inner = re.search(r"(?P<callee>[A-Za-z_]\w*)\s*\((?P<inner>[^()]*)\)", args)
            if not inner:
                continue
            siblings = args[:inner.start()] + args[inner.end():]
            if any(part.strip() and not literal.fullmatch(part) for part in siblings.split(",")):
                continue
            callee = inner.group("callee")
            found_types = re.findall(r"\b(" + SCALAR + r"(?:\s*\*)?)\s+(?:[A-Za-z_]\w*::)?" + re.escape(callee) + r"\s*\(", signature_text)
            found_types = list(dict.fromkeys(re.sub(r"\s+", " ", item).strip() for item in found_types))
            if len(found_types) != 1:
                continue
            name = local_name()
            call = args[inner.start():inner.end()].strip()
            changed_args = args[:inner.start()] + name + args[inner.end():]
            eol = "\r\n" if "\r\n" in body else "\n"
            changed = (f"{m.group('i')}{found_types[0]} {name} = {call};{eol}"
                       f"{m.group('i')}{m.group('ret') or ''}{m.group('outer')}({changed_args});{m.group('t')}")
            add("materialize-call-argument", body[:m.start()] + changed + body[m.end():])

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

    # BLUPI-LOC-01: fixed local arrays can move between function and a sole
    # braced scope. Restrict this to POD-like scalar element arrays without an
    # initializer; the only generated difference is lexical lifetime/layout.
    array_decl = re.compile(r"(?m)^(?P<i>[ \t]*)(?P<t>char|BYTE|WORD|DWORD|int|short|long)\s+(?P<n>[A-Za-z_]\w*)\s*\[(?P<size>\d+)\];[ \t]*$")
    for declaration in array_decl.finditer(body):
        name = declaration.group("n")
        for block in re.finditer(r"\bif\s*\([^\n]*?\)\s*\{", clean):
            brace = clean.find("{", block.start(), block.end())
            end = matching(clean, brace, "{", "}")
            if end is None:
                continue
            inner = body[brace + 1:end]
            outside = body[:declaration.start()] + body[declaration.end():brace] + body[end + 1:]
            uses_inner = list(re.finditer(r"\b" + re.escape(name) + r"\b", mask(inner)))
            uses_outside = re.search(r"\b" + re.escape(name) + r"\b", mask(outside))
            if declaration.end() < brace and uses_inner and not uses_outside:
                moved_decl = body[declaration.start():declaration.end()]
                without = body[:declaration.start()] + body[declaration.end():]
                new_brace = brace - len(moved_decl)
                changed = without[:new_brace + 1] + "\n" + moved_decl + without[new_brace + 1:]
                add("move-array-into-block", changed)
            elif brace < declaration.start() < end and uses_inner and not uses_outside:
                moved_decl = body[declaration.start():declaration.end()]
                changed = (body[:block.start()] + moved_decl + "\n" + body[block.start():declaration.start()] +
                           body[declaration.end():])
                add("move-array-out-of-block", changed)

    return made


def load_manifest() -> dict:
    return json.loads((ROOT / "recon.json").read_text(encoding="utf-8"))


def compile_and_score(source: Path, object_path: Path, function: str, rva: int,
                      unit_stem: str, unit: dict, manifest: dict, worker: Path) -> dict:
    tool = manifest["toolchain"]
    worker.mkdir(parents=True, exist_ok=True)
    object_path.parent.mkdir(parents=True, exist_ok=True)
    include_dirs = [source.parent, *unit.get("_candidate_include_dirs", []), ROOT / "src",
                   *[(ROOT / p).resolve() if not Path(p).is_absolute() else Path(p)
                                                       for p in tool.get("include_dirs", [])]]
    command = [sys.executable, str(ROOT / "tools" / "msvc.py"), str(source),
               "--tree", tool["tree"], "--out", str(object_path), "--log-dir", str(worker)]
    if unit.get("build_path"):
        command.extend(["--build-path", unit["build_path"]])
    for path in include_dirs:
        command.extend(["--include", str(path.resolve())])
    for definition in tool.get("defines", []) + unit.get("defines", []):
        command.extend(["--define", definition])
    command += ["--flag=" + flag for flag in tool.get("flags", [])]
    run = subprocess.run(command, cwd=ROOT, stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT, text=True, encoding="mbcs", errors="replace")
    (worker / "compile.log").write_text(subprocess.list2cmdline(command) + "\n" + run.stdout, encoding="utf-8")
    if run.returncode:
        return {"compile": "failed", "returncode": run.returncode, "log": str(worker / "compile.log")}
    try:
        score = asmscore.score_function(object_path, function, rva)
    except Exception as exc:
        return {"compile": "ok", "score_error": str(exc), "object": str(object_path),
                "log": str(worker / "compile.log")}
    score.update({"compile": "ok", "object": str(object_path)})
    return score


def digest(text: str) -> str:
    return hashlib.sha256(text.encode("cp1252", "replace")).hexdigest()


def restricted_mutations(body: str, preserve_lines: bool, source: str,
                         body_start: int, line_ranges: list[tuple[int, int]],
                         operators: set[str]) -> list[tuple[str, str]]:
    """One operator application, confined to each original inclusive line range.

    Regions are sliced before rewriting, so a rewrite cannot reach across their
    boundaries. Exhaustive mode enumerates this finite one-step neighborhood;
    explicit reviewed variants supply combinations or unsupported families.
    """
    if not line_ranges:
        regions = [(0, len(body))]
    else:
        starts = [0]
        starts.extend(match.end() for match in re.finditer("\n", source))
        starts.append(len(source))
        regions = []
        for first, last in line_ranges:
            if first < 1 or last < first or last >= len(starts):
                raise ValueError(f"invalid source line range {first}:{last}")
            left, right = starts[first - 1] - body_start, starts[last] - body_start
            if left < 0 or right > len(body):
                raise ValueError(f"line range {first}:{last} is not inside the function body")
            regions.append((left, right))
    result = []
    for left, right in regions:
        for label, changed in mutations(body[left:right], preserve_lines, source):
            if not operators or label in operators:
                result.append((label, body[:left] + changed + body[right:]))
    return result


def explicit_variants(path: Path, source: str, body_start: int, body_end: int,
                      line_ranges: list[tuple[int, int]]) -> list[tuple[str, str]]:
    """Load [{label, source}] full-TU candidates; paths are relative to the list."""
    result = []
    for item in json.loads(path.read_text(encoding="utf-8")):
        text = (path.parent / item["source"]).read_bytes().decode("cp1252")
        if not text.startswith(source[:body_start]) or not text.endswith(source[body_end:]):
            raise ValueError(f"explicit variant changes text outside function: {item['source']}")
        if "__LINE__" in source and text.count("\n") != source.count("\n"):
            raise ValueError(f"explicit variant shifts __LINE__ source lines: {item['source']}")
        if line_ranges:
            matcher = difflib.SequenceMatcher(a=source.splitlines(keepends=True),
                                              b=text.splitlines(keepends=True), autojunk=False)
            for tag, first, last, _bfirst, _blast in matcher.get_opcodes():
                if tag == "equal":
                    continue
                # A zero-width insertion is allowed only within a selected range.
                if not any(left <= first + 1 and max(first + 1, last) <= right
                           for left, right in line_ranges):
                    raise ValueError(f"explicit variant changes unselected lines: {item['source']}")
        end = len(text) - len(source[body_end:]) if body_end < len(source) else len(text)
        result.append((item["label"], text[body_start:end]))
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tu", required=True, metavar="src/X.cpp=CANDIDATE.cpp")
    parser.add_argument("--function", required=True, help="decorated COFF function symbol")
    parser.add_argument("--rva", required=True, type=lambda value: int(value, 0))
    parser.add_argument("--budget", type=int, default=80, help="total compiles, including baseline")
    parser.add_argument("--jobs", type=int, default=int(os.environ.get("BLUPI_JOBS", "4")),
                        help="concurrent compiles (default $BLUPI_JOBS or 4; keep low - sandbox desktop heap)")
    parser.add_argument("--seed", type=int, default=1)
    parser.add_argument("--exhaustive", action="store_true",
                        help="enumerate every unique one-step rewrite plus explicit variants; no beam/random search")
    parser.add_argument("--lines", action="append", default=[], metavar="FIRST:LAST",
                        help="inclusive original source lines; repeat for disjoint regions")
    parser.add_argument("--operators", help="comma-separated mutation labels to allow")
    parser.add_argument("--variants", type=Path,
                        help="JSON list of reviewed {label, source} candidates, relative to the JSON file")
    parser.add_argument("--plan-only", action="store_true", help="write the exhaustive plan without compiling")
    parser.add_argument("--output-dir", type=Path, help="result directory below build/permute")
    parser.add_argument("--scratch-dir", type=Path, help="variant directory below scratch")
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
    try:
        line_ranges = [tuple(map(int, value.split(":"))) for value in args.lines]
        if any(len(value) != 2 for value in line_ranges):
            raise ValueError("--lines requires FIRST:LAST")
    except ValueError as exc:
        parser.error(str(exc))
    if (args.lines or args.operators or args.variants or args.plan_only) and not args.exhaustive:
        parser.error("--lines, --operators, --variants and --plan-only require --exhaustive")
    operators = set(args.operators.split(",")) if args.operators else set()
    preserve_lines = "__LINE__" in source
    unit_stem = canonical_path.stem
    unit_config = dict(next(item for item in manifest["translation_units"] if item["source"] == canonical))
    unit_config["_candidate_include_dirs"] = [candidate_path.parent]
    slug = re.sub(r"[^A-Za-z0-9_.-]+", "_", args.function).strip("._")
    output_root = ROOT / "build" / "permute" / slug / f"seed_{args.seed}"
    variants_root = ROOT / "scratch" / "permute" / slug / f"seed_{args.seed}" / "variants"
    if args.output_dir:
        output_root = (ROOT / args.output_dir).resolve()
    if args.scratch_dir:
        variants_root = (ROOT / args.scratch_dir).resolve()
    if not output_root.is_relative_to((ROOT / "build" / "permute").resolve()):
        parser.error("--output-dir must be below build/permute")
    if not variants_root.is_relative_to((ROOT / "scratch").resolve()):
        parser.error("--scratch-dir must be below scratch")
    output_root.mkdir(parents=True, exist_ok=True)
    variants_root.mkdir(parents=True, exist_ok=True)
    rng = random.Random(args.seed)
    exhaustive_plan = []
    if args.exhaustive:
        try:
            options = restricted_mutations(original_body, preserve_lines, source, body_start,
                                           line_ranges, operators)
            if args.variants:
                options.extend(explicit_variants(args.variants.resolve(), source, body_start,
                                                 body_end, line_ranges))
        except ValueError as exc:
            parser.error(str(exc))
        seen = {digest(original_body)}
        for label, body in options:
            key = digest(body)
            if key not in seen:
                exhaustive_plan.append((label, body))
                seen.add(key)
        plan = {"mode": "exhaustive-one-step-and-explicit", "lines": line_ranges,
                "operators": sorted(operators), "compiles_required": 1 + len(exhaustive_plan),
                "variants": [{"label": label, "sha256": digest(body)} for label, body in exhaustive_plan]}
        (output_root / "plan.json").write_text(json.dumps(plan, indent=2) + "\n", encoding="utf-8")
        print(f"exhaustive plan: {1 + len(exhaustive_plan)} compiles including baseline", flush=True)
        if args.plan_only:
            return 0
        if 1 + len(exhaustive_plan) > args.budget:
            parser.error("exhaustive plan exceeds --budget; narrow the finite set (no compiles started)")

    def materialize(body: str, index: int) -> tuple[Path, str]:
        full = source[:body_start] + body + source[body_end:]
        variant_dir = variants_root / f"v{index:05d}"
        variant_dir.mkdir(parents=True, exist_ok=True)
        path = variant_dir / f"{unit_stem}.cpp"
        path.write_bytes(full.encode("cp1252"))
        return path, full

    started = time.perf_counter()
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
    if args.exhaustive:
        pending = [(base_entry, body, [label]) for label, body in exhaustive_plan]
    serial = 1
    records = [{"index": 0, "source": str(baseline_path), "mutations": [], "result": baseline}]

    def random_child(parent: dict) -> tuple[str, list[str]] | None:
        current = parent["body"]
        labels = []
        count = rng.choice((1, 1, 1, 2, 2, 3))
        for _ in range(count):
            options = mutations(current, preserve_lines, source)
            options = [item for item in options if digest(item[1]) not in compiled_hashes]
            if not options:
                break
            label, current = rng.choice(options)
            labels.append(label)
        if current == parent["body"]:
            return None
        return current, labels

    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        while attempts < args.budget and (pending if args.exhaustive else beam):
            for parent in ([] if args.exhaustive else beam):
                parent_hash = digest(parent["body"])
                if parent_hash in expanded_hashes:
                    continue
                options = mutations(parent["body"], preserve_lines, source)
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
            while not args.exhaustive and len(batch) < generation and not pending:
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
                records.append({"index": item[0], "source": str(item[5]),
                                "mutations": item[4], "result": result})
                prior_best = min((entry["score"]["score"] for entry in scored), default=None)
                if result.get("compile") == "ok" and "score" in result:
                    entry = {"body": item[2], "text": item[3], "score": result,
                             "path": item[5], "mutations": item[4]}
                    scored.append(entry)
                    is_improvement = prior_best is None or result["score"] < prior_best
                else:
                    is_improvement = False
                if is_improvement or attempts % 100 == 0:
                    if result.get("score") is not None:
                        print(f"compile {attempts}/{args.budget}: score={result['score']} status={result['status']} "
                              f"prefix={result['identical_prefix']} via {','.join(item[4])}", flush=True)
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
    elapsed = max(time.perf_counter() - started, 1e-9)
    summary = {"tu": canonical, "candidate": str(candidate_path), "function": args.function,
               "mode": "exhaustive-one-step-and-explicit" if args.exhaustive else "beam-random",
               "lines": line_ranges, "operators": sorted(operators),
               "rva": f"0x{args.rva:08x}", "seed": args.seed, "budget": args.budget,
               "compiles_attempted": attempts, "compiled_and_scored": len(scored),
               "elapsed_seconds": round(elapsed, 3), "compiles_per_second": round(attempts / elapsed, 3),
               "baseline": baseline, "best": [{k: v for k, v in item["score"].items() if k not in ("relocations",)}
                                                | {"mutations": item["mutations"], "source": str(item["path"])}
                                                for item in best]}
    (output_root / "results.json").write_text(json.dumps(records, indent=2) + "\n", encoding="utf-8")
    (output_root / "summary.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    if ranked:
        top = ranked[0]["score"]
        print(f"best: {args.function} RVA {args.rva:#x} score={top['score']} status={top['status']} "
              f"exact={top['exact_matching_instructions']}/{top['oracle_instructions']} "
              f"prefix={top['identical_prefix']} compiles={attempts} "
              f"elapsed={elapsed:.1f}s rate={attempts / elapsed:.2f}/s")
        print(f"results: {output_root}")
        return 0
    print(f"no scoreable variants; baseline compile result: {baseline}")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
