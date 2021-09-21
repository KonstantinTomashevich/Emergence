#!/usr/bin/env pwsh

if ($args.Count -gt 1)
{
    echo "Usage: <script> <path to compilation database>"
    exit -1
}

$CLangTidyExecutable = "clang-tidy"
$CompilationDatabase = $args[0]

if (-Not(Test-Path $CompilationDatabase -PathType Leaf))
{
    echo "Unable to open compilation database $CompilationDatabase!"
    exit -2
}

$Sources = @()
$RootChildren = Get-ChildItem -Directory

foreach ($RootChild in $RootChildren)
{
    #if (($RootChild.Name -ne "ThirdParty") -and ($RootChild.Name -ne "Build"))
    if ($RootChild.Name -eq "Library")
    {
        $Sources += Get-ChildItem -Path $RootChild -Recurse -Include "*.cpp"
    }
}

& $CLangTidyExecutable $Sources -p="$CompilationDatabase"
if ($?)
{
    exit 0
}
else
{
    exit -3
}
