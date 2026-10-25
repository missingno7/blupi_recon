# netassess — bounded multiplayer protocol and transport assessment

Date: 2026-10-25. Repository: D:\Prog\blupi_recon. Branch pointer inspected: .git/HEAD:1, refs/heads/sb2-matching.

## Scope and recommendation

[EVIDENCE] Static assessment only. No build, compilation, game, original executable, installer, or ISO program was run. Files were read through cat or rg; shell filtering only selected rg output. Only notes/report under scratch/w-netassess/ were written. No tracked file was edited. No tests are claimed. A tool-location search was stopped and its process exited; no background process was left running.

[HYPOTHESIS / recommendation] Use **ENet**, pinned and unmodified under port/third_party/, behind a small common session/transport service. Start with **SB1 Blupi US, two desktop loopback instances**, preserving the provider/session/lobby UI and unchanged historical CEvent/CDecor logic. Exercise create, join, names, colours, ready, start, movement, one replicated gameplay mutation, pause, and leave. Keep SB2 unavailable until its own adapter/tests land. No general framework, input lockstep, rollback, or replacement physics is needed.

[EVIDENCE] SB1 directly shares 12 historical TUs, including event/decmove/decnet (port/CMakeLists.txt:46-49). SB2 shares 11 plus a port-owned event derivative (port/games/sb2/CMakeLists.txt:25-31). Port rules prohibit historical edits (port/AGENTS.md:10-22); source identity is not runtime equivalence (port/docs/FIDELITY.md:3-23). Stage10 specifies assessment -> two local instances -> LAN4 -> internet -> Android, preserving algorithms and documenting migration support/absence (port/docs/ROADMAP.md:606-650).

Notation: **S1** = src/, **S2** = games/sb2/src/. Thus S1/event.cpp:994 means src/event.cpp:994. Source ranges refer to the inspected checkout. **R** means nonzero send flags (guaranteed); **U** means zero. Roles mean intended send behaviour, not legacy enforcement. Recommendations are [HYPOTHESIS], unknown evidence [OPEN]. Calculated struct sizes are static derivations, not compiled observations.

## 1. CNetwork contract and complete call-site census

### Public contract, identical in SB1 and SB2

[EVIDENCE] Both headers declare these methods and public m_dpid/m_players fields at network.h:37-69. The implementation function line numbers and behaviour below apply to **both** S1/network.cpp and S2/network.cpp. Enumeration callbacks alone differ in linkage: SB1 static at network.cpp:89,160 / network.h:72-76; SB2 external at the same locations.

| Public signature | Historical operation, results, failures; line evidence in each network.cpp |
|---|---|
| CNetwork() | :67-78 sets DP pointer NULL, DPID 0, host FALSE, three list counts0/pointers NULL. Does **not** initialise m_players. |
| ~CNetwork() | :80-86 frees three lists, releases DP interface if non-NULL; no explicit DestroyPlayer/Close. |
| BOOL EnumProviders() | :102-119 frees list, allocates100 NamedGUIDs, DirectPlayEnumerate (ANSI normally). FALSE allocation/API error, frees on API error; TRUE even zero results. Callback :89-100 copies GUID and strcpy name, caps100, returns TRUE. |
| int GetNbProviders() | :121-124 returns count. |
| char* GetProviderName(int index) | :126-130 NULL if index>=count, else internal pointer until list free. Negative index unchecked. |
| BOOL CreateProvider(int index) | :132-149 rejects index>=count; DirectPlayCreate(GUID), QueryInterface(IID_IDirectPlay2A); releases temporary interface. TRUE iff query succeeds, otherwise FALSE. Negative index unchecked; no release of a previously stored provider. |
| void FreeProviderList() | :151-157 frees/reset count/pointer. |
| BOOL EnumSessions() | :176-198 frees/allocates list, zeroes DPSESSIONDESC2, sets size/APP_GUID, EnumSessions(timeout0, AVAILABLE). FALSE allocation/API error (API failure frees); TRUE including empty. Requires non-NULL provider. Callback :160-174 stops on TIMEDOUT, copies instance GUID/name with strcpy, caps100. |
| int GetNbSessions() | :200-203 returns count. |
| char* GetSessionName(int index) | :205-209 same pointer/index contract as provider name. |
| BOOL JoinSession(int index, char* pPlayerName) | :211-247 rejects index>=count; Open(instance GUID, DPOPEN_OPENSESSION=JOIN), error logs/FALSE. CreatePlayer(&m_dpid, shortName=argument, longName=NULL, no event/data/flags), failure logs/Close/FALSE. Success host FALSE/TRUE. Negative index/provider NULL unchecked. |
| void FreeSessionList() | :249-255 frees/reset. |
| BOOL CreateSession(char* pSessionName, char* pPlayerName) | :257-296 Open(CREATE), KEEPALIVE|MIGRATEHOST, APP_GUID, max players4, supplied session name. Failure logs/FALSE. CreatePlayer as join; failure logs/Close/FALSE; success host TRUE/TRUE. No password/join-disable. |
| BOOL Send(LPVOID lpData, DWORD dwDataSize, DWORD dwFlags) | :298-313 FALSE if no DP; any nonzero flags collapse to GUARANTEED, zero stays zero. Send(m_dpid,0=ALLPLAYERS,flags,data,length); error logs/FALSE, DP_OK TRUE. No directed-send API. |
| BOOL Receive(LPVOID pDest, DWORD dwDataSize, LPDWORD lpdwPlayer) | :315-342 Receive(ALL) into500-byte stack buffer. Any error FALSE; only NOMESSAGES avoids log. On success memcpy **requested size**, ignoring actual dataSize. Output DWORD(-1), then present-player lookup by **sender DPID** returns slot0..3. TRUE even unknown/system sender. No returned actual length/raw sender; NULL DP/capacity unchecked. |
| BOOL Close() | :344-349 DP Close; TRUE iff DP_OK; no log. Does not clear DPID/host/provider/player table. Requires DP. |
| void FreeUnknownList() | :351-357 frees/reset unused third list. |
| BOOL IsHost() | :359-362 returns cached host; no election/system processing. |

[EVIDENCE] APP_GUID is the same in both: be4af9a0-c311-11d1-be62-0040f6944838 (both network.cpp:13-14). This does not establish cross-play: map/type differences below require separate portable game protocol IDs.

### All direct non-message calls in game TUs

[EVIDENCE] Exhaustive method-name/g_pNetwork searches over S1/S2 .cpp files found these calls. Event callers are CEvent methods; lift caller is CDecor::MoveObjectStepLine. Repeated cleanup calls are listed individually.

| Method/caller, phase and arguments | SB1 file:line | SB2 file:line |
|---|---|---|
| new CNetwork in DoInit, failure InitFail; passed into Decor/Event Create | blupi.cpp:918-928 | blupi.cpp:792-802 |
| delete g_pNetwork during cleanup; set NULL | blupi.cpp:559-562 | blupi.cpp:606-609 |
| EnumProviders() entering SERVICE; return ignored | event.cpp:2544 | event.cpp:3156 |
| GetNbProviders() SERVICE entry; DrawButtons/provider list | event.cpp:2545,1559 | event.cpp:3157,2169 |
| GetProviderName(m_filePageOffset+i), six DrawButtons rows | event.cpp:1563 | event.cpp:2173 |
| CreateProvider(provider) in NetCreate; called with m_fileIndex by DOSERVICE | event.cpp:874,2130 | event.cpp:1453,2749 |
| FreeProviderList() DOSERVICE success, fallthrough, CANCELSERVICE | event.cpp:2132,2135,2140 | event.cpp:2751,2754,2759 |
| EnumSessions() in NetEnumSessions; mouse release/capture around call | event.cpp:884 | event.cpp:1463 |
| All NetEnumSessions invocations: DOSERVICE; failed JOIN; REFRESH; failed DOCREATE; CANCELCREATE; CANCELMULTI; received host LEAVE; received CLOSE | event.cpp:2133,2153,2164,2180,2187,2206,4174,4189 | event.cpp:2752,2772,2783,2799,2806,2825,4846,4861 |
| GetNbSessions() DrawButtons/SESSION; SESSION entry | event.cpp:1581,2572 | event.cpp:2191,3184 |
| GetSessionName(m_filePageOffset+i), six rows | event.cpp:1585 | event.cpp:2195 |
| JoinSession(m_fileIndex,m_name), JOIN | event.cpp:2145 | event.cpp:2764 |
| FreeSessionList(): JOIN success/failure, REFRESH, CANCELSESSION, DOCREATE | event.cpp:2147,2152,2163,2171,2177 | event.cpp:2766,2771,2782,2790,2796 |
| CreateSession(m_textInput,m_name), DOCREATE | event.cpp:2178 | event.cpp:2797 |
| Close(): leaving permitted multiplayer phases; QUITPLAY; CANCELMULTI; received host LEAVE; received CLOSE | event.cpp:2085,2121,2205,4173,4188 | event.cpp:2698,2740,2824,4845,4860 |
| IsHost(): lobby buttons; NetStartPlay; CANCELMULTI payload; MULTI entry; own colour; own ready; received JOIN; received LEAVE | event.cpp:955,1026,2203,2604,3213,3232,4110,4158 | event.cpp:1540,1611,2822,3216,3796,3815,4782,4830 |
| IsHost() moving-object start/end dwell boundaries | decmove.cpp:522,571 | decmove.cpp:573,622 |
| FreeUnknownList() | destructor only network.cpp:84; no game-TU call | same |

### Every Send and Receive call

[EVIDENCE] All Send results are ignored. Receive FALSE stops polling, without distinguishing empty/error/disconnection.

| Caller/payload/phase | Arguments | SB1 | SB2 |
|---|---|---|---|
| CEvent::ChangePause, multiplayer pause | &data,3,GUARANTEED | event.cpp:994 | event.cpp:1579 |
| NetSendLobby | data,132,GUARANTEED | event.cpp:1006 | event.cpp:1591 |
| NetStartPlay, host START | data,2,GUARANTEED | event.cpp:1030 | event.cpp:1615 |
| CEvent::NetSendBarePacket, QUITPLAY ABANDON | &packet,4,GUARANTEED | event.cpp:1058, caller:2120 | event.cpp:1643, caller:2739 |
| ChangePhase CANCELMULTI, LEAVE | netData,12,GUARANTEED | event.cpp:2204 | event.cpp:2823 |
| ChangePhase MULTI entry, nonhost JOIN | data,28,1 (=guaranteed) | event.cpp:2621 | event.cpp:3233 |
| TreatEvent colour button, nonhost TEAM | netData,12,GUARANTEED | event.cpp:3223 | event.cpp:3806 |
| TreatEvent ready button, nonhost READY | netData,12,GUARANTEED | event.cpp:3242 | event.cpp:3825 |
| ChatSend, nonempty lobby text | data,108,GUARANTEED | event.cpp:3901 | event.cpp:4573 |
| TreatEventBase lobby poll | &data,200,&playerId; at most one/event dispatch | event.cpp:4106 | event.cpp:4778 |
| CDecor::NetSendBarePacket, DIE/LOST | &data,sizeof(data)=4,GUARANTEED | decmove.cpp:39 | decmove.cpp:39 |
| TreatNetData movement/event bundle | &pack,20+8*nbMessages,flag(0/GUARANTEED) | decmove.cpp:109 | decmove.cpp:123 |
| TreatNetData gameplay poll | &pack,sizeof(pack)=60,(LPDWORD)&player; up to10/invocation | decmove.cpp:116-118 | decmove.cpp:130-132 |

## 2. Layouts and complete protocol tables

[EVIDENCE / static derivation] Both decor.h:53-74 define **NetMessage=8 bytes**: type char@0; signed char data1/speed@1; shorts data2/posX@2,data3/posY@4,data4/type/sound/lift/cloud/icon@6. Both decor.h:76-97 define **NetPacket=60 bytes**: size char@0,type char@1,short keyPress/bPause/nbVies@2,int32 time@4; shorts x@8,y@10,icon@12,sec@14; chars channel@16,transport@17,flag@18,nbMessages@19; five8-byte messages@20. MAXMESSAGEPERPACKET5, MAXNETMESSAGE20: S1/def.h:24-35,S2:22-33.

[EVIDENCE] MOVE puts60 in size but sends **20+8*n**: lengths20,28,36,44,52,60; do not compare size byte with transport length for MOVE. Bare gameplay packet is4 bytes(size,type,short), both decmove.cpp:27-39. CEvent PAUSE instead sends **3 bytes**, boolean byte@2 (S1/event.cpp:977-994,S2:1562-1579); receive casts padded bPause short back to char (S1/decmove.cpp:221-224,S2:235-238).

[EVIDENCE / static derivation] NetPlayer=32 bytes: present@0,ready@1,unknown chars@2/3,DPID u32@4,team short@8,name[22]@10. Four=128 bytes, LOBBY=4+128=132 (both network.h:11-21; S1/event.cpp:998-1006). Many unused bytes are uninitialised; they must not become required zero-reserved fields. Size132 occupies a signed char but is an unsigned wire byte.

### SB1 table

All rows [EVIDENCE]. Codes S1/def.h:705-725. Embedded events share the reliability of their entire MOVE bundle; any R-trigger makes the whole bundle R.

| Namespace/code/name | Length/body after size/type | Intended origin -> recipients; trigger/rate | Flags and receive dispatch, S1 evidence |
|---|---|---|---|
| lobby1 CLOSE | no sender found; receive needs byte1 only; historical send layout unknown | receive-only session close | send flag unestablished; event.cpp:4186-4192 |
| lobby2 JOIN | 28; DPID@4,name[20]@8,NUL@27 | nonhost -> all, once MULTI entry | R(1); event.cpp:2615-2621 -> :4108-4127 host first free slot, readyFALSE,team=slot,strcpy name, broadcasts LOBBY |
| lobby3 LEAVE | 12; DPID@4,host bool@8 | leaving peer -> all, CANCELMULTI | R; :2197-2206 -> :4156-4178 host clears presence; host=true closes recipients |
| lobby4 LOBBY | 132; map short@2,NetPlayer[4]@4 | host -> all, JOIN/own colour/ready/map change | R; :998-1006,:3215,:3234,:4707,:4730 -> :4129-4135 copies table,loads map+100 |
| lobby5 TEAM | 12; DPID@4,team short@8 | nonhost -> all, own colour click; immediate local update | R; :3202-3226 -> :4137-4145 lookup DPID,set team; host own change sends LOBBY |
| lobby6 READY | 12; DPID@4,ready byte@8 | nonhost -> all, own toggle; immediate local update | R; :3228-3244 -> :4147-4154; host own toggle sends LOBBY |
| lobby7 START | 2 | host -> all, enabled START | R; :1019-1048 -> :4180-4184 NetStartPlay,PLAY |
| lobby11 CHAT | 108; DPID@4,NUL text[100]@8 | any -> all, send/Enter; immediate local echo | R; :3882-3906 -> :4194-4196 ChatPush |
| gameplay1 MOVE | 20+8*n; header+0..5 events | every peer -> all, pose/icon change or queued event; optionally even tick only | U/escalated; decmove.cpp:59-109 -> :123-186 |
| gameplay8 ABANDON | 4; short0 | peer -> all, quit gameplay | R; event.cpp:2120,1051-1058 -> decmove.cpp:188-195 notify,presentFALSE,icon=-1 |
| gameplay9 LOST | 4; short0 | peer -> all, exhausted lives/fall | R; decblupi.cpp:3518,3570 -> decmove.cpp:196-203 remove/notify |
| gameplay10 DIE | 4; remaining lives short | peer -> all, death/respawn | R; decblupi.cpp:3492-3511 -> decmove.cpp:204-219 notify |
| gameplay12 PAUSE | 3; bool byte@2 | any peer -> all, multiplayer ChangePause(...,TRUE) | R; event.cpp:977-994 -> decmove.cpp:221-224 |
| embedded20 OBJECTSTART | 8; signed speed@1,x/y@2/4,type@6 | producing peer -> all, ObjectStart(...,TRUE), projectiles/effects/vehicles | R except transient list; decnet.cpp:20-99 -> decmove.cpp:163-164 ObjectStart(...,FALSE) |
| embedded21 OBJECTDELETE | 8; x/y,type,data1=0 | producing peer -> all, ObjectDelete(...,TRUE) | same escalation; decnet.cpp:103-121 -> decmove.cpp:165-166 |
| embedded30 MODIFDECOR | 8; pixel x/y,tile icon(-1 allowed) | mutating peer -> all, tile change | R; decnet.cpp:126-140 -> decmove.cpp:167-168 |
| embedded40 PLAYSOUND | 8; pixel x/y,sound ID@6,data1=0 | peer -> all, nonlocal PlaySound | U unless bundled R; decblock.cpp:1772,decnet.cpp:144-153 -> decmove.cpp:169-174 |
| embedded41 STOPSOUND | 8; sound ID@6,others0 | peer -> all, mockery stop | U/bundledR; decblupi.cpp:3619,3624; decnet.cpp:156-165 -> decmove.cpp:175-176 |
| embedded50 ASCENSEURSTART | 8; object index@6,x/y0,data1 unset | **host** -> all, moving object start dwell time==0 | U/bundledR; decmove.cpp:520-528 -> :177-178 NetAscenseurSynchro(STOPSTART) |
| embedded51 ASCENSEUREND | 8; object index@6,x/y0,data1 unset | **host** -> all, end dwell time==0 | U/bundledR; decmove.cpp:569-577 -> :179-180 NetAscenseurSynchro(STOPEND) |
| embedded60 STOPCLOUD | 8; rank@6,others0 | collision peer -> all | R; decblupi.cpp:3302-3307,decnet.cpp:8-17 -> decmove.cpp:181-185 rank compared with local team, clears cloud/gauge |

[EVIDENCE] SB1 START/DELETE do not escalate for TYPE_EXPLO1..4,PLOUF,BLUP,MAGICTRACK,TIPLOUF,POLLUTION (decmove.cpp:86-100). MODIFDECOR/STOPCLOUD always escalate (:102-106). Sound/lift do not themselves escalate. Queue Push failure at :344-351 is ignored by producers in decnet.cpp; individual events are not periodically resent by those producer functions.

### SB2 table

All rows [EVIDENCE]. Codes S2/def.h:703-723; same struct shapes at S2/decor.h:53-97.

| Namespace/code/name | Length/body | Origin/trigger/rate -> all | Flags and S2 dispatch |
|---|---|---|---|
| lobby1 CLOSE | no sender; receive needs byte1, size unknown | receive-only close | send flags unknown; event.cpp:4858-4864 |
| lobby2 JOIN | 28;DPID@4,name[20]@8 | nonhost MULTI entry | R; event.cpp:3227-3233 -> :4780-4798 slot/team allocation,LOBBY |
| lobby3 LEAVE | 12;DPID@4,host@8 | peer CANCELMULTI | R; :2816-2825 -> :4828-4850 |
| lobby4 LOBBY | 132;map short@2,players[4]@4 | host join/own changes/map | R; :1583-1591,:3798,:3817,:5392,:5415 -> :4801-4807 map+200 |
| lobby5 TEAM | 12;DPID@4,team short@8 | nonhost colour click | R; :3785-3809 -> :4809-4817 |
| lobby6 READY | 12;DPID@4,ready byte@8 | nonhost ready toggle | R; :3811-3827 -> :4819-4826 |
| lobby7 START | 2 | host START | R; :1604-1633 -> :4852-4856 |
| lobby11 CHAT | 108;DPID@4,text[100]@8 | any send/Enter; local echo | R; :4555-4578 -> :4866-4868 |
| gameplay1 MOVE | 20+8*n | every peer pose/icon change/queue; optionally even tick | U/escalated; decmove.cpp:59-123 -> :137-200 |
| gameplay8 ABANDON | 4;short0 | peer quit | R; event.cpp:2739,1636-1643 -> decmove.cpp:202-209 |
| gameplay9 LOST | 4;short0 | exhausted lives/fall | R; decblupi.cpp:4093,4156 -> decmove.cpp:210-217 |
| gameplay10 DIE | 4;lives short | death/respawn | R; decblupi.cpp:4086 -> decmove.cpp:218-233 |
| gameplay12 PAUSE | 3;bool byte | any multiplayer pause | R; event.cpp:1562-1579 -> decmove.cpp:235-238 |
| embedded20 OBJECTSTART | 8;speed,x,y,type | producing peer ObjectStart(...,TRUE) | R except larger transient list; decnet.cpp:48-127 -> decmove.cpp:177-178 |
| embedded21 OBJECTDELETE | 8;x,y,type | producing peer ObjectDelete | same escalation; decnet.cpp:131-151 -> decmove.cpp:179-180 |
| embedded30 MODIFDECOR | 8;pixel x,y,icon | tile changes, including switches | R; decnet.cpp:154-169; decblupi.cpp:4825,4834,5301,5310 -> decmove.cpp:181-182 |
| embedded40 PLAYSOUND | 8;x,y,sound ID | nonlocal PlaySound | U/bundledR; decblock.cpp:2378,decnet.cpp:172-181 -> decmove.cpp:183-188 |
| embedded41 STOPSOUND | 8;sound ID | mockery/ouf stops | U/bundledR; decblupi.cpp:4205,4210,4215; decnet.cpp:184-193 -> decmove.cpp:189-190 |
| embedded50 ASCENSEURSTART | 8;object index | host start dwell time==0 | U/bundledR; decmove.cpp:573-579 -> :191-192 |
| embedded51 ASCENSEUREND | 8;object index | host end dwell time==0 | U/bundledR; decmove.cpp:622-628 -> :193-194 |
| embedded60 STOPCLOUD | 8;rank | collision peer | R; decblupi.cpp:3843,decnet.cpp:8-17 -> decmove.cpp:195-199 compares to team |

[EVIDENCE] SB2's non-escalating START/DELETE list: TYPE_EXPLO1..10,SPLOUTCH1..3,TENTACULE,PLOUF,BLUP,MAGICTRACK,SHIELDTRACK,TRESORTRACK,TIPLOUF,INVERTSTART,INVERTSTOP,POLLUTION (decmove.cpp:86-115); MODIFDECOR/STOPCLOUD escalate :116-120. StartSploutchGlu produces seven object events plus sound (decnet.cpp:20-45): useful burst test.

### DirectPlay system traffic: neither game properly dispatches it

[EVIDENCE] SDK sender DPID_SYSMSG=0 and destination ALLPLAYERS=0: C:/tools/dx5sdk/INCLUDE/dplay.h:85-95. System payload starts with DWORD dwType, not game size/type bytes (:1128-1132).

| System message | SDK definition | Game handling, both |
|---|---|---|
| CREATEPLAYERORGROUP0x0003 | dplay.h:1136-1141 | No case/DPMSG cast; game uses JOIN/LOBBY. |
| DESTROYPLAYERORGROUP0x0005 | :1143-1147 | No case; explicit LEAVE/ABANDON/LOST instead. |
| SESSIONLOST0x0031 | :1161-1166 | No case/error-to-phase mapping. |
| HOST0x0101 | :1168-1173 | No case/host flag update. Little-endian byte1 is1, accidentally aliases lobby CLOSE or gameplay MOVE. |

[EVIDENCE / bounded negative search] No DPSYS_/DPID_SYS/DPMSG dispatch found in either historical .cpp/.h tree. Wrapper network.cpp:315-342 does not separate system sender; consumers inspect byte1 (S1/event.cpp:4106-4197,decmove.cpp:123-224; S2/event.cpp:4778-4869,decmove.cpp:137-238). Most small system types have byte1==0 and are accidentally ignored. HOST aliases a real type, while wrapper copies beyond its actual length. This is a static hazard, **not an observed original-runtime result**.

## 3. Session, lobby and gameplay flow

[EVIDENCE] Both headers' **effective final phase numbers** are the later undef/define block, not the earlier inherited definitions: S1/def.h:1165-1295; S2:1163-1293. IDs below are shared by both games.

| Phase/action ID | Flow / CNetwork operations | Evidence S1/event.cpp / S2/event.cpp unless noted |
|---|---|---|
| GAMER1545 / NAMEGAMER1527 | Existing profile m_name feeds join/create. Network names limited to19 chars+NUL in lobby setup. | :2145,2178,2610-2620 / :2764,2797,3222-3232 |
| SERVICE1568 | EnumProviders/count/selected index0; six name rows; disables confirm when empty. | :2542-2568,1559-1563 / :3154-3180,2169-2173 |
| DOSERVICE1569 / CANCELSERVICE1570 | CreateProvider(selection), free list; enumerate -> SESSION or SERVICE. Cancel frees -> GAMER. | :2128-2141 / :2747-2760 |
| SESSION1571 | Count/names/six rows, select; JOIN disabled if no sessions. | :2570-2595,1581-1585 / :3182-3207,2191-2195 |
| JOIN1572 | JoinSession(index,m_name), free list; success MULTI; failure enumerates -> SESSION/SERVICE. | :2143-2154 / :2762-2773 |
| CREATELOBBY1573 -> CREATE1579 | Clear input, type session name. | :2157-2160,2634-2640 / :2776-2779,3246-3252 |
| REFRESH1574 / CANCELSESSION1575 | Free list; refresh enumerates; cancel returns SERVICE. | :2161-2172 / :2780-2791 |
| DOCREATE1580 / CANCELCREATE1581 | Reject empty session name; CreateSession(text,m_name) -> MULTI; failure/cancel enumerates. | :2174-2190 / :2793-2809 |
| MULTI1576 | Read preview, clear present flags; host slot0/team0/not ready/name; nonhost JOIN; host fills first absent slot team=slot and broadcasts LOBBY. | :2597-2632,4110-4135 / :3209-3244,4782-4807 |
| MULTI colour/ready/map/chat | Own colour enabled only while not ready; own ready enabled. Teams cycle0..3, not unique. Host own changes/map -> LOBBY, clients -> TEAM/READY; all can chat. | :915-974,3202-3248,4700-4730 / :1500-1559,3785-3831,5386-5415 |
| STARTMULTI1577 -> PLAY1525 | Host start enabled only all present ready and >1 ready. NetStartPlay sends START if host, finds own DPID/team, sets multi flags; enters PLAY. Receiver does same without host send. | :955-967,1019-1048,2192-2195,4180-4184 / :1540-1552,1604-1633,2811-2814,4852-4856 |
| PLAY1525 | ReadInput while m_bMulti sends one bundle/drains<=10 before local MoveStep. Normal50ms tick; packed every other m_time. | :781-784; S1/blupi.cpp:60,204-220,decmove.cpp:59-118 / :1360-1363; S2/blupi.cpp:63,decmove.cpp:59-132 |
| STOP1582, SETUPp1541, HELP1583, QUITPLAY1587, save phases | Connection retained through these overlays; multiplayer save/load UI disabled. ReadInput still pumps before phase-dependent movement. | :2075-2088,2647-2650 / :2688-2701,3259-3262 |
| CANCELMULTI1578 | R LEAVE(DPID,IsHost), Close, enumerate -> SESSION/SERVICE; host=true closes other lobbies. | :2197-2207,4156-4178 / :2816-2826,4828-4850 |
| QUITPLAY1587 | R ABANDON(0), Close, clear multi, -> GAMER. | :2118-2124 / :2737-2743 |
| LOSTMULTI1549 / WINMULTI1550 / WINMOVIEMULTI1551 and other departures | Leaving preserved phase set closes connection/clears multi. Gameplay ABANDON/LOST removes peer; <2 present triggers local victory. | :2075-2088; S1/decblupi.cpp:3521-3551 / :2688-2701; S2/decblupi.cpp:4096-4126 |

[EVIDENCE] Limits:4 players,4 teams,100 session/provider results (S1/def.h:24-28; S2:22-26; both network.cpp:264-269). No team uniqueness rule. Slot,DPID,team differ. SB1 map choices1..10 load map+100 (S1/event.cpp:961-964,2599,4703-4730); SB2 choices1..12 load map+200 (S2/event.cpp:1546-1549,3211,5388-5415). Send census contains no level-file transfer.

[EVIDENCE] Start exchanges no map hash or load acknowledgement (S1/event.cpp:1019-1048; S2:1604-1633). PlayPrepare sets multiplayer lives10 and starts at team-indexed positions (S1/decblock.cpp:971-985; S2:1344 onward). SetMulti initialises local/remote characters from team (S1/decor.cpp:203-238; S2:484-519), indexing start-direction arrays before the limited channel check. [HYPOTHESIS] Compatible local map data and definitions are assumed.

## 4. Synchronisation, loss, ordering and determinism

[EVIDENCE] **Peer broadcasts of locally owned poses/events, with host lobby/start and lift coordination**, neither input lockstep nor host-authoritative world simulation. Receive does not replay MOVE.keyPress: it assigns remote pose/icon/effect/channel/transport, derives velocity, applies embedded events. S1/decmove.cpp:67-109,128-185; S2:67-123,142-199. Local physics proceeds without global barrier (S1/blupi.cpp:208-220).

[EVIDENCE] Defaults: netpacked FALSE/prediction TRUE (S1/decblock.cpp:779-780; S2:1161-1162). At50ms, continuous movement can send20 packets/s or about10/s packed. These are calculated upper rates under default settings, not measurements or heartbeats. No send if position/icon unchanged and queue empty, even when another header field alone changes. One send before MoveStep per frame, irrespective of speed multiplier (S1/blupi.cpp:60,208-220; both decmove.cpp:59-78).

[EVIDENCE] Newer timestamp updates pose; old/equal timestamp does not. **Embedded events in old packets still execute**, even if sender lookup failed (S1/decmove.cpp:128-186; S2:142-200). These routines contain no general event IDs/de-duplication, rollback, input acknowledgement, periodic full world snapshot or repair.

[EVIDENCE] Missing pose: DoNetSmooth extrapolates using velocity/TestPath for at most5 calls, then stops. NetAdjustToLift only uses index0..199 (S1/decmove.cpp:227-265; S2:241-279). Lost U effects/sounds/lift events are not individually repaired here; later lift boundaries may correct lift timing. Queue20/drain5 per frame can drop events before transport because Push FALSE is ignored by producers (S1:337-361; S2:351-375; decnet.cpp producers).

[EVIDENCE] Ordering dependencies include JOIN->LOBBY->START, team/ready changes, object create/delete and decor changes, without general application event sequencing. SDK GUARANTEED is dplay.h:1066-1070. Microsoft's later DirectPlay4 specification documents default reliable order: [DPSESSIONDESC2](https://learn.microsoft.com/en-us/openspecs/windows_protocols/mc-dpl4cs/8743fe84-59ab-4e98-b0a0-362aa8ce9b1d). [OPEN] DX5 header alone does not establish every provider's cross-class ordering, blocking or timeout semantics; do not substitute DirectPlay8 guarantees or claim global cross-sender order. [HYPOTHESIS] Keep one reliable ordered stream per original sender across lobby/start/gameplay.

[EVIDENCE] No RNG seed/tick barrier in message inventory. InitRandom seeds1 (both misc.cpp:93-96); port reproduces Microsoft LCG (port/common/compat/src/win32.cpp:222-234). PlayPrepare randomises some object phases (S1/decblock.cpp:1021-1039; S2:1404-1422); local state/camera and RNG histories may differ. Fixed seed/input/tick/ingress makes a given peer's repeat run testable; it does not imply identical full states across peers.

[EVIDENCE] Both clocks wrap0 at32768 (S1/decblock.cpp:1545-1546; S2:2042-2043; port/games/sb1/game/end_of_tick.cpp:113-115). Pose acceptance uses plain time>previous (S1/decmove.cpp:130; S2:144). [HYPOTHESIS] Poses may stop updating after wrap for much of the next cycle: normal cycle27m18.4s. Embedded events still run. Test accelerated rollover; no silent timestamp rewriting without port fidelity disposition.

[EVIDENCE] Cloud-stop producer passes player **slot** from collision (S1/decblupi.cpp:3302-3307; S2:3838-3843), receiver compares to local **team** (S1/decmove.cpp:181; S2:195). [HYPOTHESIS] Team!=slot exposes a historical targeting defect; distinguish it from transport failures.

## 5. Host migration and disconnects

[EVIDENCE] Both sessions set exactly KEEPALIVE|MIGRATEHOST (network.cpp:264-269). SDK KEEPALIVE detects abnormal remote exits (dplay.h:236-240); MIGRATEHOST attempts transport host migration so joins/player creation continue (:213-219). No CLIENTSERVER/MULTICASTSERVER/JOINDISABLED/NEWPLAYERSDISABLED is set, and neither network.cpp later changes session flags.

[EVIDENCE] Application authority migration is missing: cached host assigned only constructor/join/create (both network.cpp:71,244,292), not cleared by historical Close (:344-348), never updated on HOST. Host-only lift production/lobby controls therefore do not elect a replacement. [OPEN] Real original-provider behaviour was not captured or executed; requested DP migration is not proof that the game continues correctly.

[EVIDENCE] Graceful lobby host leave deliberately closes recipients via LEAVE.host=true (S1/event.cpp:2203,4171-4177; S2:2822,4843-4849). Client LEAVE clears presence only on host; branch does not broadcast fresh LOBBY (S1:4156-4169; S2:4828-4841), so third/fourth peers may retain stale slots until another snapshot. Gameplay ABANDON/LOST removes sender/character (S1/decmove.cpp:188-203; S2:202-217). Sudden loss has no explicit game timeout/removal; prediction stops after5 calls. Fewer than2 present triggers victory.

[HYPOTHESIS / pilot policy] Clearly advertise “Host migration unsupported; host disconnect ends the session.” Graceful leave flushes reliable LEAVE/ABANDON before Close within a deadline; Send TRUE means queued, not remote application acknowledgement. Unexpected client disconnect uses a trusted adapter-generated leave/abandon transition for the mapped slot; host loss ends the session through explicit port-side error/phase handling. Never feed DP-shaped system bytes to legacy parsing.

[HYPOTHESIS] Later migration needs stable IDs, epoch/election/reconnection and explicit application host/lift ownership transfer. ENet does not provide this for the game. It is outside pilot.

## 6. Transport assessment and reference repositories

External facts checked against primary upstream sources on2026-10-25. Links were read, not executed; no code copied.

| Option | Evidence / implications | Assessment |
|---|---|---|
| UDP + own reliable ordered channel | [HYPOTHESIS] Small payloads/4 peers make sockets simple, but ack/retry windows, sequencing, duplicate suppression, wrap, congestion/backpressure, heartbeat and portability become repository maintenance. | Avoid unless dependency avoidance outweighs correctness work. UDP alone cannot satisfy existing R sends. |
| ENet | [EVIDENCE] Optional reliability, sequencing, independent channels, retransmission/connection handling, Windows/Unix portability: [upstream design](https://github.com/lsalzman/enet/blob/master/docs/design.dox). MIT requires retaining notice: [LICENSE](https://github.com/lsalzman/enet/blob/master/LICENSE). | [HYPOTHESIS] **Recommended**: use tested transport machinery, keep CNetwork/session rules ours. Pin reviewed revision, vendor unmodified, record URL/hash/licence and ship notice. No ENet API in historical TUs. |
| SDL_net3 | [EVIDENCE] Upstream3.2.0 official release2026-05-31 followed3.1 prerelease; rewritten async stream/datagram API, IPv4/IPv6, zlib licence: [release](https://github.com/libsdl-org/SDL_net/releases/tag/release-3.2.0), [README](https://github.com/libsdl-org/SDL_net), [API](https://wiki.libsdl.org/SDL3_net/FrontPage). | [HYPOTHESIS] Useful socket portability, but UDP still needs reliability/session work; TCP needs framing and has stream head-of-line behaviour. TCP+UDP adds coordination. Existing SDL dependency does not remove this work. |

[EVIDENCE] ENet default unreliable mode discards late lower-sequence packets, and channels are independently sequenced (design above); RELIABLE/UNSEQUENCED flags are in [enet.h](https://github.com/lsalzman/enet/blob/master/include/enet/enet.h). [HYPOTHESIS] Use channel0 for all R traffic from an origin, channel1 **unsequenced U** to avoid silently dropping old bundles' events due to pose age. Legacy already filters old pose timestamps but applies events. Tiny60-byte gameplay/132-byte lobby bodies need no fragmentation. Envelope: explicit game/protocol ID, session epoch, origin ID, payload length, reliability class. Duplicate suppression at transport/envelope delivery, not blanket old-MOVE-time rejection. Exact channel policy requires pilot compatibility tests.

[HYPOTHESIS] A host-routed star is enough for<=4: client->host->other peers with **original origin ID**, one local application at host, no echo to sender; host's own sends go only to clients. Preserve per-origin reliable order. This topology does not change simulation authority. Echoing would duplicate effects/chat. Connection ownership, not client-provided DPID, establishes origin.

[HYPOTHESIS] Pilot explicit loopback endpoint appears as an enumerated session, avoiding fixed-port discovery listener conflicts. LAN later adds bounded discovery requests/responses(session UUID, game/revision/content hash, name/capacity/endpoint), separate from gameplay unicast. Internet first direct IP/port, configurable host port/client ephemeral ports; NAT traversal/relay only later on evidence of need. Do not assume ENet solves firewall/NAT or authentication/encryption.

### Android

[EVIDENCE] Current manifest has no network permissions (port/android/app/src/main/AndroidManifest.xml:1-41); minSDK24,target/compile35,arm64/x86_64 (port/android/app/build.gradle:25-46). Current app is SB1; common transport alone does not establish SB2 Android support.

[HYPOTHESIS] Android milestone adds INTERNET, native ENet build and lifecycle/error handling for pause/resume, network change and timeout. Keep explicit endpoint fallback when discovery is blocked. JNI discovery/permission bridge only where needed.

[EVIDENCE] Official [LAN permission guidance](https://developer.android.com/privacy-and-security/local-network-permission) says target<=36 with INTERNET retains implicit LAN access; target37/Android17 requires ACCESS_LOCAL_NETWORK runtime permission for direct LAN access, or an applicable system service picker. Android16 protection is opt-in. Guidance covers unicast/broadcast/multicast sockets; do not request that new runtime permission prematurely for target35.

[EVIDENCE] Raw Wi-Fi multicast can require MulticastLock; acquire/release around discovery, not whole gameplay: [MulticastLock](https://developer.android.com/reference/android/net/wifi/WifiManager.MulticastLock) and [CHANGE_WIFI_MULTICAST_STATE](https://developer.android.com/reference/android/Manifest.permission#CHANGE_WIFI_MULTICAST_STATE). System mDNS through NsdManager manages foreground multicast automatically from T extensions7; older devices need lock: [NsdManager](https://developer.android.com/reference/android/net/nsd/NsdManager). [HYPOTHESIS] Choose UDP broadcast or mDNS/NSD during LAN/Android milestone after device/AP tests. MulticastLock does not guarantee broadcast discovery everywhere; AP isolation/VPN/interfaces remain tests.

### Supplied reference checkout findings

[EVIDENCE] **Free Eggbert has no modern transport replacement here.** external/free-eggbert/src/network.cpp:9,52-98 still DP enumeration/create/query; :218 original session flags; :254 DP Send; :262-288 DP Receive then **zeroes destination instead of copying payload**, compares sender with slot i instead of stored DPID. README.md:66-75 calls network.cpp tentatively complete; that is not runtime proof. include/network.hpp:5,36-67 retains DP contract. LICENSE:1-2 GPLv3. No code copied.

[EVIDENCE] external/sb2-decomp/src/network.cpp:57-59,86-88 remains DirectPlay. LICENSE:1-2 GPLv3. Reference ancestry, not a portable transport.

[EVIDENCE / bounded negative search] Supplied modern Planet Blupi tree yielded **no game multiplayer transport**: src file inventory has no network TU; CMakeLists.txt:39-41 globs that tree; DirectPlay/enet/SDL_net/socket searches found no game transport. movie.cxx:41,116 is media URL/network support; decor.h:574/decblupi.cxx:1347-1349 “multi_goal” means multiple goals. LICENSE:20 identifies GPLv3+ (:23-24 contains GPLv3 text). [OPEN] Context's “own network code” may refer to another historical revision; cannot attribute such a protocol to this checkout.

## 7. Validation before legacy entry

[HYPOTHESIS / required design] Decode fixed-width portable fields into bounded temporary storage. Validate **whole packet and all nested events atomically** before legacy sees it. Historical unused bytes are not required-zero reserved fields. Encode meaningful fields with zero padding; preserve signed-char speed and -1 transport semantics. Zero-fill requested destination capacity (200 lobby/60 gameplay), copy only validated received bytes, return mapped slot; never copy uninitialised receive tails. Use envelope length, not signed char size, for bounds.

[HYPOTHESIS] Common checks: envelope/payload cap, revision/game ID, session epoch, connection-to-DPID ownership, lobby/gameplay namespace, per-peer rate/queue budget, no local echo/prior-session replay. A transport-established but game-table-unknown sender is allowed only for JOIN; other messages need present slot0..3. All identity validation must precede Receive's hiding of raw sender. Existing hazard: both network.cpp:315-342.

| Message, both games | Required checks | Legacy consequence / evidence |
|---|---|---|
| JOIN2 | Exactly28,size28; issued DPID==origin, unique, no duplicate join; lobby/host recipient; NUL within20 name bytes, <=19 chars; capacity4. | strcpy into22-byte name; repeated JOIN consumes slots: S1/event.cpp:4112-4122; S2:4784-4794. |
| LOBBY4 | Exactly132,unsigned size132; host only; map1..10 SB1/1..12 SB2. Four32-byte entries; present/ready0/1,team0..3,bounded NUL names; present DPIDs unique/known, local player exactly once. Teams need not be unique. Validate replacement lifecycle. | memcpy table; team indexes spawn arrays; map influences file selection: S1/event.cpp:4131-4134,decor.cpp:211-231; S2/event.cpp:4803-4806,decor.cpp:492-512. |
| TEAM5 | Exactly12,size12; lobby,present owned DPID,team0..3; disallow colour changes while own ready as UI does. | Any DPID otherwise changed; unsafe future team indexing: S1/event.cpp:4139-4142; S2:4811-4814. |
| READY6 | Exactly12,size12; lobby,owned present DPID,bool0/1. | Forged readiness changes start gate: S1/event.cpp:4149-4153; S2:4821-4825. |
| LEAVE3 | Exactly12,size12; owned DPID; host0/1 matches actual origin role; trusted service may generate controlled disconnect equivalent. | Forged host bit closes recipients: S1/event.cpp:4156-4177; S2:4828-4849. |
| START7 | Exactly2,size2; host,lobby,2..4 present/all ready,valid local slot/team; content agreed; once only. | Unconditional start/phase switch: S1/event.cpp:4180-4184; S2:4852-4856. |
| CLOSE1 lobby | Historical send size unknown. Define internal terminal control (suggest2 bytes) for host/service only; never accept gameplay MOVE in lobby. | Blind close S1/event.cpp:4186-4192; S2:4858-4864. Suggested size2 is not historical evidence. |
| CHAT11 | Exactly108,size108; lobby,owned DPID; NUL within100 bytes,max99 display bytes,allowed legacy text/control policy,rate cap. Check outgoing length too. | ChatPush strcpy100-byte row: S1/event.cpp:3860-3875. ChatSend's concatenations can overflow payload if UI constraints fail: S1:3885-3896; S2:4558-4568. |
| MOVE1 gameplay | size60,n0..5(reject negative),length20+8*n; present origin slot0..3,time0..32767 with explicit wrap policy; keys low6 bits only; flag0/1 consistent with class. icon=-1 or valid rank in selected sheet; sec0..4; channel whitelist for game's character/vehicle/colour sprites; signed transport normally -1 or0..127,compatible live lift when used (high-slot truncation caveat below). Validate every nested event. | n unbounded loops beyond pack; unknown/stale sender events still execute; invalid pose reaches prediction/collision/render: S1/decmove.cpp:128-186,241-264; S2:142-200,255-278; rank table access S1/pixmap.cpp:810-816; sec constants S1/def.h:728-731,S2:726-729. |
| MOVE coordinates | Signed16 representation, bounded world-pixel rectangle with **explicit fall/effect margins**. Do not blindly clamp0..6399: loss/fall permits below world and effects use negative offsets. | Position affects TestPath/spatial math: S1/decmove.cpp:135-152,243-252; fall bottom+1000 S1/decblupi.cpp:3554-3570,S2:4140-4156; offset effects S2/decnet.cpp:20-43. Exact margins [OPEN]. |
| ABANDON8 / LOST9 | Exactly4,size4,short0; gameplay,known present slot0..3; once per lifecycle. Keep identity/name valid until notification consumed. | Branches index arrays **without sender guard**, unknown returns-1: S1/decmove.cpp:188-203; S2:202-217. |
| DIE10 | Exactly4,size4; known present slot; normal multiplayer lives0..10 or explicitly evidenced extended range. | Unchecked player name index/arbitrary life notice: S1/decmove.cpp:204-219; S2:218-233; initial10 S1/decblock.cpp:978. |
| PAUSE12 | Exactly3,size3,bool0/1; known member; gameplay/supported overlays; zero high byte of padded short. A4-byte variant requires separate evidence. | Any sender sets pause: S1/decmove.cpp:221-224; S2:235-238. |
| OBJECTSTART20 | Embedded8; signed-byte speed; **game-specific emitted/implemented object whitelist**, type-specific coordinate margins, required R escalation,allocation/rate cap. Do not allow arbitrary shorts or all inherited def.h symbols. | Creates object/type-specific movement/icon behaviour, can exhaust200 slots: S1/decnet.cpp:20-98; S2:48-126. Exact semantic type/speed list remains follow-up. |
| OBJECTDELETE21 | Embedded8,valid game type and bounded x/y; member sender; ignore unused data1. | Deletes shared object by position/type with no ownership checks: S1/decnet.cpp:103-121; S2:131-149. |
| MODIFDECOR30 | Embedded8; **0<=x<6400,0<=y<6400**,checked division/index; icon=-1 or valid game's tile rank; R required. Only require64 alignment if all producers prove it. | Direct unchecked cell write: S1/decnet.cpp:129-132; S2:157-160. Negative/large index corrupts state; icon affects rendering/collision. |
| PLAYSOUND40 | Embedded8; sound0..MAXSOUND-1(100) plus actual loaded/defined whitelist; bounded x/y. | ID/position passed to PlayImage: S1/decmove.cpp:169-174; S2:183-188; S1/sound.h:12. |
| STOPSOUND41 | Embedded8; sound whitelist as above. | Direct StopSound: S1/decmove.cpp:175-176; S2:189-190. |
| ASCENSEURSTART50 / END51 | Embedded8; actual host origin; **index0..199**,live compatible moving object; step selected by type; ignore historically unset data1. | Immediate unchecked m_moveObject[index] write/read: S1/decmove.cpp:269-295; S2:283-309. |
| STOPCLOUD60 | Embedded8; rank0..3,known member,R required; preserve/document slot/team ambiguity. | Clears local effect if rank==team: S1/decmove.cpp:181-185; S2:195-199. |
| DP system messages | Consume as trusted service control, never pass dwType bytes into game namespace. | Conflation shown in system table. |

[OPEN] Exact per-game channel/icon/object-type/speed whitelists and legal coordinate margins require focused emitter/table review before pilot acceptance. This report gives evidenced structural limits, semantic rules and dangerous sinks; guessed margins are not historical facts. Completing these rules is an implementation prerequisite and must block internet exposure.

[EVIDENCE / additional representation limit] MOVE.blupiTransport is a **signed char**, despite MAXMOVEOBJECT=200. Sender truncates the int at both decmove.cpp:76; receiver promotes it at S1:140/S2:154, and lift-follow only accepts nonnegative indices at S1:262/S2:276. Thus a representable valid wire value is -1 or0..127, not an encoded signed value128..199. Other negative byte values can be historical truncations of high lift indices and are safely ignored by the legacy follow check. [OPEN] Test maps using object slots128..199; choose a documented compatibility policy before rejecting their entire otherwise valid MOVE or repairing their index. The validator must not reinterpret an arbitrary negative value as a safe array index. Normal lives ceiling10 also has pickup evidence: S1/decblupi.cpp:3101/S2:3628 only collects an egg while lives<10.

## 8. Smallest end-to-end SB1 pilot proposal

### Scope and contract implementation

[HYPOTHESIS] SB1 Blupi US desktop data, two loopback endpoints, faithful50ms tick, default packedFALSE/predictionTRUE. Smaller transient-type set, directly shared event path, future Android relevance. Avoid SB2 event derivative/extra network particle effects initially (port/games/sb2/CMakeLists.txt:29-31; port/docs/ARCHITECTURE.md:175-178).

[HYPOTHESIS] Keep historical public header/signatures. Portable state in an adapter-owned sidecar keyed by CNetwork*, freed in destructor; no fake DirectPlay cast. One native provider. CreateProvider validates/initialises; EnumSessions returns bounded snapshot of configured endpoint, TRUE if empty. Join/Create bounded setup allocates stable nonzero32-bit IDs, assigns host only on success; failures keep retryable provider. Free enumeration lists must not destroy live session.

[HYPOTHESIS] Send validates/encodes/enqueues; Receive nonblocking services ENet, validates atomically, returns one legacy payload or FALSE, zeroes destination tail and maps original DPID to game slot. JOIN's table-unknown slot=-1 is legal only for that message. Close is portable-layer idempotent and bounded; terminal error surfaced via port status/phase hook because BOOL cannot distinguish empty/timeout/lost.

[HYPOTHESIS] Minimal session metadata handshake: game/protocol/version, content fingerprint, assigned membership IDs, capacity and readiness-to-receive. These are transport metadata, not new CDecor sync. Refuse new joins after START; buffer valid early gameplay across START transition rather than consuming MOVE as lobby CLOSE. Drop old session epoch packets. Selected map must be locally compatible; no level transfer.

### Exact future file inventory (none edited here)

| Action | Files under port/ | Purpose |
|---|---|---|
| Add | common/net/transport.h, common/net/transport_enet.cpp | Compat-free envelope, socket ownership/service, queues/reliability, bounded close. |
| Add | common/net/session.h, common/net/session.cpp | Session/member IDs, endpoint enumeration, compatibility/capacity/status. Small used contract only. |
| Add | games/sb1/game/network_protocol.h, games/sb1/game/network_protocol.cpp | SB1 codec/atomic validator/role-phase checks and legacy buffer mapping. No game constants in common/. |
| Replace | games/sb1/game/network.cpp | Full CNetwork methods using sidecar/common service, preserve list/header semantics. |
| Add | games/sb1/game/network_port.h, games/sb1/game/network_port.cpp | Phase/status lifecycle, bounds/sheet metadata snapshot, trusted disconnect phase handling, coordinator hook. |
| Modify | games/sb1/game/main.cpp | Pump lifecycle/status at stable tick points; terminal phase path. Historical gameplay TUs unchanged. |
| Modify | common/app/launcher.h, common/app/launcher.cpp | Endpoint/listen options, deterministic test step gate; inactive by default. Hooks launcher.h:79-81 and launcher.cpp:560-593. |
| Add | tests/net_coordinator.py, tests/test_net_transport.cpp, tests/test_net_sb1_protocol.cpp, tests/net_sb1.cmake | Bounded supervisor, byte/contract corpus, two-process session test. Future outputs under build/. |
| Add | tests/net/sb1_host.input, tests/net/sb1_client.input, tests/net/sb1_faults.txt, tests/golden/net_sb1_host.trace, tests/golden/net_sb1_client.trace | Small lobby/gameplay scripts, fault schedule, peer-specific goldens recorded only once implementation works. |
| Add | games/sb1/game/network_trace.h, games/sb1/game/network_trace.cpp | Read-only membership/replicated-state projection and message audit, separate from full hashes. |
| Modify if fixture requires | games/sb1/game/probe.cpp | Test-only initial event fixture through --probe; never fake incoming success/remote state. Existing policy :1-21. |
| Modify | CMakeLists.txt, tests/CMakeLists.txt | Build/link transport/protocol, register tests; retain direct-share lists. |
| Add dependency | third_party/enet/ | Pinned unmodified upstream files and LICENSE. Exact commit/source manifest [OPEN] pending pin review; no download here. |
| Modify | third_party/README.md, release/THIRD_PARTY.txt, cmake/package_app.cmake | Pin/hash/licence and distribution notice, preferably static link. |
| Modify | README.md, docs/ARCHITECTURE.md, docs/ROADMAP.md, docs/FIDELITY.md, provenance.json | Supported subset, migration/disconnect policy, review/ownership/evidence. |

[HYPOTHESIS] SB2 network.cpp remains stubbed for pilot; add games/sb2/game/network_protocol.* and its common-service consumer in a separate milestone. Android permission/UI changes are outside desktop pilot, while common APIs remain NDK-buildable. No src/ or games/sb2/src/ changes, no matching acceptance change.

### Deterministic two-instance regression design

[EVIDENCE] Headless frames run back-to-back without waiting (port/common/platform/host.h:19; common/app/launcher.cpp:560). Script and prepareTick precede timerStep (:562-567), completedTick/trace follow (:576-593). Golden gate runs one process and compares frame/phase/state/screen (port/tests/golden_check.cmake:1-17,26-43). Current full hash includes local character/camera/team/queues/counters/prediction (port/games/sb1/game/state_hash.cpp:56-100); it accesses CDecor/CEvent, **not CNetwork membership** (:26-28). Netblupi probes inject render state, not transport (probe.cpp:113-123).

[HYPOTHESIS] Two fast independent headless instances are not deterministic merely with --frames. Add a **test-only external coordinator**, exactly two supervised children, one session at a time, fresh separate user roots. Grant one logical step to each, receive bounded “tick done/all sends flushed” acknowledgements, collect expected per-tick message manifests, then release prescribed next ingress batch. Messages emitted during ReadInput/MoveStep/EndOfTick in N become visible at fixed receive point N+1 (or scheduled later). Freeze visibility there; late OS arrivals cannot opportunistically enter mid-tick.

[HYPOTHESIS] Use **real ENet loopback sockets** for end-to-end testing with an ingress staging gate. Wait for the known envelope IDs/batch with a wall deadline; admit in scripted order(delivery tick,origin ID,sender sequence). Wall time never changes simulation ticks. Transport unit tests can inject virtual time for retry behaviour. ENet integration controls application-visible delivery; it does **not** claim deterministic OS retransmission timings/counts.

[HYPOTHESIS] Scenario: original UI provider->host create->client enumerate explicit endpoint/join; verify identical2-member tables/names/DPIDs; colour changes/chat/ready false-true, assert start gate; select map101/start. Then short fixed movement/jump/fire on bounded fixture: local position must change, remote must receive that origin's pose, and real ObjectStart/Delete or ModifDecor changes recipient simulation. Pause/resume; a reachable death/life notice; graceful leave and correct win/end state. Keep scripted gameplay short smoke coverage, no bots, in line with port/AGENTS.md testing policy.

[HYPOTHESIS] Hash checks:

1. Keep existing PortStateHash unchanged; compare host A to host B and client A to client B **each frame**, with fixed IDs/startup/input/RNG history/ingress. Screen hashes per peer too; never require cross-peer full/screen hashes equal.
2. Separate DPID-ordered membership/replicated-state trace: lobby map/names/team/ready; each peer's owned pose and last accepted remote pose/time; delivered events and selected shared-world mutations. Compare membership at barriers, remote snapshots to origin's transmitted pose at that timestamp, shared mutations after known delivery. Local predicted pose need not equal origin at same render frame. Object slot arrays/RNG histories are not globally promised identical; compare logical scenario objects/events, not broad normalisation hiding divergence.

[OPEN] Existing full hashes include m_netMessages, including inactive queue slots (state_hash.cpp:74). Lift producers leave NetMessage.data1 unset (S1/decmove.cpp:524-528,573-577; S2:575-579,624-628). Even a fixed delivery schedule might expose raw unused-byte instability. Diagnose field dumps before calling this gameplay divergence; retain full traces as diagnostics, and use an explicitly scoped semantic network projection for unused/padding fields. Do not regenerate offline goldens or assert deterministic full-hash equality until this is checked. Keep the smallest baseline fixture free of unrelated burst/lift paths, then add them as separate tests.

[HYPOTHESIS] Matrix: no loss; prescribed U loss/delay/reorder including embedded events; R retransmit/duplicate suppression/create-before-delete; packed mode; queue burst; mismatched game/revision/map; malformed length/count/index/name/sender; duplicate JOIN; client graceful/abrupt exit; host graceful/timeout with explicit unsupported migration; immediate START->MOVE; rapid team/ready; team!=slot cloud; accelerated32767 wrap. Reliable enqueue/delivery failure must visibly fail session, not silently lose accepted events. Bound every wait/process, use reserved ephemeral ports, kill/reap children on timeout. This assessment did not run any of these.

[HYPOTHESIS] Acceptance: actual create/join/lobby/ready/start and non-cosmetic gameplay mutation, repeat hashes under fixed ingress, malformed packets rejected before legacy, offline golden/scene/commands/layout unchanged, fidelity/impact clear. Windows and Linux receipts separately; later Android interoperability. No original-executable runtime proof inferred.

### Provenance proposal

| Edge / current evidence location in port/provenance.json | Pilot treatment |
|---|---|
| F055 :3458-3572, VERIFIED_DERIVATIVE list/helpers | Review constructor/lists/IsHost with live sidecar. Keep unchanged-body evidence only where true; split rewritten service symbols if needed, retaining stable IDs/ledger_ids. Remove “offline m_bHost stays FALSE”; lifecycle/list tests. |
| F056 :3575-3695, zero-provider INTENTIONAL_DIFFERENCE | Replace unavailable reason with scoped native replacement. Retain intentional differences for DP interop, topology, validation and unsupported migration. Verified contract subgroup only after evidence; loopback tests do not certify all providers/editions. Record SB1 US pilot/untested editions explicitly. |
| F274 :6619-6724, SB2 helpers | Shared build/context review only; SB2 runtime remains offline. |
| F275 :6727-6838, SB2 unavailable network | Remains unavailable/INTENTIONAL_DIFFERENCE for SB1 pilot. Change only with later SB2 adapter/tests. |
| F005/F006/F009 SB1 decmove/decnet/event direct shares (:838,:929,:1202); F017/F018 SB2 counterparts (:1897,:1979) | Keep DIRECT_SHARED, sources unchanged; review changed service context and bounded multiplayer evidence. No matching proof claim. |
| F469 :14581-14694 SB1 hash/probes; F470 :14695-14794 SB2 tooling | Add scoped observer/fixture evidence to F469 if touched; F470 unchanged except shared impact review. Drawing probes are not multiplayer proof. |
| Main/launcher/build/common_contracts and all consumers of changed common paths | Run impact analysis for exact patch-dependent ID closure. Review lifecycle/zeroing/offline/forced-context dependencies. New common/net,game codec/trace files require explicit owned edges; tests/tools/prose can use appropriate exact-file exclusions, not directory exclusions. |

[EVIDENCE] Fidelity policy requires stable IDs, ownership of new files, immutable review revision/disposition/evidence/tests, LF-normalised reviewed_files hashes **after** review; hash-only refresh cannot clear consumers (port/docs/FIDELITY.md:25-57). [OPEN] Exact full impact closure depends on final patch; no invented affected-edge count. Future gates check_fidelity --check, --base/--head and named tests; **not run here**.

## 9. Ranked risks, unknowns and decisions

| Rank | Risk / source evidence | Resolution |
|---|---|---|
| 1 | **Unsafe packet parsing / identity.** Actual length ignored; unchecked count/player/lift/tile indices and strcpy. Both network.cpp:315-342; S1/event.cpp:4122,decmove.cpp:156-218,276,decnet.cpp:129-132; S2 equivalents above. | Complete semantic whitelists/margins, atomic decoder, origin-bound membership, rejection tests before internet. |
| 2 | **Start/order races and false determinism.** Code1 namespace collision, old pose still executes events, full hashes peer-local. S1/event.cpp:4186,decmove.cpp:130-186; port/state_hash evidence above. | Stage buffering, per-origin R order, deterministic ingress and peer-specific traces plus shared assertions. Also test normal unscripted network timing. |
| 3 | **Migration/disconnect gaps.** Requested migration without host/system handling; sudden losses leave stale presence. Both network.cpp:266,359; S1/event.cpp:4156-4178. | Explicit unsupported migration/host-loss termination, bounded errors; later election separately scoped. |
| 4 | **Historical algorithm limits.** Rollover, ignored full queue, slot/team cloud ambiguity,no pose heartbeat. S1/decblock.cpp:1545-1546,decmove.cpp:59-63,130,181,346,decblupi.cpp:3307. | Isolate from transport defects; correction needs explicit port fidelity disposition, never shared-TU edits. |
| 5 | **Content/revision compatibility.** Same GUID; different maps/effect sets. Both network.cpp:14; message tables. | Separate game IDs/content hashes; reject mismatch before START; cross-game play excluded. |
| 6 | **ABI/text.** Signed speed/transport,size132,pause3,unused bytes. Layouts above; signed-char compile context port/games/sb2/CMakeLists.txt:21. | Fixed-width little-endian codec/static layout assertions, sentinel/text tests. |
| 7 | **LAN/Android lifecycle/reachability.** No INTERNET now,target35; multicast filtering/new target37 permission. Android sources above. | Later4-device/AP/platform matrix, direct-IP fallback, lifecycle/error UI, target-dependent permission handling. |
| 8 | **Dependency scope.** ENet is not membership/discovery/NAT/migration/security. | Pin/review/notice, implement only used small contracts; rate/backpressure. Internet authentication/encryption guarantees remain [OPEN] for that milestone. |

[OPEN / implementation decisions] Recommended choices to accept when implementation starts: ENet vendoring/notices; SB1 US loopback-first; explicit host-disconnect-ends-session/migration unsupported. Later decisions: documented port fixes for demonstrated historical rollover/cloud bugs, and internet authentication/encryption expectations. No decision is needed to complete this assessment.

Status: **done as bounded static research**. Runtime fidelity, exact original-provider timing, semantic whitelist completion, ENet revision selection and platform multiplayer receipts remain explicitly open implementation work.
