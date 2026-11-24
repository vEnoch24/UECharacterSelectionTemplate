param(
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [string]$RunRoot,
    [ValidateSet('All','Package','Host','Fixture','Cook','Run')][string]$Stage = 'All'
)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
if (!$RunRoot) { $RunRoot = Join-Path $repoRoot ('Saved\V' + (Get-Date -Format 'MMddHHmmss')) }
$RunRoot = [IO.Path]::GetFullPath($RunRoot)
$savedRoot = [IO.Path]::GetFullPath((Join-Path $repoRoot 'Saved')) + '\'
if (!$RunRoot.StartsWith($savedRoot, [StringComparison]::OrdinalIgnoreCase)) { throw 'RunRoot must be under this project Saved directory.' }
$package = Join-Path $RunRoot 'P'
$hostRoot = Join-Path $RunRoot 'H'
$project = Join-Path $hostRoot 'SelectionReleaseHost.uproject'
$archive = Join-Path $RunRoot 'G'
$uat = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
$build = Join-Path $EngineRoot 'Engine\Build\BatchFiles\Build.bat'
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
function Invoke-Checked([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program exited with code $LASTEXITCODE" }
}
New-Item -ItemType Directory -Force -Path $RunRoot | Out-Null
if ($Stage -in @('All','Package')) {
    if (Test-Path -LiteralPath $package) { throw 'Package output already exists. Choose a new RunRoot; existing builds are never deleted by this script.' }
    Invoke-Checked $uat @('BuildPlugin', "-Plugin=$repoRoot\Plugins\FightingCharacterSelect\FightingCharacterSelect.uplugin", "-Package=$package", '-TargetPlatforms=Win64', '-NoP4')
    # BuildPlugin's default filters do not include arbitrary Markdown files.
    foreach ($document in @('README.md','RELEASE_CHECKLIST.md')) {
        Copy-Item -LiteralPath (Join-Path $repoRoot "Plugins\FightingCharacterSelect\$document") -Destination $package
    }
}
if ($Stage -in @('All','Host')) {
    if (Test-Path -LiteralPath $hostRoot) { throw 'Host output already exists. Choose a new RunRoot.' }
    Copy-Item -LiteralPath (Join-Path $repoRoot 'Tests\ReleaseHost') -Destination $hostRoot -Recurse
    New-Item -ItemType Directory -Path (Join-Path $hostRoot 'Plugins') | Out-Null
    Copy-Item -LiteralPath $package -Destination (Join-Path $hostRoot 'Plugins\FightingCharacterSelect') -Recurse
    Invoke-Checked $build @('SelectionReleaseHostEditor','Win64','Development',"-Project=$project",'-WaitMutex','-NoHotReloadFromIDE','-NoUBA','-MaxParallelActions=1')
}
if ($Stage -in @('All','Fixture')) {
    Invoke-Checked $editor @($project,'-run=SelectionFixture','-unattended','-NullRHI','-nosound','-nop4',"-abslog=$RunRoot\Fixture.log")
}
if ($Stage -in @('All','Cook')) {
    Invoke-Checked $uat @('BuildCookRun',"-project=$project",'-noP4','-platform=Win64','-clientconfig=Development','-build','-cook','-stage','-pak','-iostore','-package','-archive',"-archivedirectory=$archive",'-map=/Game/SelectionVerification/Selection+/Game/SelectionVerification/Destination','-unattended','-NoCompileEditor','-ubtargs=-NoUBA -MaxParallelActions=1')
}
if ($Stage -in @('All','Run')) {
    $exe = Join-Path $archive 'Windows\SelectionReleaseHost\Binaries\Win64\SelectionReleaseHost.exe'
    foreach ($mode in @('CPU','Mixed','Pads')) {
        $log = Join-Path $RunRoot ($mode + '.log')
        # Packaged games are Windows GUI executables: PowerShell's call operator
        # can return before they exit and leave LASTEXITCODE unset.
        $process = Start-Process -FilePath $exe -ArgumentList @('-unattended','-NullRHI','-nosound',"-SelectionSmoke=$mode", "`"-abslog=$log`"") -WindowStyle Hidden -PassThru
        if (!$process.WaitForExit(90000)) {
            Stop-Process -Id $process.Id
            throw "Packaged $mode test timed out after 90 seconds. Inspect $log"
        }
        if ($process.ExitCode -ne 0) { throw "Packaged $mode test exited with code $($process.ExitCode). Inspect $log" }
        if (!(Select-String -LiteralPath $log -Pattern 'CHARSELECT_RELEASE_PASS' -Quiet)) { throw "No success marker for $mode" }
    }
}
Write-Output "Release verification stage $Stage finished. Evidence: $RunRoot"
