#!/usr/bin/env pwsh

if ($args.Count -gt 1)
{
    echo "Usage: <script> <path to build directory>"
    exit -1
}

$BuildDirectory = $args[0]
$CompilationDatabase = "$BuildDirectory\compile_commands.json"

if (-Not(Test-Path $CompilationDatabase -PathType Leaf))
{
    echo "Unable to open compilation database $CompilationDatabase!"
    exit -2
}

# Not all distributions provide run-clang-tidy.py script, therefore we download it manually.
$ClangTidyScriptUri =
"https://raw.githubusercontent.com/llvm/llvm-project/main/clang-tools-extra/clang-tidy/tool/run-clang-tidy.py"

$CLangTidyScript = "$env:TEMP\run-clang-tidy.py"
Invoke-WebRequest -Uri $ClangTidyScriptUri -OutFile $CLangTidyScript

if (-Not(Test-Path $CLangTidyScript -PathType Leaf))
{
    echo "Unable to download run-clang-tidy.py!"
    exit -3
}

python $CLangTidyScript files="Executable|Library|Service|Test" -p "$BuildDirectory"
if ($?)
{
    exit 0
}
else
{
    exit -4
}
