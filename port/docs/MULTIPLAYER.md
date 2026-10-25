# Native multiplayer (roadmap stage 10)

## Pilot status (2026-10-25, branch `multiplayer-pilot`)

[EVIDENCE] SB1 desktop, two instances on one machine, Windows (MinGW build):

- `net_sb1_two_instances` (`tests/net_sb1.py`, scripts in `tests/net/`): a host and a client, both headless
  and lockstepped by the supervisor (`--net-test-sync`), go through the original screens: choose player,
  the provider list ("Native network (UDP)"), sessions, create (host) / join (client), the lobby with two
  members, ready, start, then play. Each received 138 MOVE bundles from the other. Replicated embedded
  events: lift start/end synchro from the host (types 50/51, the original host-only lift coordination), an
  object delete, and sounds; lobby JOIN, READY, LOBBY and START. No packet was rejected. Run twice, each
  peer's frame trace and network audit repeat byte for byte (about 22 s per run).
- `net_transport`: handshake, member ids, reliable order, relay to a third member without echo,
  unsequenced delivery, protocol/version/full/after-start refusals, graceful disconnect events
  (relayed), bounded close, host loss.
- `net_sb1_protocol`: the validator's corpus of valid and malformed packets for every message kind.

[DESIGN] Options: `--net-port N` (listen and default probe port, 28640) and repeatable
`--net-join HOST[:PORT]` (more sessions to list). Test only: `--net-test-sync` with the supervisor's
`BLUPI_NET_TEST_DIR`/`BLUPI_NET_TEST_ROLE`; normal play never waits.

[OPEN] Not yet: LAN discovery and more than two players in a test, internet use (the validator's
coordinate margins are a pilot policy, to be reviewed before exposure), SB2 (`games/sb2/game/network.cpp`
stays the unavailable stub), Android, Linux receipts. The host-loss and client-drop paths through the game
(synthesized LEAVE/ABANDON, below) are tested at transport level only, not yet as a two-instance scenario.
Host migration is unsupported by design (below).

Goal: restore the original multiplayer of Speedy Blupi 1 and Speedy Blupi 2 by replacing DirectPlay
underneath the historical `CNetwork` interface. The reconstructed game logic (CEvent lobby and phases,
CDecor's `TreatNetData`, `NetMessage` producers in decnet/decmove/decblupi) stays as it is: the historical
TUs in `src/` and `games/sb2/src/` are never edited. Evidence for every protocol fact below is in the
assessment [MULTIPLAYER_ASSESSMENT.md](MULTIPLAYER_ASSESSMENT.md) (static, file:line); facts are [EVIDENCE], design choices
[DESIGN], unknowns [OPEN].

## What the original does

- [EVIDENCE] **Topology and authority.** DirectPlay 5 peer session (KEEPALIVE | MIGRATEHOST, at most four
  players). Every peer broadcasts its own Blupi's pose and the events it produced; the host runs the lobby
  table, the start, and the moving-object (lift) dwell boundaries. Neither input lockstep nor a
  host-authoritative world: each peer simulates locally and applies the others' poses and events.
- [EVIDENCE] **Reliability.** `CNetwork::Send` maps any nonzero flag to `DPSEND_GUARANTEED`. Lobby traffic
  (JOIN, LEAVE, LOBBY, TEAM, READY, START, CHAT), ABANDON/LOST/DIE/PAUSE and any MOVE bundle carrying a
  decor change, cloud stop or a non-transient object start/delete are guaranteed; ordinary MOVE bundles
  (pose, sounds, lift synchro, transient effects) are unreliable. Old MOVE poses are ignored by timestamp,
  but the events embedded in an old packet are still applied.
- [EVIDENCE] **Messages.** Lobby: JOIN 28 B, LEAVE 12, LOBBY 132, TEAM 12, READY 12, START 2, CHAT 108,
  CLOSE (receive-only). Gameplay: MOVE 20+8n (n <= 5 embedded 8-byte events), ABANDON 4, LOST 4, DIE 4,
  PAUSE 3. The two receive sites ask for 200 bytes (lobby, `CEvent::TreatEventBase`) and 60 bytes
  (gameplay, `CDecor::TreatNetData`).
- [EVIDENCE] **Host migration: requested from DirectPlay, never handled by the game.** No DirectPlay
  system message is dispatched; `IsHost` is cached at create/join and never changes. A host leaving the
  lobby gracefully closes the others' sessions (LEAVE with host = TRUE). Sudden losses are not detected by
  game code beyond pose prediction stopping after five calls.

## Port design

- [DESIGN] **Transport:** ENet 1.3.18 (MIT, `third_party/enet/`, unmodified), wrapped by
  `common/net/`: a small host/client session in a star (clients connect to the host; the host relays each
  client's payload to the other clients with its original origin id and never echoes it back). Channel 0
  is reliable and ordered per origin; channel 1 carries unreliable bundles unsequenced, so that late
  bundles still deliver their events as in the original. This topology changes routing, not authority.
- [DESIGN] **Envelope:** game protocol id (SB1 and SB2 differ and never interoperate), protocol version,
  session epoch, origin member id (the DPID the game sees; assigned by the host, nonzero), payload length.
  The member id comes from the connection, never from a client-written field.
- [DESIGN] **One game protocol adapter per game** (`games/sb1/game/network_protocol.*`, later sb2). It
  classifies a payload by (length, type) into the lobby or the gameplay namespace (unambiguous for SB1:
  lobby lengths 2/12/28/108/132, gameplay 3/4/20..60), and validates it atomically before `Receive`
  returns it: exact lengths, sender owns the DPIDs it names, slot/team/ready/map ranges, NUL-terminated
  names and chat, embedded event count and per-event ranges. Rejected packets are counted and logged,
  never passed on. `Receive(…, 200, …)` returns only lobby messages; `Receive(…, 60, …)` only gameplay
  messages; a gameplay message that arrives before the receiver has entered PLAY waits in its queue.
- [DESIGN] **Host migration:** unsupported, and reported ("host disconnected; host migration
  unsupported" in the log). The star topology ends with its host, so the adapter hands the game the
  original departure messages, generated by the trusted session layer (never accepted from the network):
  in the lobby the host's LEAVE with host = TRUE, which closes the session as a graceful host leave did
  (event.cpp); in play an ABANDON for the host and every other remote member, which removes them
  (decmove.cpp), so a remaining single player wins as in the original. A client that disconnects without
  ABANDON is announced by the host as that member's ABANDON (play) or LEAVE (lobby). Unlike DirectPlay's
  MIGRATEHOST, no new host is elected; the original game never acted on migration either (no system
  message is dispatched, `IsHost` never changes).
- [DESIGN] **Discovery:** the pilot lists the sessions it finds at configured endpoints (default
  `127.0.0.1` on the default port; `--net-join HOST[:PORT]` adds more); LAN broadcast discovery follows.
- [DESIGN] **Determinism for tests:** a test-only ingress mode releases received messages at tick
  boundaries in a fixed order, so two headless instances driven by scripts produce repeatable per-peer
  traces. Normal play never waits.

## Milestones

1. **Pilot (SB1 US, two local instances):** provider, create, enumerate, join, lobby names/colours/ready,
   start, play with remote poses and replicated events, pause, leave; malformed-packet rejection tests;
   two-instance regression test.
2. LAN, up to four players, with discovery.
3. Internet: direct IP and port first; NAT traversal or a relay only where needed.
4. SB2 adapter on the same transport (its own protocol id and validator).
5. Android: INTERNET permission, lifecycle handling, interoperability with the desktop for the editions
   the app supports.
