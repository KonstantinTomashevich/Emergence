#!/usr/bin/env pwsh

# TODO: Currently works only on Windows, because there is no
#       way to check is file executable, except for the extension.

if ($args.Count -ne 1)
{
    echo "Usage: <script> <path to binary dir>"
    exit 1
}

$BinaryDirectory = $args[0]
if (-Not(Test-Path $BinaryDirectory -PathType Container))
{
    echo "Unable to find given binary directory `"$BinaryDirectory`"!"
    exit 2
}

$ConfigurationFile = "Coverage.json"
if (-Not(Test-Path $ConfigurationFile -PathType Leaf))
{
    echo "Coverage configuration file `"$ConfigurationFile`" must exist in working directory!"
    exit 3
}

if (-Not(Get-Command llvm-profdata))
{
    echo "Unable to find llvm-profdata in path!"
    exit 4
}

if (-Not(Get-Command llvm-cov))
{
    echo "Unable to find llvm-cov in path!"
    exit 5
}

$Configuration = Get-Content $ConfigurationFile | ConvertFrom-Json
$InputDirectory = Join-Path $BinaryDirectory $Configuration.InputDirectory
echo "Scanning `"$InputDirectory`" for CLang coverage information."

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
            $ScanResult.Reports += $Item
        }
        elseif ($Extension -eq ".exe")
        {
            $ScanResult.Executables += $Item
        }
    }

    $ScanResult
}

$ScanResult = Find-Coverage-Data $InputDirectory
$Reports = $ScanResult.Reports
$Executables = $ScanResult.Executables

if ($Reports.Count -eq 0)
{
    echo "Unable to find any reports!"
    exit 6
}

echo "Found raw profile data:"
foreach ($File in $Reports)
{
    echo " - $File"
}

if ($Executables.Count -eq 0)
{
    echo "Unable to find any executables!"
    exit 7
}

echo "Found associated executables:"
foreach ($File in $Executables)
{
    echo " - $File"
}

echo "Creating output directory."
$OutputDirectory = Join-Path $BinaryDirectory $Configuration.OutputDirectory

if (-Not(Test-Path $OutputDirectory))
{
    New-Item -ItemType Directory -Path $OutputDirectory
}

echo "Merging found profile data."
$MergedProfdata = Join-Path $OutputDirectory $Configuration.MergedProfileDataFilename
llvm-profdata merge $Reports -o $MergedProfdata

$ExecutablesAsArguments = ""
foreach ($Executable in $ScanResult.Executables)
{
    $ExecutablesAsArguments += "-object `"$Executable`" "
}

# TODO: Expression invokations are used below, because otherwise all executable are merged into one argument.

echo "Exporting full source coverage."
$FullSourceCoverage = Join-Path $OutputDirectory $Configuration.FullSourceCoverageFileName
Invoke-Expression "llvm-cov show -instr-profile=`"$MergedProfdata`" $ExecutablesAsArguments > $FullSourceCoverage"

echo "Exporting textual coverage report."
$FullReport = Join-Path $OutputDirectory $Configuration.TextualReportFileName
Invoke-Expression "llvm-cov report -instr-profile=`"$MergedProfdata`" $ExecutablesAsArguments > $FullReport"

echo "Exporting json coverage report."
$FullReportJson = Join-Path $OutputDirectory $Configuration.JsonReportFileName
Invoke-Expression "llvm-cov export -format=text -summary-only -instr-profile=`"$MergedProfdata`" $ExecutablesAsArguments > $FullReportJson"