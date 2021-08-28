#!/usr/bin/env pwsh

if ($args.Count -gt 1)
{
    echo "Usage: <script> [clang-format-version-major]"
    exit -1
}

$CLangFormatExecutable = "clang-format"
if ($args.Count -eq 1)
{
    $CLangFormatExecutable += "-" + $args[0]
}

if (-Not(Get-Command $CLangFormatExecutable))
{
    echo "Unable to find $CLangFormatExecutable in path!"
    exit -2
}

$Sources = @()
$RootChildren = Get-ChildItem -Directory

foreach ($RootChild in $RootChildren)
{
    if (($RootChild.Name -ne "ThirdParty") -and ($RootChild.Name -ne "Build"))
    {
        $Sources += Get-ChildItem -Path $RootChild -Recurse -Include "*.cpp", "*.hpp"
    }
}

& $CLangFormatExecutable --Werror --dry-run $Sources
