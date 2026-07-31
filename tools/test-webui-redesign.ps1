param(
    [string]$BaseUrl = "",
    [switch]$SourceOnly
)

$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$script:Passed = 0
$script:Failed = 0

function Assert-Ui([bool]$Condition, [string]$Name) {
    if ($Condition) {
        $script:Passed++
        Write-Host "PASS: $Name" -ForegroundColor Green
    } else {
        $script:Failed++
        Write-Host "FAIL: $Name" -ForegroundColor Red
    }
}

function Read-Ui([string]$Path) {
    Get-Content -LiteralPath (Join-Path $root $Path) -Raw
}

$html = Read-Ui "webui/index.html"
$css = Read-Ui "webui/styles.css"
$js = Read-Ui "webui/app.js"
$generated = Read-Ui "src/WebUiAssets.h"

Assert-Ui ($html -match '<meta name="viewport"') "responsive viewport metadata exists"
Assert-Ui ($html -match 'rel="icon"[^>]+/favicon\.svg') "favicon is referenced"
Assert-Ui ($html -match 'data-testid="octagon-clock"') "octagonal clock component exists"
Assert-Ui (($html -match '>12<') -and ($html -match '>3<') -and ($html -match '>6<') -and ($html -match '>9<')) "cardinal clock labels exist"
foreach ($section in @("Overview", "Time &amp; Display", "Network", "Weather", "Diagnostics", "Settings")) {
    Assert-Ui ($html -match [regex]::Escape($section)) "section is present: $section"
}
Assert-Ui ($html -match 'Quick controls') "quick controls exist"
Assert-Ui (($html -match 'Selected mode') -and ($html -match 'Effective mode')) "selected and effective mode labels exist"
Assert-Ui ($html -match 'Automatic NIGHT') "automatic NIGHT controls exist"
Assert-Ui (($html -match 'Configured button') -and ($html -match 'USB recovery')) "network recovery guidance exists"
Assert-Ui (($html -match 'Hardware setup') -and ($html -match 'hardwareForm')) "hardware setup controls exist"
Assert-Ui (($html -match 'hardwareLedData') -and ($html -match 'hardwareOledSda') -and ($html -match 'hardwareOledScl') -and ($html -match 'hardwareButton')) "hardware pin selectors exist"
Assert-Ui (($html -match 'ota-fields') -and ($html -match 'otaPasswordConfirm')) "OTA password confirmation has a dedicated layout"
Assert-Ui (($html -match 'Copy diagnostics') -and ($html -match 'Download diagnostics JSON')) "diagnostics actions exist"
Assert-Ui (($html -match '<option>CUSTOM</option>') -and ($html -match 'customColorForm')) "CUSTOM color controls exist"
Assert-Ui ($html -match 'calibrationForm') "ring calibration controls exist"
Assert-Ui (($html -match 'weatherLocationForm') -and ($html -match 'open-meteo\.com')) "weather location and provider link exist"
Assert-Ui (($html -match '<label') -and ($html -notmatch '<button[^>]*>\s*</button>')) "forms and buttons have visible labels"

Assert-Ui ($css -match ':focus-visible') "visible focus styles exist"
Assert-Ui ($css -match 'prefers-reduced-motion') "reduced-motion preference is respected"
Assert-Ui ($css -match '@media\s*\(max-width:\s*520px\)') "phone breakpoint exists"
Assert-Ui ($css -match 'overflow-x:\s*hidden') "document horizontal overflow is constrained"
Assert-Ui ($css -match 'clip-path:\s*polygon') "octagonal CSS geometry exists"
Assert-Ui (($css -match 'backdrop-filter') -and ($css -match '--glass')) "glass visual treatment exists"

Assert-Ui ($js -match 'POLL_INTERVAL_MS\s*=\s*3000') "status polling interval is three seconds"
Assert-Ui (([regex]::Matches($js, 'setInterval\s*\(').Count) -eq 1) "only one repeating interval exists"
Assert-Ui ($js -match 'api\("/api/status"\)') "regular polling uses the status endpoint"
Assert-Ui ($js -match 'statusInFlight') "overlapping status polls are suppressed"
Assert-Ui ($js -match 'diagnosticsPanel.+toggle') "diagnostics load is tied to panel demand"
Assert-Ui ($js -match 'api\("/api/diagnostics"\)') "diagnostics endpoint is used"
Assert-Ui ($js -match 'api\("/api/network"\)') "network details are fetched on demand"
Assert-Ui ($js -match 'api\("/api/hardware"\)') "hardware details are fetched on demand"
Assert-Ui ($js -match 'validateHardwareSelection') "hardware pin conflicts are checked in the interface"
Assert-Ui ($js -match 'api\("/api/customization"\)') "customization settings load once outside status polling"
Assert-Ui ($js -match 'catch\s*\(') "request errors are handled"
Assert-Ui ($js -notmatch 'https?://[^"'']+\.(js|css|woff2?)') "no external UI library or font requests exist"
Assert-Ui ($js -notmatch '\balert\s*\(') "no blocking alert is used"

Assert-Ui ($generated -match 'GENERATED FILE.+DO NOT EDIT') "embedded asset header is marked generated"
Assert-Ui ($generated -match 'PROGMEM') "embedded page is flash resident"
Assert-Ui ($generated -match 'HALO CST') "generated page contains current branding"
Assert-Ui ($generated -notmatch '\{\{[^}]+\}\}') "generated page has no unresolved template tokens"

if (-not $SourceOnly -and -not [string]::IsNullOrWhiteSpace($BaseUrl)) {
    $base = $BaseUrl.TrimEnd('/')
    $response = Invoke-WebRequest -Uri "$base/" -UseBasicParsing -TimeoutSec 10
    Assert-Ui ($response.StatusCode -eq 200) "live root returns HTTP 200"
    Assert-Ui ($response.Content -match 'data-testid="octagon-clock"') "live root serves redesigned clock"
    Assert-Ui ($response.Content -match '<meta name="viewport"') "live root is responsive"
    $favicon = Invoke-WebRequest -Uri "$base/favicon.svg" -UseBasicParsing -TimeoutSec 10
    Assert-Ui ($favicon.StatusCode -eq 200) "live favicon returns HTTP 200"
    Assert-Ui ([string]$favicon.Headers.'Content-Type' -match 'image/svg\+xml') "live favicon has SVG content type"
}

Write-Host "WEB UI REDESIGN TEST SUMMARY: $($script:Passed) passed, $($script:Failed) failed"
if ($script:Failed -gt 0) { exit 1 }
