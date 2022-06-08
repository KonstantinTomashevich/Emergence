# Simple script for converting intermediate assets. Should be run from TopDownShooter directory.

if ($args.Count -ne 1)
{
    echo "Usage: <script> <path to Urho3D SDK>"
    exit -1
}

$IntermediatePath = "Assets/Intermediate"
$ConvertedPath = "Assets/Converted"
$ModelsPath = "$ConvertedPath/Models"

if (-Not(Test-Path $ModelsPath -PathType Container))
{
    New-Item -ItemType "directory" -Path $ModelsPath
}

$SDKPath = $args[0]
$ConverterPath = "$SDKPath\bin\tool\AssetImporter"
$Objects = Get-ChildItem $IntermediatePath -Filter "*.fbx"

foreach ($Object in $Objects)
{
    $Name = $Object.Basename
    $Input = $Object.FullName

    $OutputFile = "$ModelsPath\$Name.mdl"
    & $ConverterPath model $Input $OutputFile -p $ConvertedPath
}
