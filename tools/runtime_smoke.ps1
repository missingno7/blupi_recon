param([int]$Seconds = 15, [switch]$Play, [switch]$Mission,
      [ValidateSet(8,16)][int]$ColorDepth = 16)
$ErrorActionPreference = 'Stop'
if ([Environment]::Is64BitProcess) {
    $forward = @('-NoProfile','-ExecutionPolicy','Bypass','-File',$PSCommandPath,'-Seconds',"$Seconds",'-ColorDepth',"$ColorDepth")
    if ($Play) { $forward += '-Play' }
    if ($Mission) { $forward += '-Mission' }
    & "$env:WINDIR\SysWOW64\WindowsPowerShell\v1.0\powershell.exe" @forward
    exit $LASTEXITCODE
}
if ($Seconds -lt 3 -or $Seconds -gt 45) { throw 'Seconds must be 3..45' }
$repo = Split-Path $PSScriptRoot -Parent
$original = Join-Path $repo 'assets\Speedy Blupi'
$built = Join-Path $repo 'build\standalone-validation\BLUPI.EXE'
$gate = Get-Content (Join-Path (Split-Path $built) 'gate.json') -Raw | ConvertFrom-Json
if ($gate.status -ne 'PASS') { throw 'Validated source build required' }
if ((Get-FileHash $built).Hash -eq (Get-FileHash (Join-Path $original 'BLUPI.EXE')).Hash) {
    throw 'Refusing original executable'
}
$runDir = Join-Path $repo ('build\runtime-' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $runDir | Out-Null
foreach ($name in @('DATA','IMAGE08','IMAGE16','SOUND','Shaders')) {
    Copy-Item -LiteralPath (Join-Path $original $name) -Destination $runDir -Recurse
}
Copy-Item -LiteralPath $built -Destination (Join-Path $runDir 'BLUPI.EXE')
Copy-Item -LiteralPath (Join-Path $original 'ddraw.dll') -Destination $runDir
$ini = Get-Content (Join-Path $original 'ddraw.ini') -Raw
foreach ($setting in @(@('width','640'),@('height','480'),@('windowed','true'),
    @('fullscreen','false'),@('renderer','gdi'),@('devmode','true'),@('maxfps','60'),
    @('posX','100'),@('posY','100'),@('savesettings','0'))) {
    $ini = [regex]::Replace($ini, '(?m)^' + $setting[0] + '=.*$', $setting[0] + '=' + $setting[1])
}
[IO.File]::WriteAllText((Join-Path $runDir 'ddraw.ini'), $ini)
$configPath = Join-Path $runDir 'DATA\config.def'
$config = Get-Content $configPath -Raw
# AddCDPath resolves data/image/movie relative to the language folder's parent.
$languageDir = Join-Path $runDir 'ENGLISH'
New-Item -ItemType Directory -Path $languageDir | Out-Null
$config = [regex]::Replace($config, '(?m)^CD-Rom=.*$', 'CD-Rom=' + $languageDir)
$config = [regex]::Replace($config, '(?m)^Benchmark=.*$', 'Benchmark=0')
# Benchmark >= 3100 implicitly selects 16-bit; make the desired mode explicit.
$config = [regex]::Replace($config, '(?m)^TrueColor=.*$', 'TrueColor=' + $ColorDepth)
[IO.File]::WriteAllText($configPath, $config)
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class BlupiSmokeWindow {
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
}
'@
[BlupiSmokeWindow]::SetProcessDPIAware() | Out-Null
$receipt = [ordered]@{
    executable = $built; sha256 = (Get-FileHash $built).Hash;
    wrapper_sha256 = (Get-FileHash (Join-Path $runDir 'ddraw.dll')).Hash;
    wrapper_version = (Get-Item (Join-Path $runDir 'ddraw.dll')).VersionInfo.FileVersion;
    runtime_dir = $runDir; seconds = $Seconds; renderer = 'gdi'; color_depth = $ColorDepth;
    scope = 'Bounded rebuilt-game startup smoke test, not equivalence proof';
    samples = @(); forced_termination = $false
}
$game = $null
try {
    # A visible window is intentional for this user-authorized interactive test.
    $game = Start-Process -FilePath (Join-Path $runDir 'BLUPI.EXE') -ArgumentList '-nocd' -WorkingDirectory $runDir -PassThru
    for ($elapsed = 0; $elapsed -lt $Seconds; $elapsed += 3) {
        Start-Sleep -Seconds 3
        $game.Refresh()
        if ($game.HasExited) { break }
        $receipt.samples += @{ seconds=$elapsed+3; title=$game.MainWindowTitle; responding=$game.Responding }
    }
    if (-not $game.HasExited) {
        $receipt.modules = @((Get-Process -Id $game.Id).Modules | ForEach-Object { $_.FileName })
        $receipt.wrapper_loaded = @($receipt.modules | Where-Object { $_ -eq (Join-Path $runDir 'ddraw.dll') }).Count -eq 1
        if (-not $receipt.wrapper_loaded) { throw 'Local cnc-ddraw module not confirmed' }
        $receipt.window_title = $game.MainWindowTitle
        $window = $game.MainWindowHandle
        if (($Play -or $Mission) -and $window -ne [IntPtr]::Zero) {
            # Title-screen dice button, observed at client (108,430).
            $point = [IntPtr](108 + (430 -shl 16))
            [BlupiSmokeWindow]::PostMessage($window,0x200,[IntPtr]::Zero,$point) | Out-Null
            [BlupiSmokeWindow]::PostMessage($window,0x201,[IntPtr]1,$point) | Out-Null
            [BlupiSmokeWindow]::PostMessage($window,0x202,[IntPtr]::Zero,$point) | Out-Null
            Start-Sleep -Seconds 3
            $receipt.play_clicked = $true
            if ($Mission) {
                # Observed Choose player screen: single-player dice at (85,430).
                $point = [IntPtr](85 + (430 -shl 16))
                [BlupiSmokeWindow]::PostMessage($window,0x200,[IntPtr]::Zero,$point) | Out-Null
                [BlupiSmokeWindow]::PostMessage($window,0x201,[IntPtr]1,$point) | Out-Null
                [BlupiSmokeWindow]::PostMessage($window,0x202,[IntPtr]::Zero,$point) | Out-Null
                Start-Sleep -Seconds 3
                # Enter is the source-defined INFO -> PLAY command (TreatEventBase).
                [BlupiSmokeWindow]::PostMessage($window,0x100,[IntPtr]13,[IntPtr]0x1c0001) | Out-Null
                [BlupiSmokeWindow]::PostMessage($window,0x101,[IntPtr]13,[IntPtr](-1071906815)) | Out-Null
                Start-Sleep -Seconds 3
                $receipt.mission_requested = $true
            }
        }
        if ($window -ne [IntPtr]::Zero) {
            [BlupiSmokeWindow]::SetForegroundWindow($window) | Out-Null
            Start-Sleep -Milliseconds 300
            $rect = New-Object BlupiSmokeWindow+RECT
            [BlupiSmokeWindow]::GetWindowRect($window, [ref]$rect) | Out-Null
            $bitmap = New-Object Drawing.Bitmap ($rect.R-$rect.L),($rect.B-$rect.T)
            $graphics = [Drawing.Graphics]::FromImage($bitmap)
            try {
                $graphics.CopyFromScreen($rect.L,$rect.T,0,0,$bitmap.Size)
                $bitmap.Save((Join-Path $runDir 'startup.png'),[Drawing.Imaging.ImageFormat]::Png)
            } finally { $graphics.Dispose(); $bitmap.Dispose() }
        }
        $receipt.close_requested = $game.CloseMainWindow()
        if (-not $game.WaitForExit(5000)) {
            $receipt.forced_termination = $true
            $game.Kill(); $game.WaitForExit()
        }
    }
    $receipt.exit_code = $game.ExitCode
    if (-not $receipt.wrapper_loaded -or $receipt.exit_code -ne 0 -or $receipt.forced_termination) {
        throw 'Runtime smoke test failed; inspect smoke.json'
    }
} finally {
    if ($null -ne $game -and -not $game.HasExited) {
        $receipt.forced_termination = $true
        $game.Kill(); $game.WaitForExit()
    }
    $receipt | ConvertTo-Json -Depth 5 | Set-Content (Join-Path $runDir 'smoke.json')
    Write-Output $runDir
}
