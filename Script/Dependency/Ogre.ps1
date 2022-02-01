#!/usr/bin/env pwsh

if ($args.Count -ne 1)
{
    echo "Usage: <script> <platform>"
    exit -1
}

$Platform = $args[0]
if ($Platform -eq "Windows")
{
    $OgreSDK = "https://dl.cloudsmith.io/public/ogrecave/ogre/raw/files/ogre-sdk-v13.2.4-msvc141-x64.zip"
}
else
{
    echo "Only 'Windows' platform is supported right now."
    exit -2
}

$DependencyDir = "$PWD\Dependency\OGRE\$Platform"
$Archive = "$DependencyDir\SDK.zip"

if (-Not(Test-Path $DependencyDir -PathType Container))
{
    echo "Downloading OGRE SDK..."
    New-Item -ItemType "directory" -Path $DependencyDir

    $webclient = New-Object System.Net.WebClient
    $webclient.DownloadFile($OgreSDK, $Archive)

    Expand-Archive -Path $Archive -DestinationPath $DependencyDir
    Remove-Item -Path $Archive
}
else
{
    echo "OGRE SDK already found!"
}
