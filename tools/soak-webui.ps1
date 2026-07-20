param(
    [string]$BaseUrl = "http://halo-cst.local",
    [int]$Hours = 8,
    [int]$IntervalSeconds = 60
)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Net.Http
$client = [System.Net.Http.HttpClient]::new()
$client.Timeout = [TimeSpan]::FromSeconds(10)
$logDirectory = Join-Path (Split-Path $PSScriptRoot -Parent) "logs"
New-Item -ItemType Directory -Force -Path $logDirectory | Out-Null
$logPath = Join-Path $logDirectory ("webui-soak-{0}.csv" -f (Get-Date -Format "yyyyMMdd-HHmmss"))
$deadline = (Get-Date).AddHours($Hours)
$nextPageCheck = Get-Date
$total = 0; $successful = 0; $httpFailures = 0; $jsonFailures = 0
$reboots = 0; $wifiDisconnects = 0; $slowResponses = 0; $consecutiveFailures = 0; $maximumConsecutiveFailures = 0
$pageChecks = 0; $pageFailures = 0
$lastUptime = $null; $finalStatus = $null
$responseTimes = [System.Collections.Generic.List[double]]::new()
$validModes = @("CLASSIC","MINIMAL","NIGHT","CUSTOM")
$validBrightness = @(10,25,40,80)

function Write-SoakRow([object]$Row) {
    $Row | Export-Csv -LiteralPath $logPath -Append -NoTypeInformation
}

while ((Get-Date) -lt $deadline) {
    $started = Get-Date
    $total++
    $success = $false; $errorText = ""; $status = $null
    try {
        $json = $client.GetStringAsync("$($BaseUrl.TrimEnd('/'))/api/status").GetAwaiter().GetResult()
        try { $status = $json | ConvertFrom-Json } catch { $jsonFailures++; throw "Invalid JSON: $($_.Exception.Message)" }
        $elapsed = ((Get-Date) - $started).TotalMilliseconds
        $responseTimes.Add($elapsed)
        if ($elapsed -gt 2000) { $slowResponses++ }
        if (-not ($validBrightness -contains [int]$status.brightness)) { throw "Invalid brightness $($status.brightness)" }
        if (-not ($validModes -contains [string]$status.selectedMode) -or -not ($validModes -contains [string]$status.effectiveMode)) { throw "Invalid display mode" }
        if ($null -ne $lastUptime -and [long]$status.uptimeSeconds -lt $lastUptime) { $reboots++ }
        if (-not [bool]$status.wifiConnected) { $wifiDisconnects++ }
        $lastUptime = [long]$status.uptimeSeconds
        $finalStatus = $status
        $successful++
        $consecutiveFailures = 0
        $success = $true
    } catch {
        $httpFailures++
        $consecutiveFailures++
        if ($consecutiveFailures -gt $maximumConsecutiveFailures) { $maximumConsecutiveFailures = $consecutiveFailures }
        $errorText = $_.Exception.Message
        $elapsed = ((Get-Date) - $started).TotalMilliseconds
    }

    Write-SoakRow ([pscustomobject]@{
        timestamp = (Get-Date).ToString("o"); requestSuccess = $success; responseMilliseconds = [math]::Round($elapsed,2)
        uptimeSeconds = if ($status) {$status.uptimeSeconds} else {$null}; freeHeap = if ($status) {$status.freeHeap} else {$null}
        wifiConnected = if ($status) {$status.wifiConnected} else {$null}; rssi = if ($status) {$status.rssi} else {$null}
        selectedMode = if ($status) {$status.selectedMode} else {$null}; effectiveMode = if ($status) {$status.effectiveMode} else {$null}
        brightness = if ($status) {$status.brightness} else {$null}; autoNightEnabled = if ($status) {$status.autoNightEnabled} else {$null}
        autoNightActive = if ($status) {$status.autoNightActive} else {$null}; otaReady = if ($status) {$status.otaReady} else {$null}
        otaUpdating = if ($status) {$status.otaUpdating} else {$null}; error = $errorText
        weatherAvailable = if ($status) {$status.weatherAvailable} else {$null}; weatherStale = if ($status) {$status.weatherStale} else {$null}
        temperature = if ($status) {$status.temperature} else {$null}; weatherLastUpdate = if ($status) {$status.weatherLastUpdate} else {$null}
        weatherError = if ($status) {$status.weatherError} else {$null}
    })

    if ((Get-Date) -ge $nextPageCheck) {
        $pageChecks++
        try {
            $page = $client.GetStringAsync("$($BaseUrl.TrimEnd('/'))/").GetAwaiter().GetResult()
            if ($page -notmatch "HALO CST") { throw "Main page identification missing" }
        } catch {
            $pageFailures++
            Write-Warning "Main page check failed: $($_.Exception.Message)"
        }
        $nextPageCheck = (Get-Date).AddMinutes(30)
    }

    $remaining = ($deadline - (Get-Date)).TotalSeconds
    if ($remaining -gt 0) { Start-Sleep -Seconds ([math]::Min($IntervalSeconds, [math]::Ceiling($remaining))) }
}

$client.Dispose()
$successPercent = if ($total) { 100.0 * $successful / $total } else { 0 }
$minimum = if ($responseTimes.Count) { ($responseTimes | Measure-Object -Minimum).Minimum } else { 0 }
$average = if ($responseTimes.Count) { ($responseTimes | Measure-Object -Average).Average } else { 0 }
$maximum = if ($responseTimes.Count) { ($responseTimes | Measure-Object -Maximum).Maximum } else { 0 }
Write-Host "SOAK TEST SUMMARY"
Write-Host "Log: $logPath"
Write-Host "Total requests: $total"
Write-Host "Successful requests: $successful"
Write-Host "Failed requests: $($total - $successful)"
Write-Host ("Success percentage: {0:N2}%" -f $successPercent)
Write-Host ("Response ms min/avg/max: {0:N2}/{1:N2}/{2:N2}" -f $minimum,$average,$maximum)
Write-Host "Detected reboots: $reboots"
Write-Host "Wi-Fi disconnect observations: $wifiDisconnects"
Write-Host "JSON failures: $jsonFailures"
Write-Host "Main page checks/failures: $pageChecks/$pageFailures"
Write-Host "Slow responses: $slowResponses"
Write-Host "Maximum consecutive failures: $maximumConsecutiveFailures"
Write-Host "Final uptime: $(if($finalStatus){$finalStatus.uptimeSeconds}else{'unknown'})"
Write-Host "Final state: $(if($finalStatus){$finalStatus | ConvertTo-Json -Compress}else{'unavailable'})"

if ($successPercent -lt 99 -or $reboots -gt 0 -or $jsonFailures -gt 0 -or $pageFailures -gt 0 -or $maximumConsecutiveFailures -gt 5) { exit 1 }
