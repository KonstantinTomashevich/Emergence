#!/usr/bin/env pwsh

if ($args.Count -ne 0)
{
    echo "Usage: <script>"
    exit -1
}

if (-Not(Get-Command clang-format))
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

clang-format --Werror --dry-run $Sources