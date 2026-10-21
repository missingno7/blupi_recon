# Cross-build oracle (port/docs/ROADMAP.md stage 0.3).
#
# Runs the validated historical rebuild (build/standalone-validation/BLUPI.EXE,
# never the original executable) under cnc-ddraw with the frame-capture proxy
# (port/oracle/ddraw_proxy.cpp) in an isolated copy of the data, lets the
# original attract mode play, closes the game after -Frames displayed frames,
# then runs the SDL3 port headless with the same configuration and compares the
# raw back-buffer hash of every displayed frame.
#
# Do not touch the mouse or keyboard while the historical window is open: input
# changes what the game draws (cursor sprite, demo interruption).
#
# Output: build/oracle/run-<timestamp>/ with hist.log, port.log, compare.json,
# and raw frames (every -DumpEvery frames) under hist-frames/ and port-frames/.
param([int]$Frames = 2200, [ValidateSet(8,16)][int]$ColorDepth = 16, [int]$DumpEvery = 100,
      [string]$InputScript = '')
$ErrorActionPreference = 'Stop'
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$original = Join-Path $repo 'assets\Speedy Blupi'
$built = Join-Path $repo 'build\standalone-validation\BLUPI.EXE'
$proxy = Join-Path $repo 'build\oracle\ddraw.dll'
$port = Join-Path $repo 'build\port\blupi-sdl3.exe'
# (Re)build the 32-bit proxy when its source changed. The MinGW i686 compiler
# must run with its own bin directory on PATH, or it silently produces nothing.
$proxySource = Join-Path $repo 'port\oracle\ddraw_proxy.cpp'
$mingw32 = if ($env:MINGW32_BIN) { $env:MINGW32_BIN } else { 'C:\msys64\mingw32\bin' }
if (-not (Test-Path $proxy) -or (Get-Item $proxySource).LastWriteTime -gt (Get-Item $proxy).LastWriteTime) {
    New-Item -ItemType Directory -Force -Path (Split-Path $proxy) | Out-Null
    $savedPath = $env:PATH
    $env:PATH = "$mingw32;$env:PATH"
    try {
        & (Join-Path $mingw32 'g++.exe') -std=c++17 -O2 -shared -static '-Wl,--kill-at' -o $proxy $proxySource -ldxguid -luuid
        if ($LASTEXITCODE -ne 0) { throw 'proxy build failed' }
    } finally { $env:PATH = $savedPath }
    if ((Get-Item $proxySource).LastWriteTime -gt (Get-Item $proxy).LastWriteTime) { throw 'proxy build produced no new DLL' }
}
foreach ($p in @($built, $proxy, $port)) { if (-not (Test-Path $p)) { throw "missing $p" } }
$gate = Get-Content (Join-Path (Split-Path $built) 'gate.json') -Raw | ConvertFrom-Json
if ($gate.status -ne 'PASS') { throw 'Validated source build required' }
if ((Get-FileHash $built).Hash -eq (Get-FileHash (Join-Path $original 'BLUPI.EXE')).Hash) {
    throw 'Refusing original executable'
}

$runDir = Join-Path $repo ('build\oracle\run-' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
$gameDir = Join-Path $runDir 'game'
New-Item -ItemType Directory -Path $gameDir | Out-Null
foreach ($name in @('DATA','IMAGE08','IMAGE16','SOUND','Shaders')) {
    Copy-Item -LiteralPath (Join-Path $original $name) -Destination $gameDir -Recurse
}
Copy-Item -LiteralPath $built -Destination (Join-Path $gameDir 'BLUPI.EXE')
Copy-Item -LiteralPath (Join-Path $original 'ddraw.dll') -Destination (Join-Path $gameDir 'cnc-ddraw.dll')
Copy-Item -LiteralPath $proxy -Destination (Join-Path $gameDir 'ddraw.dll')
$ini = Get-Content (Join-Path $original 'ddraw.ini') -Raw
foreach ($setting in @(@('width','640'),@('height','480'),@('windowed','true'),
    @('fullscreen','false'),@('renderer','gdi'),@('devmode','true'),@('maxfps','60'),
    @('posX','100'),@('posY','100'),@('savesettings','0'),@('adjmouse','false'))) {
    $ini = [regex]::Replace($ini, '(?m)^' + $setting[0] + '=.*$', $setting[0] + '=' + $setting[1])
}
[IO.File]::WriteAllText((Join-Path $gameDir 'ddraw.ini'), $ini)

# Same configuration for both builds (see tools/runtime_smoke.ps1 for the CD layout).
$configPath = Join-Path $gameDir 'DATA\config.def'
$config = Get-Content $configPath -Raw
$languageDir = Join-Path $gameDir 'ENGLISH'
New-Item -ItemType Directory -Path $languageDir | Out-Null
$config = [regex]::Replace($config, '(?m)^CD-Rom=.*$', 'CD-Rom=' + $languageDir)
$config = [regex]::Replace($config, '(?m)^Benchmark=.*$', 'Benchmark=0')
$config = [regex]::Replace($config, '(?m)^TrueColor=.*$', 'TrueColor=' + $ColorDepth)
# The graphic cursor (MouseType=1) is drawn into the back buffer at the OS cursor
# position, which depends on the desktop and cnc-ddraw scaling, not on the game.
# The Windows cursor (MouseType=2) keeps it out of the compared frames.
$config = [regex]::Replace($config, '(?m)^MouseType=.*$', 'MouseType=2')
[IO.File]::WriteAllText($configPath, $config)
$portUser = Join-Path $runDir 'port-user'
New-Item -ItemType Directory -Path (Join-Path $portUser 'data') | Out-Null
Copy-Item -LiteralPath $configPath -Destination (Join-Path $portUser 'data\config.def')

$histFrames = Join-Path $runDir 'hist-frames'
$portFrames = Join-Path $runDir 'port-frames'
New-Item -ItemType Directory -Path $histFrames, $portFrames | Out-Null
$receipt = [ordered]@{
    executable = $built; sha256 = (Get-FileHash $built).Hash; proxy_sha256 = (Get-FileHash $proxy).Hash
    frames = $Frames; color_depth = $ColorDepth; scope = 'Runtime evidence, not a strict/equivalence proof'
}

$env:BLUPI_ORACLE_LOG = Join-Path $runDir 'hist.log'
$env:BLUPI_ORACLE_DUMP_DIR = $histFrames
$env:BLUPI_ORACLE_DUMP_EVERY = "$DumpEvery"
$env:BLUPI_ORACLE_MAX_FRAMES = "$Frames"
$env:BLUPI_ORACLE_DEBUG = Join-Path $runDir 'hist-debug.txt'
$portInput = @()
if ($InputScript) {
    $InputScript = (Resolve-Path $InputScript).Path
    $env:BLUPI_ORACLE_INPUT = $InputScript
    $portInput = @('--input', $InputScript)
    $receipt.input = $InputScript
}
$limit = [int]($Frames * 0.05 * 2) + 30
# Real mouse movement over the window would add WM_MOUSEMOVE messages the
# scripted run does not have: park the OS cursor in the screen corner.
Add-Type -AssemblyName System.Windows.Forms
[System.Windows.Forms.Cursor]::Position = New-Object System.Drawing.Point(0, 0)
$game = $null
try {
    $game = Start-Process -FilePath (Join-Path $gameDir 'BLUPI.EXE') -ArgumentList '-nocd' -WorkingDirectory $gameDir -PassThru
    if (-not $game.WaitForExit($limit * 1000)) {
        $receipt.forced_termination = $true
        $game.Kill(); $game.WaitForExit()
    } else {
        $receipt.forced_termination = $false
    }
    $receipt.exit_code = $game.ExitCode
} finally {
    if ($null -ne $game -and -not $game.HasExited) { $game.Kill(); $game.WaitForExit() }
    Remove-Item Env:BLUPI_ORACLE_LOG, Env:BLUPI_ORACLE_DUMP_DIR, Env:BLUPI_ORACLE_DUMP_EVERY, Env:BLUPI_ORACLE_MAX_FRAMES, Env:BLUPI_ORACLE_DEBUG
    if (Test-Path Env:BLUPI_ORACLE_INPUT) { Remove-Item Env:BLUPI_ORACLE_INPUT }
}

# The port shows one Display at start-up and ten in Benchmark() before the first
# timer frame, like the original, so N displays = N - 11 timer frames.
& $port --headless --data $original --user $portUser --frames ($Frames - 11) `
    --display-log (Join-Path $runDir 'port.log') --display-dump "$($portFrames):$DumpEvery" @portInput | Out-Null
$receipt.port_exit_code = $LASTEXITCODE

$hist = @(Get-Content (Join-Path $runDir 'hist.log'))
$mine = @(Get-Content (Join-Path $runDir 'port.log'))
$n = [Math]::Min($hist.Count, $mine.Count)
$first = -1
for ($i = 0; $i -lt $n; $i++) { if ($hist[$i] -ne $mine[$i]) { $first = $i; break } }
$equal = 0
for ($i = 0; $i -lt $n; $i++) { if ($hist[$i] -eq $mine[$i]) { $equal++ } }
$receipt.hist_frames = $hist.Count
$receipt.port_frames = $mine.Count
$receipt.compared = $n
$receipt.identical_frames = $equal
$receipt.first_difference = $first
$receipt | ConvertTo-Json -Depth 4 | Set-Content (Join-Path $runDir 'compare.json')
Write-Output $runDir
$receipt | ConvertTo-Json -Depth 4
