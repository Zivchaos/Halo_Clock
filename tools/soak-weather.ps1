param(
    [string]$BaseUrl = "http://halo-cst.local",
    [int]$Minutes = 30,
    [int]$IntervalSeconds = 3
)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Net.Http
$client = [System.Net.Http.HttpClient]::new()
$client.Timeout = [TimeSpan]::FromSeconds(10)
$logDirectory = Join-Path (Split-Path $PSScriptRoot -Parent) "logs"
New-Item -ItemType Directory -Force -Path $logDirectory | Out-Null
$logPath = Join-Path $logDirectory ("weather-soak-{0}.csv" -f (Get-Date -Format "yyyyMMdd-HHmmss"))
$deadline = (Get-Date).AddMinutes($Minutes)
$total = 0; $passed = 0; $failed = 0; $reboots = 0; $weatherLosses = 0
$lastUptime = $null; $lastWeatherUpdate = $null; $weatherUpdates = 0
$responseTimes = [System.Collections.Generic.List[double]]::new()

try {
    while ((Get-Date) -lt $deadline) {
        $started = Get-Date
        $total++
        $ok = $false; $errorText = ""; $status = $null
        try {
            $json = $client.GetStringAsync("$($BaseUrl.TrimEnd('/'))/api/status").GetAwaiter().GetResult()
            $status = $json | ConvertFrom-Json
            $elapsed = ((Get-Date) - $started).TotalMilliseconds
            $responseTimes.Add($elapsed)
            if (-not [bool]$status.wifiConnected) { throw "Wi-Fi disconnected" }
            if (-not [bool]$status.weatherAvailable) { $weatherLosses++; throw "Weather unavailable" }
            if ($null -eq $status.temperature -or [string]::IsNullOrWhiteSpace([string]$status.condition)) { throw "Weather fields missing" }
            if ($null -ne $lastUptime -and [long]$status.uptimeSeconds -lt $lastUptime) { $reboots++ }
            if ($null -ne $lastWeatherUpdate -and [long]$status.weatherLastUpdate -ne $lastWeatherUpdate) { $weatherUpdates++ }
            $lastUptime = [long]$status.uptimeSeconds
            $lastWeatherUpdate = [long]$status.weatherLastUpdate
            $passed++
            $ok = $true
        } catch {
            $failed++
            $errorText = $_.Exception.Message
            $elapsed = ((Get-Date) - $started).TotalMilliseconds
        }

        [pscustomobject]@{
            timestamp = (Get-Date).ToString("o"); success = $ok; responseMilliseconds = [math]::Round($elapsed,2)
            uptimeSeconds = if ($status) {$status.uptimeSeconds} else {$null}; freeHeap = if ($status) {$status.freeHeap} else {$null}
            weatherAvailable = if ($status) {$status.weatherAvailable} else {$null}; weatherStale = if ($status) {$status.weatherStale} else {$null}
            temperature = if ($status) {$status.temperature} else {$null}; condition = if ($status) {$status.condition} else {$null}
            weatherLastUpdate = if ($status) {$status.weatherLastUpdate} else {$null}; weatherError = if ($status) {$status.weatherError} else {$null}
            error = $errorText
        } | Export-Csv -LiteralPath $logPath -Append -NoTypeInformation

        $remaining = ($deadline - (Get-Date)).TotalSeconds
        if ($remaining -gt 0) { Start-Sleep -Seconds ([math]::Min($IntervalSeconds, [math]::Ceiling($remaining))) }
    }
}
finally {
    $client.Dispose()
}

$minimum = if ($responseTimes.Count) { ($responseTimes | Measure-Object -Minimum).Minimum } else { 0 }
$average = if ($responseTimes.Count) { ($responseTimes | Measure-Object -Average).Average } else { 0 }
$maximum = if ($responseTimes.Count) { ($responseTimes | Measure-Object -Maximum).Maximum } else { 0 }
Write-Host "WEATHER SOAK SUMMARY"
Write-Host "Log: $logPath"
Write-Host "Polls: $total; passed: $passed; failed: $failed"
Write-Host ("Response ms min/avg/max: {0:N2}/{1:N2}/{2:N2}" -f $minimum,$average,$maximum)
Write-Host "Detected reboots: $reboots"
Write-Host "Weather availability losses: $weatherLosses"
Write-Host "Observed weather refreshes: $weatherUpdates"
if ($failed -gt 0 -or $reboots -gt 0 -or $weatherLosses -gt 0) { exit 1 }
