#!/usr/bin/env pwsh

# TODO: If file wasn't used at all (and therefore has 0% coverage), it will not be listed and coverage report
#       and will not trigger coverage error. This behaviour seems incorrect.

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
[Double]$MinimumCoveragePercent = $Configuration.MinimumLinesCoveragePerFilePercent

echo "Checking line coverage in report `"$Report`". Minimum approved coverage per file: $MinimumCoveragePercent%."
if (-Not(Test-Path $Report -PathType Leaf))
{
    echo "Unable to find report file!"
    exit 2
}

echo "Rules:"
$Rules = @()

foreach ($Rule in $Configuration.Rules)
{
    if ($Rule.Action -eq "Exclude")
    {
        $ReadableAction = "Do not check coverage"
    }
    elseif ($Rule.Action -eq "CustomMinimumCoverage")
    {
        $CustomMinimumCoverage = $Rule.MinimumLinesCoveragePerFilePercent
        $ReadableAction = "Change coverage barrier to $CustomMinimumCoverage%"
    }
    else
    {
        $Action = $Rule.Action
        echo "Unknown rule action `"$Action`"!"
        exit 3
    }

    $Rule.Prefix = Resolve-Path $Rule.Prefix
    $Prefix = $Rule.Prefix
    $Reason = $Rule.Reason

    echo " - $ReadableAction in `"$Prefix`""
    echo "   $Reason"

    foreach ($RegisterRule in $Rules)
    {
        [String]$FirstPrefix = $RegisterRule.Prefix
        [String]$SecondPrefix = $Rule.Prefix

        if ($FirstPrefix.StartsWith($SecondPrefix) -or $SecondPrefix.StartsWith($FirstPrefix))
        {
            echo "Found overlapping rules with prefixes `"$FirstPrefix`" and `"$SecondPrefix`"!"
            exit 4
        }
    }

    $Rules += $Rule
}

$CoverageSummary = Get-Content $Report | ConvertFrom-Json
$Files = $CoverageSummary.data.files
$FilesCount = $Files.Count
$Errors = @()

for ($Index = 0; $Index -lt $Files.Count; ++$Index)
{
    $File = $Files[$Index]
    $FileName = $File.filename.Replace("/", "\")
    $Excluded = 0
    [Double]$MinimumCoverageForFile = $MinimumCoveragePercent

    foreach ($Rule in $Rules)
    {
        if ( $FileName.StartsWith($Rule.Prefix))
        {
            if ($Rule.Action -eq "Exclude")
            {
                $Excluded = 1
            }
            elseif ($Rule.Action -eq "CustomMinimumCoverage")
            {
                $MinimumCoverageForFile = $Rule.MinimumLinesCoveragePerFilePercent
            }

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
            $FormattedCoverage = "?"
            $CoverageToProgress = ""
        }

        echo "[$ReadableIndex/$FilesCount] $FileName ($LinesTotal executable lines$CoverageToProgress)"
        if ($LinesTotal -gt 0 -and $LineCoverage -lt $MinimumCoverageForFile)
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

    exit 5
}
else
{
    echo "No errors detected!"
    exit 0
}
