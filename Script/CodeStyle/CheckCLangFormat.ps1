#!/usr/bin/env pwsh

if ($args.Count -gt 1)
{
    echo "Usage: <script> [clang-format-version]"
    exit -1
}

$CLangFormatExecutable = "clang-format"
if ($args.Count -eq 1)
{
    $CLangFormatExecutable += "-" + $args[0]
}

if (-Not(Get-Command $CLangFormatExecutable))
{
    echo "Unable to find clang-format in path!"
    exit -2
}

$Sources = @()
$RootChildren = Get-ChildItem -Directory

foreach ($RootChild in $RootChildren)
{
    if (($RootChild.Name -ne "ThirdParty") -and ($RootChild.Name -ne "Build"))
    {
        $Sources += Get-ChildItem -Path $RootChild -Recurse -Include "*.hpp", "*.cpp"
    }
}

& $CLangFormatExecutable --Werror --dry-run $Sources
