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
    if (($RootChild.Name -ne "ThirdParty") -and ($RootChild.Name -ne "Build"))
    {
        $Sources += Get-ChildItem -Path $RootChild -Recurse -Include "*.cpp"
    }
}

$Output = & $CLangTidyExecutable $Sources -p="$CompilationDatabase"
$Errors = $Output | Select-String -Pattern "\.((hpp)|(cpp)):[0-9]+:[0-9]+:\serror:"

if ($Errors.Count -gt 0)
{
    echo "Check failed! Errors:"
    foreach ($Error in $Errors)
    {
        echo $Error
    }

    exit -3
}
else
{
    exit 0
}
