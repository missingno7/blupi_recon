# Toolchain identification

[EVIDENCE, 2026-10-20] Full English VC5 SP1 was obtained from the user's
[old-dos.ru source](http://old-dos.ru/files/file_2693.html). C1XX 7149, C2 7153,
LINK 7132 and LIBC are byte-identical to installed SP2. Earlier SP2 compiler
controls therefore cover the same compiler binaries. See
[CONTEXT_REVISIT.md](CONTEXT_REVISIT.md) for archive hashes and provenance limits.

## [EVIDENCE] Oracle and linker

`assets/Speedy Blupi/BLUPI.EXE` is SHA-256 `be6de5a206bc636346dde1b1a4dd11ddc3bda30c6efe64eebe96513f9c8290ac`, PE32/i386, timestamp 1998-10-14 07:17:39 UTC, PE linker version **5.10**, image base `0x400000`, entry RVA `0x24d40`, section alignment `0x1000`, and file alignment `0x200`. Sections are `.text` RVA `0x1000`/vsize `0x2af93`, `.rdata` `0x2c000`/`0x1372`, `.data` `0x2e000`/`0x51e1c`, and `.rsrc` `0x80000`/`0x5b88`. There is no base-relocation or debug directory. The import directory and IAT are both in `.rdata`.

The decoded Rich header has pairs `(0x00000, 270)` and `(0x60684, 1)` (Cvtres 5.00.1668); there is no linker product entry. It does not identify a linker patch build.

I compiled one import-only object with VC5 SP3, then linked that same object with each candidate. The command selected Windows subsystem, i386, an explicit entry point, no default libraries, and the candidate VC library directory. No output executable was run.

| Linker candidate | Banner/file version | PE linker field | Test sections and import placement | Fit |
|---|---:|---:|---|---|
| VC5 RTM | 5.00.7022 | 5.0 | `.text`, `.data`, `.idata` | No |
| VC5 SP2 | 5.02.7132 | 5.2 | `.text`, `.data`, `.idata` | No |
| VC5 SP3 | 5.10.7303 | 5.10 | `.text`, `.rdata`, `.data`; imports in `.rdata` | Yes |
| VC5 CE beta 1 | 5.10.7255 | 5.10 | `.text`, `.rdata`, `.data`; imports in `.rdata` | Yes |
| VC5 NT5 1592 / 1691, PSDK 1197 | 5.10.7168 (NT) | 5.10 | `.text`, `.rdata`, `.data`; imports in `.rdata` | Yes |
| VC5 CE 200 | 5.11.7351 | 5.11 | `.text`, `.rdata`, `.data` | No: version field differs |
| VC6 RTM | 6.00.8168 | 6.0 | `.text`, `.rdata`, `.data`; file alignment `0x1000` | No |

In the controlled test, the 5.10 linkers all emitted the same import descriptor order (`USER32`, `KERNEL32`) and IAT order (`KERNEL32` at RVA `0x2000`, `USER32` at `0x2008`). This small test does not distinguish those linker builds. The oracle only records `5.10`; no local candidate is 5.10.7287. Therefore **5.10.7303 is the best available linker candidate, but 5.10.7168 and 5.10.7255 remain consistent with the measured PE fields and layout**.

## [EVIDENCE] CRT identification

I ran `lib /list` on each `LIBC.LIB` and `LIBCMT.LIB` (624 members for each VC5 archive; 633 for VC6), then used `lib /extract` for `strlen`, `memset`, `memcpy`, `rand`, `wincrt0`, and `initcon` objects. The full scan parsed every archive member and compared public function sections against oracle `.text`. A **strict** hit has no relocation bytes; a **masked** hit ignores relocation bytes and is not strict equality. Counts below are unique public symbol names, not a claim that every matching library function was linked.

| Library release | `LIBC` strict + masked = total | `LIBCMT` strict + masked = total |
|---|---:|---:|
| VC5 RTM | 16 + 147 = 163 | 17 + 98 = 115 |
| VC5 SP2 | 16 + 153 = 169 | 17 + 104 = 121 |
| VC5 SP3 | **16 + 156 = 172** | 17 + 104 = 121 |
| VC6 RTM | 8 + 13 = 21 | 8 + 12 = 20 |

The strongest single-thread evidence is `_WinMainCRTStartup` from VC5 `LIBC.LIB`: its 416-byte function matches at the oracle entry RVA `0x24d40` with 124 relocation bytes masked. No `LIBCMT.LIB` startup matched that entry. A matched `/ML` probe imported no thread/TLS/critical-section APIs; the `/MT` probe did. The oracle has no `GetCurrentThreadId`, TLS, or critical-section imports. This identifies **single-thread `LIBC.LIB`**, not `LIBCMT.LIB`.

VC5 SP3 `LIBC.LIB` is the strongest release match: three masked matches not present in RTM or SP2 are `__setmbcp` (528 bytes, 160 relocation bytes masked, RVA `0x290e0`), `__output` (2448/304, RVA `0x25890`), and `_gmtime` (352/48, RVA `0x2a350`). Strict CRT examples shared across VC5 releases include `_strchr` (188 bytes at RVA `0x24690`), `_strncpy` (254 at `0x24750`), and `__allmul` (52 at `0x2b2e0`).

The oracle also contains masked CRT code matches for `___CxxFrameHandler` (64 bytes at RVA `0x23e90`, 4 relocation bytes masked), `___InternalCxxFrameHandler` (208 at `0x26ea0`, 12 masked), and `?__ArrayUnwind` from `ehvecdtr.obj` (112 at `0x24380`, 24 masked). This supports `/GX` exception handling being enabled.

## [EVIDENCE] Compiler and flags

I compiled tag `v1.8.0` `misc.cpp` and `fifo.cpp` from `external/planetblupi` with each compiler. `misc.cpp` needed `/Dnullptr=0` because the 2017 import contains this later C++ spelling; that shim was confined to the probe. The Planet tree has no tracked `.dsp` project at that tag. The release configuration in `external/sb2-decomp/msvc5/sb2decomp/sb2decomp.dsp` is only a hint: it specifies `/W3 /GX /O2`, `WIN32`, `NDEBUG`, `_WINDOWS`, `/YX`, and `/FD`.

The matching Planet function `ConvLongToPos` is a strict 32-byte code-section match at oracle RVA `0x1f330` (VA `0x41f330`). `GetCurrentDir` matches at RVA `0x1f360` with 16 relocation bytes masked. VC5 RTM, SP2, SP3, NT5 1592/1691, and the localized SP3 compiler packages produced identical code for all 12 public functions in this translation unit. VC6 produced identical code for only 5/12 and its `InitRandom` did not match the oracle. These source fingerprints identify the **VC5 compiler family**, but do not distinguish VC5 compiler drops by themselves.

With the SP3 compiler and otherwise fixed probe source, strict/masked code-section matches were:

| Flags | Strict + masked matches in oracle | Observation |
|---|---:|---|
| `/O1 /GX` | 0 + 1 | Poor fit |
| `/O2 /GX` | **2 + 7** | Best tested fit; includes `ConvLongToPos` and `GetCurrentDir` |
| `/Ox /GX` | 2 + 6 | Misses the `GetCurrentDir` match |
| `/O2 /GX /Oy-` | 0 + 1 | Poor fit; forces a frame-pointer prologue |
| `/O2 /GX /G5` | 2 + 7 | Same tested code as `/O2 /GX` |
| `/O2 /GX /G6` | 2 + 3 | Several functions differ from the oracle |
| `/O2 /GX /Gz` | 1 + 1 | Changes free-function calling convention/name decoration |
| `/O2` with `/Gd`, `/Gy`, `/GF`, or without `/GX` | 2 + 7 each | No change in these probe functions; these options remain unproven |

In the matching `GetCurrentDir` body, `EBP` holds a saved value rather than a stack-frame base, consistent with frame-pointer omission. `/GX` itself did not alter this utility translation unit, but the oracle contains C++ frame-handler and array-unwind code. `/G5` and the compiler default were indistinguishable in the probe; `/G6` was not.

## [HYPOTHESIS] Recommended default

Highest joint-support candidate: **VC5 SP3 compiler front ends 5.00.7303 + VC5 SP3 linker 5.10.7303 + single-thread `LIBC.LIB`**, with `/O2 /GX`. The exact compiler release remains open because all tested VC5 compiler packages produced the same matching utility code. The exact linker build remains open because the PE only records `5.10`, and the 5.10.7168/7255 candidates also fit the controlled layout test.

Recommended compile command for a game translation unit:

```bat
set TMP=D:\Prog\blupi_recon\build\tmp
set TEMP=D:\Prog\blupi_recon\build\tmp
set PATH=C:\tools\msvc5_sp3\VC\BIN;C:\tools\msvc5_sp3\SHAREDIDE\BIN;%PATH%
set INCLUDE=C:\tools\msvc5_sp3\VC\INCLUDE;C:\tools\dx5sdk\include
set LIB=C:\tools\msvc5_sp3\VC\LIB
C:\tools\msvc5_sp3\VC\BIN\cl.exe /nologo /W3 /GX /O2 /ML /Gd /DWIN32 /DNDEBUG /D_WINDOWS /c /Fo<obj> <translation-unit>.cpp
```

`/ML` selects the evidenced single-thread static CRT; `/Gd` states the observed default cdecl convention. `/G5` is consistent but need not be explicit. Add `/YX` only when the reconstructed project uses the corresponding precompiled-header/TU context. Do not treat `/Gy` or `/GF` as established by these probes.

## Candidate binary hashes (SHA-256)

The driver versions come from file metadata; front-end versions are listed for the files themselves. VC6 uses `c2.dll`, not `c2.exe`.

| Compiler candidate | `cl.exe` | `c1xx.dll` | `c2.exe` / `c2.dll` |
|---|---|---|---|
| VC5 RTM (driver 7022; c1xx/c2 11.00) | `BF9F9C74F756FED96E13F7F9A4273495C7DDE0A1FB968E3EF6D760AD6D73DFEB` | `FEA1E9FBBC2D204202F1A5EA61E5A938B17A92ACDA57E16DD53D4EC01C9D186B` | `6D1406AF5F62647347CFA7175CD7E108B838D6841A546396F29252198A14D73D` |
| VC5 SP2 (c1xx 7149; c2 7153) | `BF9F9C74F756FED96E13F7F9A4273495C7DDE0A1FB968E3EF6D760AD6D73DFEB` | `8B66D3F14035BFA228E79D45481318457594F65ED91F87319D23932372857D8B` | `592C65EEA2E159A8B7BF61FB20ED12FC0DFDB3C5B7179267D34634AA7A2DC6E4` |
| VC5 SP3 (c1xx/c2 7303) | `BF9F9C74F756FED96E13F7F9A4273495C7DDE0A1FB968E3EF6D760AD6D73DFEB` | `E27DF3BB9A37058C85DAD3FB7D1ED30B9E52D38FB6C0F66C8FDBA0A11111D3D9` | `E75AECAF4073B0817FFB638CAE5FFF636B2E2D1A090DAABF8F68DBC954515FAE` |
| NT5 1592 (driver 7071; front ends 7156) | `87334DF0CFB32223804B7DAC05CEF65959B8052AEC645204C420F295A97CC3D6` | `F86FECDE3F69926B37C8FA738745738F9FE17FD9E1173E1283627CF944DA8AA7` | `67B7244E3579B67E38375B47303025DF950FFEBB2C23B7C6794D2108A3875F54` |
| NT5 1691 (driver 7071; front ends 7156) | `10EB12EB5D9F1C2C914DA9F007953B01935DA774334E44A54D6FC9DEC2B4B2D5` | `AA3EB11E6FCC793D410372164490694F95F71AE585483547E0C0DB242E6185BC` | `C7CA717FE5CB164F8CB660D9677CBFE759302A6ADA583ADB62C9CB37B5AFB68B` |
| VC5 SP3 DEU (front ends 7303) | `BF9F9C74F756FED96E13F7F9A4273495C7DDE0A1FB968E3EF6D760AD6D73DFEB` | `53AE4DE8F3A3107495A5B376220089AD5AD787EE5632D5E30AB765077EF4DA3E` | `BBA1F1A241B1F668511D230243479810EC215A09343E4A9E521EE9B49481C4AE` |
| VC5 SP3 JPN (front ends 7303) | `BF9F9C74F756FED96E13F7F9A4273495C7DDE0A1FB968E3EF6D760AD6D73DFEB` | `AA64EFD9C46E18C6BD9EF50D56FD6DEE99F7662BED99BBD739F6E68AB710B19B` | `4ADCF32337B4DD8B403172534349874F4AE190EEFB4DFAC688B15A0D1404E858` |
| VC6 RTM (8168) | `D9220FDFAF355C81F26E7B03C5DC628EC72FF81C99DC38ED98B1AD4BDBBB309C` | `4095E9E8DE5AC4B13DBFE310ABB8DE711475B1A92C5F94375919947937F3237E` | `45187B0B6288240F73272A7C61E6329C50048A76E57DB3BB87B6F0229E09E27D` |

| Linker candidate | SHA-256 of `link.exe` |
|---|---|
| VC5 RTM 5.00.7022 | `279544809E6353C8E622EB88AEA94E86B0258F3C4B8A976373BF4D68A41B63E1` |
| VC5 SP2 5.02.7132 | `E28424D3EEFCDD96ECC8C3FE38D0FAD3D33077C62026F7774EDA90784D0EB4D9` |
| VC5 SP3 5.10.7303 | `04C892989F4CC8076EF45E033215EB7AEFBDF1459CBEE9B13A81F8DDB718B579` |
| VC5 CE beta 1 5.10.7255 | `4F0A3922E0471F1906EFF2C1CCBA0535476C7B023CC796DD5C6B68450B482207` |
| VC5 NT5 1592 5.10.7168 | `858A26BD0C182AA9D134AF24B6C643C25DCB084C422A9D69B2E9CAB0842170F6` |
| VC5 NT5 1691 5.10.7168 | `AA43191F503E95D1D53BD9D78C6366374DFE7C09314C9C3A0C2925E6DAF0F9DD` |
| VC5 PSDK 1197 5.10.7168 | `95C6AA87CE6321A3ACFDBE17929F3FFC4B563EFC5A3A3755FCDF675A617DE64C` |
| VC5 CE 200 5.11.7351 | `41DCA4F3638AA843591A89C172737C5A22289EB7ED08C6FE868A0DEB3848109E` |
| VC6 RTM 6.00.8168 | `08D3A4C9701C8F6C871B2C48BD74D07422AA1F86E5D9A4DDB2C9BD6A88B857E7` |

| CRT candidate | `LIBC.LIB` SHA-256 | `LIBCMT.LIB` SHA-256 |
|---|---|---|
| VC5 RTM | `4AE22C6F35A87C55C453AD388ADA649EA3BA190614BE3D1AB95935BA0FAA7416` | `7E161E461FD97F6FD5A8E6C1FDB5C7EC1B408E75BFACBC1C226F7556CCD1995B` |
| VC5 SP2 | `F81440DD014CA4CE7EF8B4D60ED1EE4AD544601B07FBDB4FCA54A1DFCA31F9B0` | `1F18290E7CC8419F2D343D7C262B1B04146C244B58E23DFAC95C6A1D684CFD4F` |
| VC5 SP3 | `3B5F43E920C37C8DA086B0D3EF0396E6517D44FCDCB4E7D03A6058620BC13EE3` | `200EB3E8AA6744FDD6DC1726911E16C58EE4D25B338F4677E8621F334AA280B3` |
| VC6 RTM | `E81C25E3BD4C99B88E7F209D052485C0734814346D4320AE32ACE93B56AE3E04` | `1EF9C27B4F76031E40FCBDF7D518ADC4969F90D6A00DD1EA88697907268C1B2B` |

## [OPEN]

- The exact 5.10 linker patch is not encoded in the PE and remains unresolved among available 5.10 linkers. There is no 5.10.7287 executable in the enumerated local candidates.
- The C++ function probe supports VC5 `/O2 /GX`; the exact VC5 compiler drop remains unresolved. `/Gy` and `/GF` did not change the tested function bodies, so their use cannot be inferred here.
- These are static fingerprints and controlled links, not a full source rebuild or strict whole-image match.

Scripts and generated probes are under `scratch/toolchain/` and `build/toolchain/` respectively.

## DirectX SDK ([EVIDENCE], 2026-10-18)

Later bounded controls are recorded in [CONTEXT_REVISIT.md](CONTEXT_REVISIT.md)
(2026-10-20): VC4.2 does not improve the current event TU; local LINK file version
5.11.7351 emits PE 5.10/Rich and the same section payloads as SP3 under the system
CRT. Coarse PE properties do not uniquely select SP3. SP1 Core omits the compiler;
full SP1 was subsequently obtained and its C1XX/C2, LINK and LIBC files are
identical to tested SP2. Package hashes and the exact scope are in that study.

The original was compiled against the **DirectX 5.0 SDK** headers: the oracle's `.rdata` holds the exact ten GUIDs of
DX5 dplay.h (no IDirectPlay4), defined in network.cpp via INITGUID, and dxguid.lib was not linked. The SDK is installed
at `C:\tools\dx5sdk\{INCLUDE,LIB}` (archive.org `ms-dx5-sdk`, ISO MD5 40a5aceb9683e56e6e6451d39daa3686). The manifest
uses dx5sdk since the DX5 integration (zero regressions, docs/LINK_CONTEXT.md).
