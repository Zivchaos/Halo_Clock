param()

$ErrorActionPreference = "Stop"
$root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$script:Passed = 0
$script:Failed = 0

function Assert-Release([bool]$Condition, [string]$Name) {
    if ($Condition) {
        $script:Passed++
        Write-Host "PASS: $Name" -ForegroundColor Green
    } else {
        $script:Failed++
        Write-Host "FAIL: $Name" -ForegroundColor Red
    }
}

function Read-Repo([string]$Path) {
    Get-Content -LiteralPath (Join-Path $root $Path) -Raw
}

$version = Read-Repo "include/Version.h"
$config = Read-Repo "include/Config.h"
$hardware = Read-Repo "include/Hardware.h"
$timeService = Read-Repo "src/TimeService.cpp"
$settingsService = Read-Repo "src/SettingsService.cpp"
$webService = Read-Repo "src/WebService.cpp"
$webAssets = Read-Repo "src/WebUiAssets.h"
$readme = Read-Repo "README.md"
$platformio = Read-Repo "platformIO.ini"
$partitions = Read-Repo "partitions_halo_4mb.csv"

Assert-Release ($version -match '#define HALO_CST_PRODUCT_NAME "HALO CST"') "product name is centralized"
Assert-Release ($version -match '#define HALO_CST_EXPANDED_NAME "Connected Smart Timepiece"') "expanded name is centralized"
Assert-Release ($version -match '#define HALO_CST_FIRMWARE_VERSION "1\.0\.0-rc1"') "firmware version is 1.0.0-rc1"
Assert-Release ($version -match '#define HALO_CST_HOSTNAME "halo-cst"') "hostname is halo-cst"
Assert-Release ($version -match '#define HALO_CST_SETUP_AP_NAME "HALO-CST-Setup"') "setup AP name is centralized"
Assert-Release ($config -match 'SETTINGS_NAMESPACE = "halo-clock"') "legacy Preferences namespace is preserved"
Assert-Release ($timeService -match 'WiFi\.setHostname\(Product::HOSTNAME\)') "station hostname uses centralized metadata"
Assert-Release ($timeService -match 'autoConnect\(Product::SETUP_AP_NAME\)') "fresh setup uses centralized open AP name"
Assert-Release ($timeService -notmatch 'autoConnect\(Product::SETUP_AP_NAME\s*,') "no setup AP password is embedded"
Assert-Release (($webService -match 'WebUiAssets::INDEX_HTML') -and ($webAssets -match 'HALO CST')) "Web UI uses generated HALO CST branding"
Assert-Release ($webAssets -match 'firmwareVersion') "Web UI displays firmware version"
Assert-Release ($hardware -match 'LED_PIN = 18') "LED data remains GPIO18"
Assert-Release ($hardware -match 'BUTTON_PIN = 27') "button remains GPIO27"
Assert-Release ($hardware -match 'OLED_SDA = 21') "OLED SDA remains GPIO21"
Assert-Release ($hardware -match 'OLED_SCL = 22') "OLED SCL remains GPIO22"
Assert-Release ($settingsService -match 'LED_BRIGHTNESS') "fresh settings have a defined brightness default"
Assert-Release ($settingsService -match 'DisplayMode::CLASSIC') "fresh settings default to CLASSIC"
Assert-Release ($config -match 'AUTO_NIGHT_ENABLED = true') "fresh automatic NIGHT default is enabled"
Assert-Release ($config -match 'AUTO_NIGHT_START_HOUR = 20') "fresh NIGHT start defaults to 20:00"
Assert-Release ($config -match 'AUTO_NIGHT_END_HOUR = 6') "fresh NIGHT end defaults to 06:00"
Assert-Release ($config -match 'WEATHER_LATITUDE = 51\.4769F') "weather uses documented public example latitude"
Assert-Release ($config -match 'WEATHER_LONGITUDE = 0\.0005F') "weather uses documented public example longitude"
Assert-Release ($platformio -match 'board_build\.partitions = partitions_halo_4mb\.csv') "normal build uses the custom partition table"
Assert-Release ($platformio -match '\[env:esp32dev_ota\][\s\S]*extends = env:esp32dev') "OTA build inherits the same partition table"
Assert-Release (($partitions -match 'app0.+0x1F0000') -and ($partitions -match 'app1.+0x1F0000')) "OTA application slots are equal"
Assert-Release ($partitions -notmatch 'spiffs') "partition table has no SPIFFS"
Assert-Release ($readme -match '^# HALO CST') "README uses repository-facing project name"
Assert-Release ($readme -match 'Connected Smart Timepiece') "README explains the expanded product name"
Assert-Release ($readme -match 'firmware-1\.0\.0--rc1') "README displays the release-candidate version"
Assert-Release ($readme -match 'halo-cst\.local') "README documents the new local hostname"
Assert-Release ($readme -match 'HALO-CST-Setup') "README documents fresh-device setup AP"
Assert-Release ($readme -match 'Preferences namespace `halo-clock`') "README documents settings compatibility"
Assert-Release ($readme -match 'docs/images/webui/overview-desktop\.png') "README includes the approved WebUI overview screenshot"

$requiredFiles = @(
    "LICENSE", "CONTRIBUTING.md", "SECURITY.md", "CODE_OF_CONDUCT.md",
    "CHANGELOG.md", "RELEASE_CHECKLIST.md", ".github/pull_request_template.md",
    ".github/ISSUE_TEMPLATE/bug_report.yml", ".github/ISSUE_TEMPLATE/feature_request.yml",
    ".github/ISSUE_TEMPLATE/hardware_problem.yml"
)
foreach ($path in $requiredFiles) {
    Assert-Release (Test-Path -LiteralPath (Join-Path $root $path)) "public release file exists: $path"
}

$trackedText = Get-ChildItem -LiteralPath $root -Recurse -File | Where-Object {
    $_.FullName -notmatch '[\\/]\.git[\\/]' -and
    $_.FullName -notmatch '[\\/]\.pio[\\/]' -and
    $_.FullName -notmatch '[\\/]logs[\\/]' -and
    $_.Extension -in @('.h','.cpp','.ini','.md','.ps1','.yml','.yaml','.csv','.txt')
}
$publicText = ($trackedText | ForEach-Object { Get-Content -LiteralPath $_.FullName -Raw }) -join "`n"
Assert-Release ($publicText -notmatch '32\.0853|34\.7818') "personal production coordinates are absent"
Assert-Release ($publicText -notmatch '[A-Za-z]:\\Users\\|[A-Za-z]:\\Repos\\') "machine-specific Windows paths are absent"
Assert-Release ($publicText -notmatch '(?i)(api[_-]?key|token|password)\s*[=:]\s*["''][^"'']+["'']') "no obvious embedded credentials or API keys"

Write-Host "PUBLIC RELEASE TEST SUMMARY: $($script:Passed) passed, $($script:Failed) failed"
if ($script:Failed -gt 0) { exit 1 }
