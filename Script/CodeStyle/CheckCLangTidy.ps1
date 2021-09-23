#!/usr/bin/env pwsh

if ($args.Count -gt 1)
{
    echo "Usage: <script> <path to build directory>"
    exit -1
}

# Not all distributions provide run-clang-tidy.py script, therefore we download it manually.
$ClangTidyScriptUri =
"https://raw.githubusercontent.com/llvm-mirror/clang-tools-extra/master/clang-tidy/tool/run-clang-tidy.py"

$CLangTidyScript = "$env:TEMP\run-clang-tidy.py"
Invoke-WebRequest -Uri $ClangTidyScriptUri -OutFile $CLangTidyScript

$BuildDirectory = $args[0]
$CompilationDatabase = "$BuildDirectory\compile_commands.json"

if (-Not(Test-Path $CompilationDatabase -PathType Leaf))
{
    echo "Unable to open compilation database $CompilationDatabase!"
    exit -2
}

& $CLangTidyScript files="(Executable)|(Library)|(Service)|(Test)" -p="$BuildDirectory"
if ($?)
{
    exit 0
}
else
{
    exit -3
}
