#!/usr/bin/env pwsh

if ($args.Count -ne 2)
{
    echo "Usage: <script> <platform> <build_type>"
    exit -1
}

$Platform = $args[0]
$BuildType = $args[1]
$Dependencies = "PhysX"

foreach ($Dependency in $Dependencies)
{
    $SDKFileName = "Emergence_${Dependency}_SDK_${Platform}_${BuildType}.zip"
    $SDK = "https://sourceforge.net/projects/emergence/files/Dependencies/$Dependency/$SDKFileName"
    $DependencyDir = "$PWD\Dependency\$Dependency\$Platform\$BuildType"
    $Archive = "$DependencyDir\SDK.zip"

    if (-Not(Test-Path $DependencyDir -PathType Container))
    {
        echo "Downloading $Dependency SDK..."
        New-Item -ItemType "directory" -Path $DependencyDir

        $webclient = New-Object System.Net.WebClient
        $webclient.DownloadFile($SDK, $Archive)

        Expand-Archive -Path $Archive -DestinationPath $DependencyDir
        Remove-Item -Path $Archive
    }
    else
    {
        echo "$Dependency SDK already found!"
    }
}
