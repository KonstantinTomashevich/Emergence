#!/usr/bin/env pwsh

if ($args.Count -ne 1)
{
    echo "Usage: <script> <platform>"
    exit -1
}

$Platform = $args[0]
if ($Platform -eq "Windows")
{
    $OgreSDK = "https://dl.cloudsmith.io/public/ogrecave/ogre/raw/files/ogre-sdk-v13.2.0-msvc141-x64.zip"
}
else
{
    echo "Only 'Windows' platform is supported right now."
    exit -2
}

# Surprisingly, progress bar logic slows down download speed by ~90%. Therefore we disable progress bar.
$ProgressPreference = "SilentlyContinue"

$DependencyDir = ".\Dependency\OGRE\$Platform"
$Archive = "$DependencyDir\SDK.zip"

if (-Not(Test-Path $DependencyDir -PathType Container))
{
    echo "Downloading OGRE SDK..."
    New-Item -ItemType "directory" -Path $DependencyDir
    Invoke-WebRequest -Uri $OgreSDK -OutFile $Archive
    Expand-Archive -Path $Archive -DestinationPath $DependencyDir
    Remove-Item -Path $Archive
}
else
{
    echo "OGRE SDK already found!"
}
