#!/usr/bin/env pwsh

# TODO: Currently works only on Windows, because only way in powershell
#       to check that file is executable is to check its extension.

if (($args.Count -ne 1) -and ($args.Count -ne 2))
{
    echo "Usage: <script> <path to binary dir> [selected build config]"
    exit 1
}

$BinaryDirectory = $args[0]
if ($args.Count -eq 2)
{
    $SelectedBuildConfig = $args[1]
}

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

$LLVMProfdataPath = (Get-Command llvm-profdata).Path
echo "LLVM Profdata location: $LLVMProfdataPath"

if (-Not(Get-Command llvm-cov))
{
    echo "Unable to find llvm-cov in path!"
    exit 5
}

$LLVMCovPath = (Get-Command llvm-cov).Path
echo "LLVM Cov location: $LLVMCovPath"

$Configuration = Get-Content $ConfigurationFile | ConvertFrom-Json
$Filter = $Configuration.Filter
echo "Scanning `"$BinaryDirectory`" for CLang coverage information that matches filter `"$Filter`"."

function Belongs-To-Selected-Build-Config ([String]$DirectoryName)
{
    if ($SelectedBuildConfig -ne $null)
    {
        if (($DirectoryName -eq "Release") -or ($DirectoryName -eq "Debug") -or ($DirectoryName -eq "RelWithDebInfo"))
        {
            $DirectoryName -eq $SelectedBuildConfig
            return
        }
    }

    $True
}

function Find-Coverage-Data-Recursive([String]$Directory)
{
    $Children = Get-ChildItem $Directory
    $ScanResult = [PSCustomObject]@{
        RawProfileData = @()
        Executables = @()
    }

    foreach ($Child in $Children)
    {
        $Item = Join-Path $Directory $Child
        $Extension = [IO.Path]::GetExtension($Item)

        if (Test-Path $Item -PathType Container)
        {
            if (Belongs-To-Selected-Build-Config $Child)
            {
                $ChildResult = Find-Coverage-Data-Recursive $Item
                $ScanResult.RawProfileData += $ChildResult.RawProfileData
                $ScanResult.Executables += $ChildResult.Executables
            }
        }
        elseif ($Item -match $Filter)
        {
            if ($Extension -eq ".profraw")
            {
                $ScanResult.RawProfileData += $Item
            }
            elseif ($Extension -eq ".exe")
            {
                $ScanResult.Executables += $Item
            }
        }
    }

    $ScanResult
}

$ScanResult = Find-Coverage-Data-Recursive $BinaryDirectory
$RawProfileData = $ScanResult.RawProfileData
$Executables = $ScanResult.Executables

if ($RawProfileData.Count -eq 0)
{
    echo "Unable to find any raw profile data!"
    exit 6
}

echo "Found raw profile data:"
foreach ($File in $RawProfileData)
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
llvm-profdata merge $RawProfileData -o $MergedProfdata

if (-Not(Test-Path $MergedProfdata -PathType Leaf))
{
    echo "Unable to merge profdata!"
    exit 8
}

$ExecutablesAsArguments = ""
foreach ($Executable in $ScanResult.Executables)
{
    $ExecutablesAsArguments += "-object `"$Executable`" "
}

# Expression invokations are used below, because otherwise all executable are merged into one argument.

echo "Exporting full source coverage."
$FullSourceCoverage = Join-Path $OutputDirectory $Configuration.FullSourceCoverageFileName
Invoke-Expression "llvm-cov show -instr-profile=`"$MergedProfdata`" $ExecutablesAsArguments > $FullSourceCoverage"

echo "Exporting textual coverage report."
$FullReport = Join-Path $OutputDirectory $Configuration.TextualReportFileName
Invoke-Expression "llvm-cov report -instr-profile=`"$MergedProfdata`" $ExecutablesAsArguments > $FullReport"

echo "Exporting json coverage report."
$FullReportJson = Join-Path $OutputDirectory $Configuration.JsonReportFileName
Invoke-Expression "llvm-cov export -format=text -summary-only -instr-profile=`"$MergedProfdata`" $ExecutablesAsArguments > $FullReportJson"
