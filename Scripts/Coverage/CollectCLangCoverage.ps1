#!/usr/bin/env pwsh

# TODO: Currently works only on Windows, because there is no
#       way to check is file executable, except for the extension.

if ($args.Count -ne 1)
{
    echo "Usage: <script> <path to binary dir>"
    exit 1
}

if (-Not(Get-Command llvm-profdata))
{
    echo "Unable to find llvm-profdata in path!"
    exit 2
}

if (-Not(Get-Command llvm-cov))
{
    echo "Unable to find llvm-cov in path!"
    exit 3
}

$BinaryDir = $args[0]
echo "Collecting CLang coverage information for `"$binaryDir`"."

function Find-Coverage-Data
{
    param ([String]$Directory)

    $Children = Get-ChildItem $Directory
    $CoverageReports = @()
    $Executables = @()

    $ScanResult = [pscustomobject]@{
        Reports = @()
        Executables = @()
    }

    foreach ($Child in $Children)
    {
        $Item = Join-Path $Directory $Child
        $Extension = [IO.Path]::GetExtension($Item)

        if (Test-Path $Item -PathType Container)
        {
            $ChildResult = Find-Coverage-Data $Item
            $ScanResult.Reports += $ChildResult.Reports
            $ScanResult.Executables += $ChildResult.Executables
        }
        elseif ($Extension -eq ".profraw")
        {
            $CoverageReports += $Item
        }
        elseif ($Extension -eq ".exe")
        {
            $Executables += $Item
        }
    }

    if ($CoverageReports.Count -gt 0)
    {
        $ScanResult.Reports += $CoverageReports
        $ScanResult.Executables += $Executables
    }

    $ScanResult
}

$ScanResult = Find-Coverage-Data $BinaryDir
$Reports = $ScanResult.Reports
$OutputDirectory = Join-Path $BinaryDir "Coverage\Temporary"

if (-Not(Test-Path $OutputDirectory))
{
    New-Item -ItemType Directory -Path $OutputDirectory
}

$MergedProfdata = Join-Path $OutputDirectory "Emergence.profdata"
llvm-profdata merge $Reports -o "$MergedProfdata"

$ExecutablesAsArguments = ""
foreach ($Executable in $ScanResult.Executables)
{
    $ExecutablesAsArguments += "-object `"$Executable`" "
}

# TODO: Expression invokations are used below, because otherwise all executable are merged into one argument.

$FullSourceCoverage = Join-Path $OutputDirectory "SourceCoverage.txt"
Invoke-Expression "llvm-cov show -instr-profile=`"$MergedProfdata`" $ExecutablesAsArguments > $FullSourceCoverage"

$FullReport = Join-Path $OutputDirectory "Report.txt"
Invoke-Expression "llvm-cov report -instr-profile=`"$MergedProfdata`" $ExecutablesAsArguments > $FullReport"

$FullReportJson = Join-Path $OutputDirectory "Report.json"
Invoke-Expression "llvm-cov export -format=text -summary-only -instr-profile=`"$MergedProfdata`" $ExecutablesAsArguments > $FullReportJson"