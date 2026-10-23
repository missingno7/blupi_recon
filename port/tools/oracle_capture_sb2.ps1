# SB2 runtime oracle. Only a gate-PASS source rebuild may be launched.
# -PrepareOnly stages everything and writes compare.json without launching games.
# SB1's oracle_capture.ps1 remains independent and unchanged.
param(
    [ValidateSet('eggbert','blupi')][string]$Edition = 'eggbert',
    [ValidateRange(12,1000000)][int]$Frames = 2200,
    [ValidateSet(8,16)][int]$ColorDepth = 16,
    [ValidateRange(0,1000000)][int]$DumpEvery = 100,
    [string]$InputScript = '',
    [string]$BuiltExe = '',
    [string]$PortExe = '',
    [switch]$PrepareOnly
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$target = "sb2/$Edition/en"
$targets = (Get-Content (Join-Path $repo 'targets.json') -Raw | ConvertFrom-Json).targets
$targetInfo = $targets.$target
if (-not $targetInfo) { throw "Unknown target $target" }
$originalExe = Join-Path $repo $targetInfo.exe
$data = Split-Path $originalExe
$exeName = if ($Edition -eq 'eggbert') { 'Eggbert2.exe' } else { 'BLUPI.EXE' }
if (-not $BuiltExe) { $BuiltExe = Join-Path $repo "build\sb2gate\relink-rsrc-$Edition-en\$exeName" }
if (-not $PortExe) { $PortExe = Join-Path $repo 'build\port\blupi2.exe' }
$built = (Resolve-Path -LiteralPath $BuiltExe).Path
$port = (Resolve-Path -LiteralPath $PortExe).Path
$buildRoot = [IO.Path]::GetFullPath((Join-Path $repo 'build')) + '\'
if (-not $built.StartsWith($buildRoot, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Historical rebuild must be under the repository build/ directory'
}
if ([IO.Path]::GetFileName($built) -ine $exeName) { throw "Expected $exeName for $target" }
$gatePath = Join-Path (Split-Path $built) 'gate.json'
$gate = Get-Content -LiteralPath $gatePath -Raw | ConvertFrom-Json
if ($gate.status -ne 'PASS') { throw "Validated source rebuild required: $gatePath is $($gate.status)" }
$buildPath = Join-Path (Split-Path $built) 'build.json'
$build = Get-Content -LiteralPath $buildPath -Raw | ConvertFrom-Json
if ($build.target -ne $target -or $build.exit_code -ne 0) {
    throw "Successful source relink for $target required: $buildPath"
}
if ($build.resource_status -notmatch '\.rc via target RC') {
    throw 'Source-built SB2 resources required (relink with the current target resource manifest)'
}
$builtHash = (Get-FileHash -LiteralPath $built -Algorithm SHA256).Hash
$portHash = (Get-FileHash -LiteralPath $port -Algorithm SHA256).Hash
# Check both the immutable target catalog and the installed original. Never
# accept a copied/renamed original, even if somebody put gate.json beside it.
foreach ($entry in $targets.PSObject.Properties) {
    if ($entry.Value.sha256 -eq $builtHash) { throw 'Refusing original executable (catalog hash)' }
    if ($entry.Value.sha256 -eq $portHash) { throw 'Refusing original executable as port (catalog hash)' }
}
$originalHash = (Get-FileHash -LiteralPath $originalExe -Algorithm SHA256).Hash
if ($builtHash -eq $originalHash -or $portHash -eq $originalHash) {
    throw 'Refusing original executable (installed hash)'
}

# The proxy supports these menu/smoke inputs in timer-frame coordinates.
# Reject unsupported commands instead of silently giving the two sides different input.
if ($InputScript) {
    $InputScript = (Resolve-Path -LiteralPath $InputScript).Path
    foreach ($line in Get-Content -LiteralPath $InputScript) {
        $command = ($line -split '#', 2)[0].Trim()
        if (-not $command) { continue }
        if ($command -notmatch '^\d+\s+(click\s+\d+\s+\d+|(?:key|keydown|keyup)\s+\d+)\s*$') {
            throw "Unsupported oracle input: $line"
        }
    }
}
$proxy = Join-Path $repo 'build\oracle\sb2-ddraw.dll'
$proxySource = Join-Path $repo 'port\oracle\ddraw_proxy.cpp'
$mingw32 = if ($env:MINGW32_BIN) { $env:MINGW32_BIN } else { 'C:\msys64\mingw32\bin' }
if (-not (Test-Path -LiteralPath $proxy) -or (Get-Item $proxySource).LastWriteTime -gt (Get-Item $proxy).LastWriteTime) {
    New-Item -ItemType Directory -Force -Path (Split-Path $proxy) | Out-Null
    $savedPath = $env:PATH
    $env:PATH = "$mingw32;$env:PATH"
    try {
        & (Join-Path $mingw32 'g++.exe') -std=c++17 -O2 -shared -static '-Wl,--kill-at' -o $proxy $proxySource -ldxguid -luuid
        if ($LASTEXITCODE -ne 0) { throw 'proxy build failed' }
    } finally { $env:PATH = $savedPath }
    if (-not (Test-Path $proxy) -or (Get-Item $proxySource).LastWriteTime -gt (Get-Item $proxy).LastWriteTime) {
        throw 'proxy build produced no new DLL'
    }
}

$runDir = Join-Path $repo ('build\oracle\sb2-' + $Edition + '-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
$gameDir = Join-Path $runDir 'game'
$portUser = Join-Path $runDir 'port-user'
$histFrames = Join-Path $runDir 'hist-frames'
$portFrames = Join-Path $runDir 'port-frames'
New-Item -ItemType Directory -Path $gameDir, (Join-Path $portUser 'data'), $histFrames, $portFrames | Out-Null
# Copy only game data, never the original executable, installer or launcher.
foreach ($name in @('DATA','IMAGE08','IMAGE16','SOUND','Shaders')) {
    $source = Join-Path $data $name
    if (Test-Path -LiteralPath $source) { Copy-Item -LiteralPath $source -Destination $gameDir -Recurse }
    elseif ($name -ne 'Shaders') { throw "Missing data directory $source" }
}
Copy-Item -LiteralPath $built -Destination (Join-Path $gameDir $exeName)
Copy-Item -LiteralPath (Join-Path $data 'ddraw.dll') -Destination (Join-Path $gameDir 'cnc-ddraw.dll')
Copy-Item -LiteralPath $proxy -Destination (Join-Path $gameDir 'ddraw.dll')
$ini = Get-Content -LiteralPath (Join-Path $data 'ddraw.ini') -Raw
foreach ($setting in @(@('width','640'),@('height','480'),@('windowed','true'),
    @('fullscreen','false'),@('renderer','gdi'),@('devmode','true'),@('maxfps','60'),
    @('posX','100'),@('posY','100'),@('savesettings','0'),@('adjmouse','false'))) {
    $ini = [regex]::Replace($ini, '(?m)^' + $setting[0] + '=.*$', $setting[0] + '=' + $setting[1])
}
[IO.File]::WriteAllText((Join-Path $gameDir 'ddraw.ini'), $ini)
# Both ReadConfig implementations read only the first 199 bytes. Use a short,
# complete deterministic config. Eggbert 2's AddCDPath prefixes the configured
# folder directly, so '.' is the isolated installation folder. Blupi 2's keeps
# SB1's CD layout: data, image* and movie lie one level above the configured
# folder (the shipped config.def has CD-Rom=H:\US), so it gets an empty US
# folder inside the copy; with CD-Rom=. its CheckWorld1 fails and the game
# shows the insert-CD screen at the first locked world.
# Timer=50 and SpeedRate=1 are the game's defaults. Benchmark>=3100 enables
# honoring TrueColorBack/Decor; Benchmark=0 would force BOTH depths to 8 bpp.
$cdRom = if ($Edition -eq 'blupi') { 'US' } else { '.' }
if ($cdRom -ne '.') { New-Item -ItemType Directory -Force -Path (Join-Path $gameDir $cdRom) | Out-Null }
$config = "CD-Rom=$cdRom`r`nFullScreen=1`r`nBenchmark=6666`r`nTrueColorBack=$ColorDepth`r`nTrueColorDecor=$ColorDepth`r`nMouseType=2`r`nCDAudio=0`r`n"
if ($config.Length -gt 199) { throw 'SB2 config exceeds ReadConfig buffer' }
$configPath = Join-Path $gameDir 'DATA\config.def'
[IO.File]::WriteAllText($configPath, $config, [Text.Encoding]::ASCII)
Copy-Item -LiteralPath $configPath -Destination (Join-Path $portUser 'data\config.def')
$portInput = @()
if ($InputScript) {
    Copy-Item -LiteralPath $InputScript -Destination (Join-Path $runDir 'input.txt')
    $InputScript = Join-Path $runDir 'input.txt'
    $portInput = @('--input', $InputScript)
}
# CacheAll(TRUE) displays init once. The application Benchmark displays ten
# times; CPixmap::Benchmark only blits. Timer frame 0 is display index 11,
# matching the proxy's existing post_input(displayIndex - 10) convention.
# Read pristine assets on the port side, with the identical config in its user
# root. Historical diagnostics/saves in game/ must not feed into the port run.
$portArgs = @('--headless','--reference','--data',$data,'--user',$portUser,
    '--frames',"$($Frames - 11)",'--display-log',(Join-Path $runDir 'port.log'),
    '--display-dump',"$($portFrames):$DumpEvery") + $portInput
# --display-dump requires positive N; zero means logging only.
if ($DumpEvery -eq 0) { $portArgs = $portArgs[0..9] + $portInput }
$receipt = [ordered]@{
    status = 'PREPARED'; target = $target; executable = $built; sha256 = $builtHash
    gate = $gatePath; gate_sha256 = (Get-FileHash $gatePath).Hash
    build = $buildPath; build_sha256 = (Get-FileHash $buildPath).Hash
    port = $port; port_sha256 = $portHash
    proxy_sha256 = (Get-FileHash $proxy).Hash
    cnc_ddraw_sha256 = (Get-FileHash (Join-Path $gameDir 'cnc-ddraw.dll')).Hash
    config_sha256 = (Get-FileHash $configPath).Hash
    frames = $Frames; startup_displays = 11; timer_frames = $Frames - 11; color_depth = $ColorDepth
    historical_arguments = @('-nocd'); port_arguments = $portArgs
    scope = 'Runtime evidence, not a strict/equivalence proof'
}
if ($InputScript) { $receipt.input = $InputScript; $receipt.input_sha256 = (Get-FileHash $InputScript).Hash }
$comparePath = Join-Path $runDir 'compare.json'
$receipt | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $comparePath
if ($PrepareOnly) { Write-Output $runDir; return }

$oracleEnv = @{
    BLUPI_ORACLE_LOG = Join-Path $runDir 'hist.log'
    BLUPI_ORACLE_DUMP_DIR = $histFrames
    BLUPI_ORACLE_DUMP_EVERY = "$DumpEvery"
    BLUPI_ORACLE_MAX_FRAMES = "$Frames"
    BLUPI_ORACLE_DEBUG = Join-Path $runDir 'hist-debug.txt'
    BLUPI_ORACLE_INPUT = $InputScript
}
$savedEnv = @{}
$game = $null
try {
    foreach ($name in $oracleEnv.Keys) {
        $savedEnv[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
        [Environment]::SetEnvironmentVariable($name, $oracleEnv[$name], 'Process')
    }
    if ((Get-FileHash (Join-Path $gameDir $exeName)).Hash -ne $builtHash) { throw 'Staged rebuild hash changed' }
    Add-Type -AssemblyName System.Windows.Forms
    [System.Windows.Forms.Cursor]::Position = New-Object System.Drawing.Point(0, 0)
    $limit = [int]($Frames * 0.05 * 2) + 30
    # Deliberately visible: the supervisor invokes this windowed capture.
    $game = Start-Process -FilePath (Join-Path $gameDir $exeName) -ArgumentList '-nocd' -WorkingDirectory $gameDir -PassThru
    $receipt.forced_termination = -not $game.WaitForExit($limit * 1000)
    if ($receipt.forced_termination) { $game.Kill(); $game.WaitForExit() }
    $receipt.exit_code = $game.ExitCode
    # Do not expose the historical proxy environment to the port.
    foreach ($name in $oracleEnv.Keys) { [Environment]::SetEnvironmentVariable($name, $savedEnv[$name], 'Process') }
    & $port @portArgs | Out-Null
    $receipt.port_exit_code = $LASTEXITCODE
    $hist = @(Get-Content -LiteralPath (Join-Path $runDir 'hist.log'))
    $mine = @(Get-Content -LiteralPath (Join-Path $runDir 'port.log'))
    $n = [Math]::Min($hist.Count, $mine.Count)
    $first = -1; $equal = 0
    for ($i = 0; $i -lt $n; $i++) {
        if ($hist[$i] -ceq $mine[$i]) { $equal++ }
        elseif ($first -eq -1) { $first = $i }
    }
    $receipt.hist_frames = $hist.Count; $receipt.port_frames = $mine.Count
    $receipt.compared = $n; $receipt.identical_frames = $equal; $receipt.first_difference = $first
    $complete = $hist.Count -eq $Frames -and $mine.Count -eq $Frames -and
        -not $receipt.forced_termination -and $receipt.port_exit_code -eq 0 -and $receipt.exit_code -eq 0
    $receipt.status = if (-not $complete) { 'INCOMPLETE' } elseif ($equal -eq $Frames) { 'MATCH' } else { 'DIFFERENT' }
} catch {
    $receipt.status = 'FAILED'; $receipt.error = $_.Exception.Message
    throw
} finally {
    if ($null -ne $game -and -not $game.HasExited) { $game.Kill(); $game.WaitForExit() }
    foreach ($name in $savedEnv.Keys) { [Environment]::SetEnvironmentVariable($name, $savedEnv[$name], 'Process') }
    $receipt | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $comparePath
    Write-Output $runDir
}
$receipt | ConvertTo-Json -Depth 5
