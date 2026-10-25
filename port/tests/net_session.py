"""Two to four SB1 or SB2 instances in one native multiplayer session (port/docs/MULTIPLAYER.md).

A host and clients run headless with scripted input (tests/net/<game>_*.input) and --net-test-sync: this
supervisor grants each tick to both only after both finished the previous one, telling each how many
messages every member had sent by then, so each peer sees the same messages at the same tick in every run.
The test checks from each peer's network audit (user dir/network.trace) that they created and joined one
session, both reached PLAY with two members, each received the other's moves, and no packet was rejected.
It runs the scenario twice and requires each peer's frame trace and audit to repeat exactly. Every process
and wait is bounded; children are killed on failure. Never runs an original executable.
"""
import argparse
import os
from pathlib import Path
import shutil
import socket
import subprocess
import sys
import time

PLAY = 1525  # WM_PHASE_PLAY, in both games


def free_port():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def write_atomic(path, text):
    tmp = path.with_suffix(".tmp")
    tmp.write_text(text, encoding="ascii")
    os.replace(tmp, path)


def run(args, work, port):
    if work.exists():
        shutil.rmtree(work)
    control = work / "control"
    control.mkdir(parents=True)
    peers = []
    for role, name in enumerate(peer_names(args.players)):
        user = work / name
        user.mkdir()
        env = dict(os.environ, BLUPI_NET_TEST_DIR=str(control), BLUPI_NET_TEST_ROLE=str(role))
        command = [args.exe, "--headless", "--data", args.data, "--user", str(user), "--color", "16",
                   "--frames", str(args.frames), "--input", str(Path(args.inputs) / f"{args.game}_{name}.input"),
                   "--trace", str(work / f"{name}.trace"), "--net-test-sync", "--net-port", str(port)]
        for frame in args.state_dump:
            command += ["--state-dump", f"{frame}:{work / f'{name}-state-{frame}.txt'}"]
        log = open(work / f"{name}.log", "w", encoding="utf-8")
        peers.append((name, subprocess.Popen(command, env=env, stdout=log, stderr=subprocess.STDOUT), log))
    try:
        roles = range(args.players)
        for role in roles:
            write_atomic(control / f"{role}-grant-0", " ".join("0" * args.players) + "\n")
        deadline = time.monotonic() + args.timeout
        for tick in range(args.frames):
            done = [control / f"{role}-done-{tick}" for role in roles]
            while not all(d.exists() for d in done):
                if time.monotonic() > deadline:
                    raise RuntimeError(f"tick {tick}: timed out waiting for the peers")
                if any(p.poll() is not None for _, p, _ in peers) and not all(d.exists() for d in done):
                    if tick < args.frames - 1:
                        raise RuntimeError(f"tick {tick}: a peer exited early")
                    break
                time.sleep(0.001)
            if not all(d.exists() for d in done):
                break
            sent = [int(d.read_text(encoding="ascii").split()[0]) for d in done]
            # Role r joins as member id r + 1 (the scripts join in role order).
            for role in roles:
                write_atomic(control / f"{role}-grant-{tick + 1}", " ".join(map(str, sent)) + "\n")
        for name, process, log in peers:
            process.wait(timeout=max(1.0, deadline - time.monotonic()))
    finally:
        for name, process, log in peers:
            if process.poll() is None:
                process.kill()
                process.wait()
            log.close()
    return {name: process.returncode for name, process, _ in peers}


def audit(work, name):
    lines = (work / name / "network.trace").read_text(encoding="utf-8").splitlines()
    notes = [line.split(" ", 1)[1] for line in lines if " " in line]
    received = [line.split() for line in lines if line.split()[1:2] == ["receive"]]
    states = [line.split() for line in lines if line.split()[1:2] == ["state"]]
    return lines, notes, received, states


def peer_names(players):
    return ["host", "client", "client2", "client3"][:players]


def check(work, codes, players):
    failures = []
    for name, code in codes.items():
        if code != 0:
            failures.append(f"{name} exited {code} (see {work / (name + '.log')})")
    if failures:
        return failures
    names = peer_names(players)
    peers = {name: audit(work, name) for name in names}
    expect = {name: ("joined session",) for name in names}
    expect["host"] = ("created session", "START")
    for name, (lines, notes, received, states) in peers.items():
        for note in expect[name]:
            if not any(n.startswith(note) for n in notes):
                failures.append(f"{name}: no '{note}' in its audit")
        if any(n.startswith("rejected") for n in notes):
            failures.append(f"{name}: rejected packets: {[n for n in notes if n.startswith('rejected')]}")
        if not any(int(s[2]) == PLAY and len([x for x in s if x.startswith("member=")]) == players for s in states):
            failures.append(f"{name}: never in PLAY with {players} members")
        own = str(names.index(name) + 1)
        counts = []
        for other in (str(i + 1) for i in range(players) if str(i + 1) != own):
            moves = [r for r in received if r[2] == other and len(r) > 5 and r[5] == "01"]
            if not moves:
                failures.append(f"{name}: received no MOVE from member {other}")
            events = sum(int(r[23], 16) for r in moves if len(r) > 23)
            counts.append(f"{len(moves)} moves/{events} events from {other}")
        print(f"{name}: {', '.join(counts)}; {len(lines)} audit lines")
    return failures


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True)
    parser.add_argument("--data", required=True)
    parser.add_argument("--game", choices=("sb1", "sb2"), default="sb1", help="which game's input scripts")
    parser.add_argument("--inputs", required=True, help="directory with <game>_host.input, <game>_client.input, ...")
    parser.add_argument("--work", type=Path, required=True)
    parser.add_argument("--frames", type=int, default=700)
    parser.add_argument("--players", type=int, choices=(2, 3, 4), default=2)
    parser.add_argument("--timeout", type=float, default=240.0, help="seconds per run")
    parser.add_argument("--state-dump", type=int, action="append", default=[], metavar="FRAME",
                        help="diagnostic: each peer dumps its hashed state fields after FRAME")
    args = parser.parse_args()
    args.exe = str(Path(args.exe).resolve())
    args.work = args.work.resolve()
    port = free_port()
    results = []
    for run_index in (1, 2):
        work = args.work / f"run{run_index}"
        started = time.monotonic()
        codes = run(args, work, port)
        failures = check(work, codes, args.players)
        print(f"run {run_index}: {time.monotonic() - started:.1f} s")
        if failures:
            print("\n".join(failures))
            return 1
        results.append(work)
    for name in peer_names(args.players):
        for item in (f"{name}.trace", f"{name}/network.trace"):
            a, b = (w / item for w in results)
            if a.read_bytes() != b.read_bytes():
                print(f"not repeatable: {item} differs between runs")
                return 1
    print(f"{args.players}-instance session: create/join/lobby/ready/start/play passed; each peer's traces repeat exactly")
    return 0


if __name__ == "__main__":
    sys.exit(main())
