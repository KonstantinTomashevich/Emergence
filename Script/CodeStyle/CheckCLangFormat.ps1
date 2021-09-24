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
if ($?)
{
    exit 0
}
else
{
    exit -3
}
