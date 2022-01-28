# Simple script for converting intermediate assets. Should be run from TopDownShooter directory.

if ($args.Count -ne 1)
{
    echo "Usage: <script> <path to Urho3D SDK>"
    exit -1
}

$IntermediatePath = "Assets/Intermediate"
$ConvertedPath = "Assets/Converted"
$SDKPath = $args[0]
$ConverterPath = "$SDKPath/bin/tool/AssetImporter"

$Models = Get-ChildItem $IntermediatePath -Filter "*.fbx"
foreach ($Model in $Models)
{
    $Name = $Model.Basename
    $Input = $Model.FullName
    $OutputPath = "$ConvertedPath\Objects\$Name"
    $OutputFile = "$OutputPath\$Name.mdl"

    if (-Not(Test-Path $OutputPath -PathType Container))
    {
        New-Item -ItemType "directory" -Path $OutputPath
    }

    & $ConverterPath model $Input $OutputFile -ns -p $OutputPath
}

exit 0
