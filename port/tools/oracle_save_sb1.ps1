# Separate SB1 save oracle; oracle_capture.ps1 defaults remain unchanged.
# -PrepareOnly never launches either game. Runtime invocation is for supervisor.
param(
    [ValidateSet('blupi','eggbert')][string]$Edition = 'blupi',
    [ValidateSet(8,16)][int]$ColorDepth = 16,
    [string]$BuiltExe = 'build\tmp\relink-us\BLUPI.EXE',
    [string]$PortExe = 'build\port\blupi-sdl3.exe',
    [switch]$PrepareOnly,
    [switch]$PortOnly
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$argsForOracle = @((Join-Path $PSScriptRoot 'oracle_save_sb1.py'), '--edition', $Edition,
    '--color', "$ColorDepth", '--built-exe', $BuiltExe, '--port-exe', $PortExe)
if ($PrepareOnly) { $argsForOracle += '--prepare-only' }
if ($PortOnly) { $argsForOracle += '--port-only' }
& (Join-Path $repo '.pyenv\python.exe') @argsForOracle
if ($LASTEXITCODE -ne 0) { throw "SB1 save oracle failed (exit $LASTEXITCODE); inspect its compare.json" }
