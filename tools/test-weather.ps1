param(
    [string]$BaseUrl = "http://halo-cst.local",
    [int]$FetchTimeoutSeconds = 30
)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Net.Http
$client = [System.Net.Http.HttpClient]::new()
$client.Timeout = [TimeSpan]::FromSeconds(10)
$passed = 0
$failed = 0

function Assert-Weather([bool]$Condition, [string]$Name) {
    if ($Condition) { $script:passed++; Write-Host "PASS: $Name" -ForegroundColor Green }
    else { $script:failed++; Write-Host "FAIL: $Name" -ForegroundColor Red }
}

function Get-WeatherStatus {
    $json = $client.GetStringAsync("$($BaseUrl.TrimEnd('/'))/api/status").GetAwaiter().GetResult()
    $json | ConvertFrom-Json
}

function Request-Refresh {
    $content = [System.Net.Http.StringContent]::new("")
    $response = $client.PostAsync("$($BaseUrl.TrimEnd('/'))/api/weather/refresh", $content).GetAwaiter().GetResult()
    [int]$response.StatusCode
}

try {
    $deadline = (Get-Date).AddSeconds($FetchTimeoutSeconds)
    $status = Get-WeatherStatus
    while (-not [bool]$status.weatherAvailable -and (Get-Date) -lt $deadline) {
        Start-Sleep -Seconds 1
        $status = Get-WeatherStatus
    }

    Assert-Weather ([bool]$status.weatherAvailable) "successful weather fetch is available"
    Assert-Weather ($status.weatherStale -is [bool]) "stale flag is boolean"
    Assert-Weather ($null -ne $status.temperature) "temperature is present"
    Assert-Weather (-not [string]::IsNullOrWhiteSpace([string]$status.condition)) "condition is present"
    Assert-Weather (([int]$status.humidity -ge 0) -and ([int]$status.humidity -le 100)) "humidity is in range"
    Assert-Weather ([double]$status.windSpeed -ge 0) "wind speed is non-negative"
    Assert-Weather ([long]$status.weatherLastUpdate -gt 0) "last update is present"
    Assert-Weather ([string]::IsNullOrEmpty([string]$status.weatherError)) "successful fetch has no weather error"

    $first = Request-Refresh
    if ($first -eq 429) {
        Write-Host "Refresh window already active; waiting for rate limit to expire..."
        Start-Sleep -Seconds 31
        $first = Request-Refresh
    }
    Assert-Weather ($first -eq 202) "manual refresh is accepted"
    $second = Request-Refresh
    Assert-Weather ($second -eq 429) "immediate repeated refresh is rate limited"

    $deadline = (Get-Date).AddSeconds($FetchTimeoutSeconds)
    do {
        Start-Sleep -Seconds 1
        $after = Get-WeatherStatus
    } while ([string]$after.weatherError -ne "" -and (Get-Date) -lt $deadline)
    Assert-Weather ([bool]$after.weatherAvailable) "cached weather remains available after refresh"
}
finally {
    $client.Dispose()
}

Write-Host "WEATHER TEST SUMMARY: $passed passed, $failed failed"
if ($failed -gt 0) { exit 1 }
