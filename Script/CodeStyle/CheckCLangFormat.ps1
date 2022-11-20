#!/usr/bin/env pwsh

if (($args.Count -gt 1) -or (($args.Count -eq 1) -and -not ($args[0] -eq "FixFormatting")))
{
    echo "Usage: <script> [FixFormatting]"
    exit -1
}

$RunArgument = "--dry-run"
if (($args.Count -eq 1) -and ($args[0] -eq "FixFormatting"))
{
    $RunArgument = "-i"
}

$CLangFormatExecutable = "clang-format"
if (-Not(Get-Command $CLangFormatExecutable))
{
    echo "Unable to find $CLangFormatExecutable in path!"
    exit -2
}

$AllChecksSuccessful = $true
$RootChildren = Get-ChildItem -Directory

foreach ($RootChild in $RootChildren)
{
    if (($RootChild.Name -ne "ThirdParty") -and ($RootChild.Name -ne "Build") -and ($RootChild.Name -ne "Dependency"))
    {
        $Sources = Get-ChildItem -Path $RootChild -Recurse -Include "*.cpp", "*.hpp" -Exclude "*.generated.cpp", "*.generated.hpp"
        foreach ($Source in $Sources)
        {
            & $CLangFormatExecutable --Werror $RunArgument $Source
            if (-not$?)
            {
                $AllChecksSuccessful = $false
            }
        }
    }
}

if ($AllChecksSuccessful)
{
    exit 0
}
else
{
    exit -3
}
