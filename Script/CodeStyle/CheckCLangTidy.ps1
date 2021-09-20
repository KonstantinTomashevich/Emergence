#!/usr/bin/env pwsh

if ($args.Count -gt 2 -Or $args.Count -lt 1)
{
    echo "Usage: <script> <path to compilation database> [clang-tidy-version-major]"
    exit -1
}

$CLangTidyExecutable = "clang-tidy"
$CompilationDatabase = $args[0]

if (-Not(Test-Path $CompilationDatabase -PathType Leaf))
{
    echo "Unable to open compilation database $CompilationDatabase!"
    exit - 2
}

if ($args.Count -eq 2)
{
    $CLangTidyExecutable += "-" + $args[1]
}

if (-Not(Get-Command $CLangTidyExecutable))
{
    echo "Unable to find $CLangTidyExecutable in path!"
    exit -3
}

$Sources = @()
$RootChildren = Get-ChildItem -Directory

foreach ($RootChild in $RootChildren)
{
    if (($RootChild.Name -ne "ThirdParty") -and ($RootChild.Name -ne "Build"))
    {
        $Sources += Get-ChildItem -Path $RootChild -Recurse -Include "*.cpp"
    }
}

& $CLangTidyExecutable $Sources -p="$CompilationDatabase"
