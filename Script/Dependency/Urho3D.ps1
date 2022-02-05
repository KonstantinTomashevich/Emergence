#!/usr/bin/env pwsh

if ($args.Count -ne 2)
{
    echo "Usage: <script> <platform> <build_type>"
    exit -1
}

$Platform = $args[0]
$BuildType = $args[1]

if ($Platform -eq "Windows")
{
    if ($BuildType -eq "Release")
    {
        $Urho3DSDK = "https://sourceforge.net/projects/emergence/files/Dependencies/Urho3D/Emergence_Urho3D_SDK_Windows_Release.zip"
    }
    elseif ($BuildType -eq "Debug")
    {
        $Urho3DSDK = "https://sourceforge.net/projects/emergence/files/Dependencies/Urho3D/Emergence_Urho3D_SDK_Windows_Debug.zip"
    }
    else
    {
        echo "Unknown build type \"$BuildType\"."
        exit -2
    }
}
else
{
    echo "Only 'Windows' platform is supported right now."
    exit -3
}

$DependencyDir = "$PWD\Dependency\Urho3D\$Platform\$BuildType"
$Archive = "$DependencyDir\SDK.zip"

if (-Not(Test-Path $DependencyDir -PathType Container))
{
    echo "Downloading Urho3D SDK..."
    New-Item -ItemType "directory" -Path $DependencyDir

    $webclient = New-Object System.Net.WebClient
    $webclient.DownloadFile($Urho3DSDK, $Archive)

    Expand-Archive -Path $Archive -DestinationPath $DependencyDir
    Remove-Item -Path $Archive
}
else
{
    echo "Urho3D SDK already found!"
}
