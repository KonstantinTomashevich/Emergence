#!/usr/bin/env pwsh

if ($args.Count -lt 2)
{
    echo "Usage: <script> <clang json summary report> <minimum line coverage per file in percents> [excludes]..."
    exit 1
}

$Report = $args[0]
$MinimumCoveragePercent = [convert]::ToDouble($args[1])

echo "Checking line coverage in report `"$Report`". Minimum approved coverage per file: $MinimumCoveragePercent%."
if (-Not(Test-Path $Report -PathType Leaf))
{
    echo "Unable to find report file!"
    exit 2
}

echo "Excluded:"
$Excludes = @()

for ($index = 2; $index -lt $args.Count; ++$index)
{
    $Resolved = Resolve-Path $args[$index]
    $Excludes += $Resolved
    echo " - $Resolved"
}

$CoverageSummary = Get-Content $Report | ConvertFrom-Json
$Errors = 0

foreach ($File in $CoverageSummary.data.files)
{
    $LinesTotal = $File.summary.lines.count
    $LineCoverage = $File.summary.lines.percent
    $Filename = $File.filename

    $Empty = $LinesTotal -eq 0
    $Uncovered = $LineCoverage -lt $MinimumCoveragePercent

    if ($Empty -or $Uncovered)
    {
        $Excluded = 0
        foreach ($Exclusion in $Excludes)
        {
            if ($Filename.StartsWith($Exclusion))
            {
                $Excluded = 1
                break
            }
        }

        if(-Not$Excluded)
        {
            if ($Empty)
            {
                echo "Info: no coverable lines in `"$Filename`"!"
            }
            elseif ($Uncovered)
            {
                $FormattedCoverage = $LineCoverage.ToString("#.##")
                echo "Error: Not enough lines covered in `"$Filename`": $FormattedCoverage%."
                ++$Errors
            }
        }
    }
}

if ($Errors -gt 0)
{
    echo "Total errors: $Errors."
    exit 3
}
else
{
    echo "No errors detected!"
    exit 0
}
