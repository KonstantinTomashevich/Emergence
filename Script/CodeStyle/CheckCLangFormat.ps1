#!/usr/bin/env pwsh

if ($args.Count -ne 0)
{
    echo "Usage: <script>"
    exit -1
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
            & $CLangFormatExecutable --Werror --dry-run $Source
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
