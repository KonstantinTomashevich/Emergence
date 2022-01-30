#!/usr/bin/env pwsh

if ($args.Count -ne 1)
{
    echo "Usage: <script> <platform>"
    exit -1
}

$Platform = $args[0]
if ($Platform -eq "Windows")
{
    $UrhoSDK =
    "https://sourceforge.net/projects/urho3d/files/Urho3D/1.8-ALPHA/Urho3D-1.8.ALPHA-Windows-64bit-SHARED-3D11.zip"
}
else
{
    echo "Only 'Windows' platform is supported right now."
    exit -2
}

$DependencyDir = ".\Dependency\Urho3D\$Platform"
$Archive = "$PWD\$DependencyDir\UrhoSDK.zip" # We need to specify absolute path, because DownloadFile ignores PWD.

if (-Not(Test-Path $DependencyDir -PathType Container))
{
    echo "Downloading Urho3D SDK..."
    New-Item -ItemType "directory" -Path $DependencyDir

    $webclient = New-Object System.Net.WebClient
    $webclient.DownloadFile($UrhoSDK, $Archive)

    Expand-Archive -Path $Archive -DestinationPath $DependencyDir
    Remove-Item -Path $Archive

    # Urho3D SDK comes in one directory, that is named after version. We should unpack it.
    $DependencyDirChildren = Get-ChildItem $DependencyDir

    if (($DependencyDirChildren.Count -ne 1) -or -Not(Test-Path -Path "$DependencyDir/$DependencyDirChildren" -PathType Container))
    {
        echo "Urho3D SDK structure has changed. Review this script."
        exit -3
    }

    $UrhoSDKChildren = Get-ChildItem "$DependencyDir\$DependencyDirChildren"
    foreach ($Child in $UrhoSDKChildren)
    {
        Move-Item -Path "$DependencyDir\$DependencyDirChildren\$Child" -Destination "$DependencyDir\$Child"
    }

    Remove-Item -Path "$DependencyDir\$DependencyDirChildren"
}
else
{
    echo "Urho3D SDK already found!"
}
