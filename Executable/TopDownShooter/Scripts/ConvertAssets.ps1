# Simple script for converting intermediate assets. Should be run from TopDownShooter directory.

if ($args.Count -ne 1)
{
    echo "Usage: <script> <path to Urho3D SDK>"
    exit -1
}

$IntermediatePath = "Assets/Intermediate"
$ConvertedPath = "Assets/Converted"
$ObjectsPath = "$ConvertedPath/Objects"

if (-Not(Test-Path $ObjectsPath -PathType Container))
{
    New-Item -ItemType "directory" -Path $ObjectsPath
}

$SDKPath = $args[0]
$ConverterPath = "$SDKPath\bin\tool\AssetImporter"
$Objects = Get-ChildItem $IntermediatePath -Filter "*.dae"

foreach ($Object in $Objects)
{
    $Name = $Object.Basename
    $Input = $Object.FullName

    $OutputFile = "$ObjectsPath\$Name.xml"
    & $ConverterPath node $Input $OutputFile -p $ConvertedPath
}
