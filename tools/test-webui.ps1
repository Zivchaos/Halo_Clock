param(
    [string]$BaseUrl = "http://halo-clock.local"
)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Net.Http
$client = [System.Net.Http.HttpClient]::new()
$client.Timeout = [TimeSpan]::FromSeconds(10)
$script:Passed = 0
$script:Failed = 0

function Invoke-HaloRequest {
    param(
        [string]$Method,
        [string]$Path,
        [hashtable]$Form,
        [string]$RawBody,
        [string]$ContentType
    )

    $request = [System.Net.Http.HttpRequestMessage]::new(
        [System.Net.Http.HttpMethod]::new($Method),
        "$($BaseUrl.TrimEnd('/'))$Path")
    if ($null -ne $Form) {
        $pairs = [System.Collections.Generic.List[System.Collections.Generic.KeyValuePair[string,string]]]::new()
        foreach ($entry in $Form.GetEnumerator()) {
            $pairs.Add([System.Collections.Generic.KeyValuePair[string,string]]::new([string]$entry.Key, [string]$entry.Value))
        }
        $request.Content = [System.Net.Http.FormUrlEncodedContent]::new($pairs)
    } elseif (-not [string]::IsNullOrWhiteSpace($ContentType)) {
        $request.Content = [System.Net.Http.StringContent]::new($RawBody, [Text.Encoding]::UTF8, $ContentType)
    }

    $response = $client.SendAsync($request).GetAwaiter().GetResult()
    $body = $response.Content.ReadAsStringAsync().GetAwaiter().GetResult()
    [pscustomobject]@{
        Status = [int]$response.StatusCode
        Body = $body
        ContentType = [string]$response.Content.Headers.ContentType
    }
}

function Assert-Halo {
    param([bool]$Condition, [string]$Name)
    if ($Condition) {
        $script:Passed++
        Write-Host "PASS: $Name" -ForegroundColor Green
    } else {
        $script:Failed++
        Write-Host "FAIL: $Name" -ForegroundColor Red
    }
}

function Get-Status {
    $response = Invoke-HaloRequest -Method GET -Path "/api/status"
    if ($response.Status -ne 200) { throw "Status endpoint returned HTTP $($response.Status)" }
    $response.Body | ConvertFrom-Json
}

function Post-Form {
    param([string]$Path, [hashtable]$Form)
    Invoke-HaloRequest -Method POST -Path $Path -Form $Form
}

function Test-Mode {
    param([string]$Mode)
    $response = Post-Form "/api/mode" @{ mode = $Mode }
    Assert-Halo ($response.Status -eq 200) "set mode $Mode returns HTTP 200"
    Assert-Halo ((Get-Status).selectedMode -eq $Mode) "status confirms mode $Mode"
}

function Test-Brightness {
    param([int]$Value)
    $response = Post-Form "/api/brightness" @{ value = [string]$Value }
    Assert-Halo ($response.Status -eq 200) "set brightness $Value returns HTTP 200"
    Assert-Halo ([int](Get-Status).brightness -eq $Value) "status confirms brightness $Value"
}

try {
    $root = Invoke-HaloRequest -Method GET -Path "/"
    Assert-Halo ($root.Status -eq 200) "GET / returns HTTP 200"
    Assert-Halo ($root.ContentType -match "text/html") "GET / returns HTML"
    Assert-Halo ($root.Body -match "HALO Clock") "main page identifies HALO Clock"
    Assert-Halo ($root.Body -match 'id="diagnosticsPanel"') "main page includes diagnostics panel"
    Assert-Halo ($root.Body -match "Copy diagnostics") "main page includes diagnostics copy control"
    Assert-Halo ($root.Body -match "Download diagnostics JSON") "main page includes diagnostics download control"

    $statusResponse = Invoke-HaloRequest -Method GET -Path "/api/status"
    Assert-Halo ($statusResponse.Status -eq 200) "GET /api/status returns HTTP 200"
    $status = $null
    try { $status = $statusResponse.Body | ConvertFrom-Json; Assert-Halo $true "status contains valid JSON" }
    catch { Assert-Halo $false "status contains valid JSON" }
    $expected = @("time","wifiConnected","ip","rssi","uptimeSeconds","firmwareVersion","resetReason","runningPartition","wifiReconnectCount","ntpSynchronized","timeSyncAgeValid","lastTimeSyncAgeSeconds","weatherRequestCount","weatherSuccessCount","weatherFailureCount","selectedMode","effectiveMode","brightness","autoNightEnabled","autoNightActive","autoNightStart","autoNightEnd","manualOverride","otaReady","otaUpdating","freeHeap","minimumFreeHeap","weatherAvailable","weatherStale","temperature","apparentTemperature","condition","humidity","windSpeed","weatherLastUpdate","weatherError")
    foreach ($field in $expected) { Assert-Halo ($status.PSObject.Properties.Name -contains $field) "status field $field exists" }
    Assert-Halo (@(10,25,40,80) -contains [int]$status.brightness) "status brightness is supported"
    Assert-Halo (@("CLASSIC","MINIMAL","NIGHT") -contains [string]$status.selectedMode) "selected mode is valid"
    Assert-Halo (@("CLASSIC","MINIMAL","NIGHT") -contains [string]$status.effectiveMode) "effective mode is valid"
    Assert-Halo ($status.uptimeSeconds -is [ValueType]) "uptime is numeric"
    Assert-Halo ($status.wifiConnected -is [bool]) "Wi-Fi state is boolean"
    Assert-Halo ($status.weatherAvailable -is [bool]) "weather availability is boolean"
    Assert-Halo ($status.weatherStale -is [bool]) "weather stale state is boolean"

    $diagnosticsResponse = Invoke-HaloRequest -Method GET -Path "/api/diagnostics"
    Assert-Halo ($diagnosticsResponse.Status -eq 200) "GET /api/diagnostics returns HTTP 200"
    Assert-Halo ($diagnosticsResponse.ContentType -match "application/json") "diagnostics returns JSON content type"
    $diagnostics = $null
    try { $diagnostics = $diagnosticsResponse.Body | ConvertFrom-Json; Assert-Halo $true "diagnostics contains valid JSON" }
    catch { Assert-Halo $false "diagnostics contains valid JSON" }
    foreach ($section in @("system","wifi","time","weather","ota","firmware")) {
        Assert-Halo ($diagnostics.PSObject.Properties.Name -contains $section) "diagnostics section $section exists"
    }
    Assert-Halo ($diagnostics.system.uptimeSeconds -is [ValueType]) "diagnostics uptime is numeric"
    Assert-Halo ($diagnostics.system.freeHeap -is [ValueType]) "diagnostics free heap is numeric"
    Assert-Halo ($diagnostics.system.minimumFreeHeap -is [ValueType]) "diagnostics minimum heap is numeric"
    Assert-Halo (-not [string]::IsNullOrWhiteSpace([string]$diagnostics.system.resetReason)) "diagnostics reset reason exists"
    Assert-Halo ($diagnostics.wifi.connected -is [bool]) "diagnostics Wi-Fi state is boolean"
    Assert-Halo ($diagnostics.wifi.reconnectCount -is [ValueType]) "diagnostics reconnect count is numeric"
    Assert-Halo ($diagnostics.wifi.rssi -is [ValueType]) "diagnostics RSSI is numeric"
    Assert-Halo ($diagnostics.time.ntpSynchronized -is [bool]) "diagnostics NTP state is boolean"
    Assert-Halo ($diagnostics.time.syncAgeValid -is [bool]) "diagnostics time-sync age validity is boolean"
    Assert-Halo ($diagnostics.time.lastSuccessfulSyncAgeSeconds -is [ValueType]) "diagnostics time-sync age is numeric"
    Assert-Halo ($diagnostics.weather.requestCount -is [ValueType]) "diagnostics weather request count is numeric"
    Assert-Halo ($diagnostics.weather.successCount -is [ValueType]) "diagnostics weather success count is numeric"
    Assert-Halo ($diagnostics.weather.failureCount -is [ValueType]) "diagnostics weather failure count is numeric"
    Assert-Halo ($diagnostics.weather.lastError -is [string]) "diagnostics last weather error is text"
    Assert-Halo ($diagnostics.ota.ready -is [bool]) "diagnostics OTA ready state is boolean"
    Assert-Halo ($diagnostics.ota.updating -is [bool]) "diagnostics OTA updating state is boolean"
    Assert-Halo (-not [string]::IsNullOrWhiteSpace([string]$diagnostics.ota.runningPartition)) "diagnostics running partition exists"
    Assert-Halo (-not [string]::IsNullOrWhiteSpace([string]$diagnostics.firmware.version)) "diagnostics firmware version exists"
    Assert-Halo (-not [string]::IsNullOrWhiteSpace([string]$diagnostics.firmware.buildDate)) "diagnostics build date exists"
    Assert-Halo (-not [string]::IsNullOrWhiteSpace([string]$diagnostics.firmware.buildTime)) "diagnostics build time exists"
    Assert-Halo ([bool]$diagnostics.ota.ready -eq [bool]$status.otaReady) "diagnostics OTA ready matches status"
    Assert-Halo ([bool]$diagnostics.ota.updating -eq [bool]$status.otaUpdating) "diagnostics OTA updating matches status"
    if ([bool]$status.weatherAvailable) {
        Assert-Halo ($null -ne $status.temperature) "available weather includes temperature"
        Assert-Halo (-not [string]::IsNullOrWhiteSpace([string]$status.condition)) "available weather includes condition"
        Assert-Halo (([int]$status.humidity -ge 0) -and ([int]$status.humidity -le 100)) "available weather humidity is valid"
        Assert-Halo ([double]$status.windSpeed -ge 0) "available weather wind speed is valid"
        Assert-Halo ([long]$status.weatherLastUpdate -gt 0) "available weather has update time"
    }

    $weatherRefresh = Post-Form "/api/weather/refresh" @{}
    Assert-Halo (@(202,429) -contains $weatherRefresh.Status) "weather refresh is accepted or rate limited"
    if ($weatherRefresh.Status -eq 202) {
        $weatherRateLimit = Post-Form "/api/weather/refresh" @{}
        Assert-Halo ($weatherRateLimit.Status -eq 429) "repeated weather refresh is rate limited"
    }

    Test-Mode "CLASSIC"
    Test-Mode "MINIMAL"
    Test-Mode "NIGHT"
    $beforeInvalidMode = (Get-Status).selectedMode
    $invalidMode = Post-Form "/api/mode" @{ mode = "UNKNOWN" }
    Assert-Halo ($invalidMode.Status -eq 400) "invalid mode returns HTTP 400"
    Assert-Halo ((Get-Status).selectedMode -eq $beforeInvalidMode) "invalid mode preserves selected mode"

    foreach ($brightness in @(10,25,40,80)) { Test-Brightness $brightness }
    foreach ($invalid in @("0","11","255","text")) {
        $before = [int](Get-Status).brightness
        $response = Post-Form "/api/brightness" @{ value = $invalid }
        Assert-Halo ($response.Status -eq 400) "invalid brightness '$invalid' returns HTTP 400"
        Assert-Halo ([int](Get-Status).brightness -eq $before) "invalid brightness '$invalid' preserves value"
    }
    $beforeMissingBrightness = [int](Get-Status).brightness
    $missingBrightness = Post-Form "/api/brightness" @{}
    Assert-Halo ($missingBrightness.Status -eq 400) "missing brightness returns HTTP 400"
    Assert-Halo ([int](Get-Status).brightness -eq $beforeMissingBrightness) "missing brightness preserves value"

    $disabled = Post-Form "/api/auto-night" @{ enabled="false"; startHour="20"; startMinute="0"; endHour="6"; endMinute="0" }
    Assert-Halo ($disabled.Status -eq 200) "automatic NIGHT can be disabled"
    Assert-Halo (-not [bool](Get-Status).autoNightEnabled) "status confirms automatic NIGHT disabled"
    $enabled = Post-Form "/api/auto-night" @{ enabled="true"; startHour="21"; startMinute="30"; endHour="6"; endMinute="45" }
    Assert-Halo ($enabled.Status -eq 200) "cross-midnight automatic NIGHT schedule is accepted"
    $scheduled = Get-Status
    Assert-Halo ([bool]$scheduled.autoNightEnabled) "status confirms automatic NIGHT enabled"
    Assert-Halo ($scheduled.autoNightStart -eq "21:30") "status confirms automatic NIGHT start"
    Assert-Halo ($scheduled.autoNightEnd -eq "06:45") "status confirms automatic NIGHT end"

    $invalidSchedules = @(
        @{ Name="start hour -1"; Form=@{enabled="true";startHour="-1";startMinute="0";endHour="6";endMinute="0"} },
        @{ Name="start hour 24"; Form=@{enabled="true";startHour="24";startMinute="0";endHour="6";endMinute="0"} },
        @{ Name="start minute -1"; Form=@{enabled="true";startHour="20";startMinute="-1";endHour="6";endMinute="0"} },
        @{ Name="start minute 60"; Form=@{enabled="true";startHour="20";startMinute="60";endHour="6";endMinute="0"} },
        @{ Name="missing fields"; Form=@{enabled="true";startHour="20"} }
    )
    foreach ($case in $invalidSchedules) {
        $before = Get-Status
        $response = Post-Form "/api/auto-night" $case.Form
        Assert-Halo ($response.Status -eq 400) "$($case.Name) returns HTTP 400"
        $after = Get-Status
        Assert-Halo (($after.autoNightStart -eq $before.autoNightStart) -and ($after.autoNightEnd -eq $before.autoNightEnd) -and ($after.autoNightEnabled -eq $before.autoNightEnabled)) "$($case.Name) preserves schedule"
    }
    $beforeMalformed = Get-Status
    $malformed = Invoke-HaloRequest -Method POST -Path "/api/auto-night" -RawBody '{"enabled":' -ContentType "application/json"
    Assert-Halo ($malformed.Status -eq 400) "malformed JSON returns HTTP 400"
    $afterMalformed = Get-Status
    Assert-Halo (($afterMalformed.autoNightStart -eq $beforeMalformed.autoNightStart) -and ($afterMalformed.autoNightEnd -eq $beforeMalformed.autoNightEnd)) "malformed JSON preserves schedule"

    $unknown = Invoke-HaloRequest -Method GET -Path "/api/does-not-exist"
    Assert-Halo ($unknown.Status -eq 404) "unknown route returns HTTP 404"
    $unconfirmedReboot = Post-Form "/api/reboot" @{}
    Assert-Halo ($unconfirmedReboot.Status -eq 400) "unconfirmed reboot returns HTTP 400"
    Start-Sleep -Milliseconds 750
    Assert-Halo ((Invoke-HaloRequest -Method GET -Path "/api/status").Status -eq 200) "unconfirmed reboot does not restart device"
}
finally {
    Write-Host "Restoring safe production state..."
    try { [void](Post-Form "/api/mode" @{mode="CLASSIC"}) } catch {}
    try { [void](Post-Form "/api/brightness" @{value="40"}) } catch {}
    try { [void](Post-Form "/api/auto-night" @{enabled="true";startHour="20";startMinute="0";endHour="6";endMinute="0"}) } catch {}
    $client.Dispose()
}

Write-Host "API TEST SUMMARY: $($script:Passed) passed, $($script:Failed) failed"
if ($script:Failed -gt 0) { exit 1 }
