param([switch]$Sanitize)
$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
$buildRoot = Join-Path $projectRoot $(if ($Sanitize) { 'build/asan' } else { 'build' })
New-Item -ItemType Directory -Path $buildRoot -Force | Out-Null
$sourceFiles = @(Get-ChildItem -LiteralPath $projectRoot -Filter '*.cpp' | Sort-Object Name)
$coreFiles = @($sourceFiles | Where-Object Name -ne 'main.cpp')
$testFile = Join-Path $projectRoot 'tests/SimWorldTests.cpp'
$gcc = Get-Command g++ -ErrorAction SilentlyContinue
if ($gcc) {
    $flags = @('-std=c++17', '-Wall', '-Wextra', '-Werror', '-pedantic', '-g')
    if ($Sanitize) { $flags += @('-fsanitize=address,undefined', '-fno-omit-frame-pointer') }
    & $gcc.Source @flags @($sourceFiles.FullName) '-o' (Join-Path $buildRoot 'simworld.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Application compilation failed.' }
    & $gcc.Source @flags '-I' $projectRoot @($coreFiles.FullName) $testFile '-o' (Join-Path $buildRoot 'simworld_tests.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed.' }
    & (Join-Path $buildRoot 'simworld_tests.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Regression tests failed.' }
} else {
    $vswhere = Join-Path ([Environment]::GetFolderPath('ProgramFilesX86')) 'Microsoft Visual Studio/Installer/vswhere.exe'
    if (!(Test-Path -LiteralPath $vswhere)) { throw 'Install g++ or the Visual Studio C++ desktop workload.' }
    $installation = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$installation) { throw 'Visual Studio C++ tools were not found.' }
    $vcvars = Join-Path $installation 'VC/Auxiliary/Build/vcvars64.bat'
    $flags = '/nologo /std:c++17 /EHsc /W4 /WX /Zi /Od /MDd /D_DEBUG'
    if ($Sanitize) { $flags += ' /fsanitize=address' }
    $quotedApp = ($sourceFiles.FullName | ForEach-Object { '"' + $_ + '"' }) -join ' '
    $quotedCore = ($coreFiles.FullName | ForEach-Object { '"' + $_ + '"' }) -join ' '
    $commands = @(
        '@echo off',
        ('call "' + $vcvars + '" >nul'),
        'if errorlevel 1 exit /b 1',
        'cd /d "%~dp0"',
        ('cl ' + $flags + ' /Fe:simworld.exe ' + $quotedApp + ' /link /INCREMENTAL:NO'),
        'if errorlevel 1 exit /b 1',
        ('cl ' + $flags + ' /I"' + $projectRoot + '" /Fe:simworld_tests.exe ' + $quotedCore + ' "' + $testFile + '" /link /INCREMENTAL:NO'),
        'if errorlevel 1 exit /b 1',
        'simworld_tests.exe',
        'exit /b %errorlevel%'
    )
    $commandFile = Join-Path $buildRoot 'compile-and-test.cmd'
    Set-Content -LiteralPath $commandFile -Value $commands -Encoding ASCII
    & $env:ComSpec /d /c $commandFile
    if ($LASTEXITCODE -ne 0) { throw 'Compilation or regression tests failed.' }
}
Write-Host "Built application: $(Join-Path $buildRoot 'simworld.exe')"
