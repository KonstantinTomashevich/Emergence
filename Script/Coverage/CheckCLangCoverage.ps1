#!/usr/bin/env pwsh

if ($args.Count -ne 1)
{
    echo "Usage: <script> <path to binary dir>"
    exit 1
}

$BinaryDir = $args[0]
if (-Not(Test-Path $BinaryDir -PathType Container))
{
    echo "Unable to find given binary directory `"$BinaryDir`"!"
    exit 2
}

$ConfigurationFile = "Coverage.json"
if (-Not(Test-Path $ConfigurationFile -PathType Leaf))
{
    echo "Coverage configuration file `"$ConfigurationFile`" must exist in working directory!"
    exit 3
}

$Configuration = Get-Content $ConfigurationFile | ConvertFrom-Json
$OutputDirectory = Join-Path $BinaryDir $Configuration.OutputDirectory
$Report = Join-Path $OutputDirectory $Configuration.JsonReportFileName
$MinimumCoveragePercent = $Configuration.MinimumLinesCoveragePerFilePercent

echo "Checking line coverage in report `"$Report`". Minimum approved coverage per file: $MinimumCoveragePercent%."
if (-Not(Test-Path $Report -PathType Leaf))
{
    echo "Unable to find report file!"
    exit 2
}

echo "Excluded:"
$Excludes = @()

foreach ($Exclude in $Configuration.Excludes)
{
    $Resolved = Resolve-Path $Exclude.Prefix
    $ExclusionReason = $Exclude.Reason
    $Excludes += $Resolved

    echo " - $Resolved"
    echo "   $ExclusionReason"
}

$CoverageSummary = Get-Content $Report | ConvertFrom-Json
$Files = $CoverageSummary.data.files
$FilesCount = $Files.Count
$Errors = @()

for ($Index = 0; $Index -lt $Files.Count; ++$Index)
{
    $File = $Files[$Index]
    $FileName = $File.filename
    $Excluded = 0

    foreach ($Exclusion in $Excludes)
    {
        if ( $FileName.StartsWith($Exclusion))
        {
            $Excluded = 1
            break
        }
    }

    $ReadableIndex = $Index + 1
    if (-Not$Excluded)
    {
        $LinesTotal = $File.summary.lines.count
        $LineCoverage = $File.summary.lines.percent

        if ($LineCoverage)
        {
            $FormattedCoverage = $LineCoverage.ToString("#.##")
            $CoverageToProgress = ", " + $LineCoverage.ToString("#.##") + "% coverage"
        }
        else
        {
            $CoverageToProgress = ""
        }

        echo "[$ReadableIndex/$FilesCount] $FileName ($LinesTotal executable lines$CoverageToProgress)"
        if ($LinesTotal -gt 0 -and $LineCoverage -lt $MinimumCoveragePercent)
        {
            $Errors += "File `"$FileName`" low coverage: $FormattedCoverage%."
        }
    }
    else
    {
        echo "[$ReadableIndex/$FilesCount] $FileName (excluded)"
    }
}

if ($Errors.Count -gt 0)
{
    $ErrorsCount = $Errors.Count
    echo "Found $ErrorsCount errors:"

    foreach ($CoverageError in $Errors)
    {
        echo " - $CoverageError"
    }

    exit 3
}
else
{
    echo "No errors detected!"
    exit 0
}
