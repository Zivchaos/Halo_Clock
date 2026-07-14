#include "WebService.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <WiFi.h>

#include "AutoNightService.h"
#include "Clock.h"
#include "Config.h"
#include "DiagnosticsService.h"
#include "Halo.h"
#include "OtaService.h"
#include "SettingsService.h"
#include "TimeService.h"
#include "Version.h"
#include "WeatherService.h"

namespace
{
    WebServer server(Config::WEB_PORT);
    bool started = false;
    bool rebootScheduled = false;
    uint32_t rebootRequestedAt = 0;

    const char INDEX_HTML[] PROGMEM = R"HALOHTML(
<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>)HALOHTML" HALO_CST_PRODUCT_NAME R"HALOHTML(</title><style>
:root{color-scheme:dark;--bg:#080d18;--panel:#121b2d;--line:#263653;--text:#f2f6ff;--muted:#9aabc7;--accent:#47b8ff;--ok:#54d69c;--bad:#ff6b7a}*{box-sizing:border-box}body{margin:0;background:radial-gradient(circle at top,#14213a,var(--bg) 45%);color:var(--text);font:16px system-ui,sans-serif}main{width:min(980px,calc(100% - 28px));margin:24px auto 48px}header{display:flex;justify-content:space-between;align-items:end;gap:16px;margin-bottom:18px}h1{margin:0;font-size:clamp(1.8rem,5vw,3rem);letter-spacing:.08em}header p{margin:4px 0;color:var(--muted)}#clock{font:700 clamp(1.8rem,7vw,3.8rem) ui-monospace,monospace;color:var(--accent)}.grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:14px}.card{background:rgba(18,27,45,.94);border:1px solid var(--line);border-radius:16px;padding:18px;box-shadow:0 12px 32px #0004}.wide{grid-column:1/-1}h2{font-size:1rem;letter-spacing:.08em;text-transform:uppercase;color:var(--muted);margin:0 0 14px}.stats{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px}.stat{padding:10px;background:#0b1322;border-radius:10px}.stat b,.stat span{display:block}.stat span{color:var(--muted);font-size:.8rem;margin-bottom:4px}.weather-now{font-size:2rem;font-weight:700;color:var(--accent);margin:0 0 4px}.weather-meta{color:var(--muted);margin:4px 0}.controls{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:14px}label{display:block;color:var(--muted);font-size:.86rem}select,input[type=time],button{width:100%;min-height:44px;margin-top:6px;border:1px solid var(--line);border-radius:10px;background:#0b1322;color:var(--text);padding:9px 11px;font:inherit}button{cursor:pointer;background:#173556;border-color:#2b71a3;font-weight:700}button.danger{background:#4a1d29;border-color:#9a3e50}button:disabled,select:disabled,input:disabled{opacity:.55;cursor:wait}.toggle{display:flex;align-items:center;gap:10px;min-height:44px}.toggle input{width:22px;height:22px}.feedback{min-height:24px;margin:14px 0 0;color:var(--ok)}.feedback.error{color:var(--bad)}dialog{width:min(420px,calc(100% - 32px));border:1px solid var(--line);border-radius:16px;background:var(--panel);color:var(--text);padding:20px}dialog::backdrop{background:#02050acc}.dialog-actions{display:grid;grid-template-columns:1fr 1fr;gap:10px}@media(max-width:680px){header{display:block}.grid,.controls,.stats{grid-template-columns:1fr}.wide{grid-column:auto}main{width:min(980px,calc(100% - 18px));margin-top:14px}.card{padding:15px}}
</style><style>details summary{cursor:pointer;font-weight:700;letter-spacing:.08em;text-transform:uppercase;color:var(--muted)}.diagnostics{margin-top:16px;padding:12px;overflow:auto;border-radius:10px;background:#08101d;color:#c9dcf8;font:13px/1.5 ui-monospace,monospace;white-space:pre-wrap}.diagnostic-actions{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;margin-top:12px}@media(max-width:680px){.diagnostic-actions{grid-template-columns:1fr}}</style></head><body><main><header><div><h1>)HALOHTML" HALO_CST_PRODUCT_NAME R"HALOHTML(</h1><p>)HALOHTML" HALO_CST_EXPANDED_NAME R"HALOHTML(</p></div><div id="clock">--:--:--</div></header>
<section class="grid"><article class="card wide"><h2>Live status</h2><div class="stats">
<div class="stat"><span>Wi-Fi</span><b id="wifi">--</b></div><div class="stat"><span>IP address</span><b id="ip">--</b></div><div class="stat"><span>Signal</span><b id="rssi">--</b></div>
<div class="stat"><span>Uptime</span><b id="uptime">--</b></div><div class="stat"><span>Firmware</span><b id="firmware">--</b></div><div class="stat"><span>OTA</span><b id="ota">--</b></div>
<div class="stat"><span>Selected mode</span><b id="selected">--</b></div><div class="stat"><span>Effective mode</span><b id="effective">--</b></div><div class="stat"><span>Brightness</span><b id="brightnessText">--</b></div>
<div class="stat"><span>Automatic NIGHT</span><b id="autoState">--</b></div><div class="stat"><span>Schedule</span><b id="schedule">--</b></div><div class="stat"><span>Manual override</span><b id="override">--</b></div>
</div></article>
<article class="card"><h2>Display</h2><div class="controls"><label>Mode<select id="mode" class="control"><option>CLASSIC</option><option>MINIMAL</option><option>NIGHT</option></select></label><label>Brightness<select id="brightness" class="control"><option>10</option><option>25</option><option>40</option><option>80</option></select></label></div></article>
<article class="card"><h2>Weather</h2><p class="weather-now" id="weatherTemperature">--</p><p class="weather-meta" id="weatherCondition">Waiting for data</p><p class="weather-meta" id="weatherDetails">Humidity -- · Wind --</p><p class="weather-meta" id="weatherAge">Not updated</p><button id="weatherRefresh" class="control" type="button">Refresh weather</button></article>
<article class="card"><h2>Automatic NIGHT</h2><form id="autoForm"><label class="toggle"><input id="autoEnabled" class="control" type="checkbox">Enabled</label><div class="controls"><label>Starts<input id="autoStart" class="control" type="time" required></label><label>Ends<input id="autoEnd" class="control" type="time" required></label></div><button class="control" type="submit">Save schedule</button></form></article>
<details class="card wide" id="diagnosticsPanel"><summary>Diagnostics</summary><pre class="diagnostics" id="diagnosticsOutput">Open or refresh to load current diagnostics.</pre><div class="diagnostic-actions"><button id="diagnosticsRefresh" type="button">Refresh</button><button id="diagnosticsCopy" type="button">Copy diagnostics</button><button id="diagnosticsDownload" type="button">Download diagnostics JSON</button></div></details>
<article class="card wide"><h2>System</h2><button id="reboot" class="danger control">Reboot )HALOHTML" HALO_CST_PRODUCT_NAME R"HALOHTML(</button><p id="feedback" class="feedback" aria-live="polite"></p></article></section>
<dialog id="rebootDialog"><h2>Confirm reboot</h2><p>The clock will be unavailable briefly while it restarts.</p><div class="dialog-actions"><button id="rebootCancel" type="button">Cancel</button><button id="rebootConfirm" class="danger control" type="button">Reboot now</button></div></dialog></main>
<script>
const $=id=>document.getElementById(id),controls=()=>document.querySelectorAll('.control');let busy=false,diagnosticsData=null;
function setBusy(value){busy=value;controls().forEach(x=>x.disabled=value)}
function feedback(message,error=false){$('feedback').textContent=message;$('feedback').className='feedback'+(error?' error':'')}
function hhmm(seconds){seconds=Number(seconds)||0;const d=Math.floor(seconds/86400),h=Math.floor(seconds%86400/3600),m=Math.floor(seconds%3600/60);return(d?d+'d ':'')+h+'h '+m+'m'}
function applyStatus(s){$('clock').textContent=s.time;$('wifi').textContent=s.wifiConnected?'Connected':'Disconnected';$('ip').textContent=s.ip;$('rssi').textContent=s.rssi+' dBm';$('uptime').textContent=hhmm(s.uptimeSeconds);$('firmware').textContent=s.firmwareVersion;$('ota').textContent=s.otaUpdating?'Updating':(s.otaReady?'Ready':'Waiting');$('selected').textContent=s.selectedMode;$('effective').textContent=s.effectiveMode;$('brightnessText').textContent=s.brightness;$('autoState').textContent=(s.autoNightEnabled?'Enabled':'Disabled')+' / '+(s.autoNightActive?'Active':'Inactive');$('schedule').textContent=s.autoNightStart+'–'+s.autoNightEnd;$('override').textContent=s.manualOverride?'Active':'None';$('weatherTemperature').textContent=s.weatherAvailable?s.temperature.toFixed(1)+' °C':'--';$('weatherCondition').textContent=s.weatherAvailable?s.condition:'WEATHER UNAVAILABLE';$('weatherDetails').textContent=s.weatherAvailable?'Humidity '+s.humidity+'% · Wind '+s.windSpeed.toFixed(1)+' km/h':(s.weatherError||'Waiting for data');const weatherState=s.weatherStale?'Stale':(s.weatherError&&s.weatherAvailable?'Cached / '+s.weatherError:(s.weatherAvailable?'Current':'Unavailable'));$('weatherAge').textContent=weatherState+(s.weatherLastUpdate?' · '+new Date(s.weatherLastUpdate*1000).toLocaleTimeString(): '');if(!busy){$('mode').value=s.selectedMode;$('brightness').value=String(s.brightness);$('autoEnabled').checked=s.autoNightEnabled;$('autoStart').value=s.autoNightStart;$('autoEnd').value=s.autoNightEnd}}
async function status(){try{const r=await fetch('/api/status',{cache:'no-store'});if(!r.ok)throw Error('Status request failed');applyStatus(await r.json())}catch(e){feedback(e.message,true)}}
async function loadDiagnostics(){try{const r=await fetch('/api/diagnostics',{cache:'no-store'});if(!r.ok)throw Error('Diagnostics request failed');diagnosticsData=await r.json();$('diagnosticsOutput').textContent=JSON.stringify(diagnosticsData,null,2)}catch(e){$('diagnosticsOutput').textContent=e.message;feedback(e.message,true)}}
async function copyDiagnostics(){if(!diagnosticsData)await loadDiagnostics();if(!diagnosticsData)return;const text=JSON.stringify(diagnosticsData,null,2);try{if(navigator.clipboard)await navigator.clipboard.writeText(text);else{const area=document.createElement('textarea');area.value=text;document.body.appendChild(area);area.select();document.execCommand('copy');area.remove()}feedback('Diagnostics copied')}catch(e){feedback('Copy failed',true)}}
async function downloadDiagnostics(){if(!diagnosticsData)await loadDiagnostics();if(!diagnosticsData)return;const url=URL.createObjectURL(new Blob([JSON.stringify(diagnosticsData,null,2)],{type:'application/json'})),link=document.createElement('a');link.href=url;link.download='halo-cst-diagnostics.json';link.click();URL.revokeObjectURL(url)}
async function post(path,data){setBusy(true);feedback('Applying…');try{const r=await fetch(path,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(data)});const result=await r.json();if(!r.ok||!result.ok)throw Error(result.error||'Request failed');feedback('Saved');await status()}catch(e){feedback(e.message,true)}finally{setBusy(false);await status()}}
$('mode').addEventListener('change',e=>post('/api/mode',{mode:e.target.value}));$('brightness').addEventListener('change',e=>post('/api/brightness',{value:e.target.value}));
$('weatherRefresh').addEventListener('click',()=>post('/api/weather/refresh',{}));
$('autoForm').addEventListener('submit',e=>{e.preventDefault();const a=$('autoStart').value.split(':'),b=$('autoEnd').value.split(':');post('/api/auto-night',{enabled:String($('autoEnabled').checked),startHour:a[0]||'',startMinute:a[1]||'',endHour:b[0]||'',endMinute:b[1]||''})});
$('reboot').addEventListener('click',()=>$('rebootDialog').showModal());$('rebootCancel').addEventListener('click',()=>$('rebootDialog').close());$('rebootConfirm').addEventListener('click',()=>{$('rebootDialog').close();post('/api/reboot',{confirm:'true'})});
$('diagnosticsPanel').addEventListener('toggle',e=>{if(e.target.open&&!diagnosticsData)loadDiagnostics()});$('diagnosticsRefresh').addEventListener('click',loadDiagnostics);$('diagnosticsCopy').addEventListener('click',copyDiagnostics);$('diagnosticsDownload').addEventListener('click',downloadDiagnostics);
status();setInterval(status,3000);
</script></body></html>)HALOHTML";

    const char* boolText(bool value)
    {
        return value ? "true" : "false";
    }

    void sendJson(int statusCode, const char* body)
    {
        server.sendHeader("Cache-Control", "no-store");
        server.send(statusCode, "application/json", body);
    }

    void sendError(int statusCode, const char* reason)
    {
        char body[160];
        snprintf(body, sizeof(body), "{\"ok\":false,\"error\":\"%s\"}", reason);
        sendJson(statusCode, body);
        Serial.printf("WEB ERROR: %s\r\n", reason);
    }

    bool parseByteArgument(const char* name, uint8_t maximum, uint8_t& result)
    {
        if (!server.hasArg(name))
        {
            return false;
        }

        const String value = server.arg(name);
        if (value.isEmpty())
        {
            return false;
        }

        uint16_t parsed = 0;
        for (size_t index = 0; index < value.length(); ++index)
        {
            const char character = value[index];
            if (character < '0' || character > '9')
            {
                return false;
            }
            parsed = static_cast<uint16_t>(parsed * 10U + static_cast<uint8_t>(character - '0'));
            if (parsed > maximum)
            {
                return false;
            }
        }
        result = static_cast<uint8_t>(parsed);
        return true;
    }

    bool changesAllowed()
    {
        if (OtaService::isUpdating())
        {
            sendError(503, "OTA update in progress");
            return false;
        }
        return true;
    }

    void handleRoot()
    {
        Serial.println("WEB REQUEST: GET /");
        server.sendHeader("Cache-Control", "no-store");
        server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
    }

    void handleStatus()
    {
        const DiagnosticsData diagnostics = DiagnosticsService::snapshot();
        char timeText[9] = "--:--:--";
        if (TimeService::isSynchronized())
        {
            strftime(timeText, sizeof(timeText), "%H:%M:%S", &TimeService::localTime());
        }

        const AutoNightSettings& autoNight = SettingsService::autoNight();
        const WeatherData weather = WeatherService::snapshot();
        char startText[8];
        char endText[8];
        snprintf(startText, sizeof(startText), "%02u:%02u", autoNight.startHour, autoNight.startMinute);
        snprintf(endText, sizeof(endText), "%02u:%02u", autoNight.endHour, autoNight.endMinute);
        const String ipAddress = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "0.0.0.0";

        char temperatureText[16] = "null";
        char apparentTemperatureText[16] = "null";
        char humidityText[8] = "null";
        char windSpeedText[16] = "null";
        if (weather.valid)
        {
            snprintf(temperatureText, sizeof(temperatureText), "%.1f", static_cast<double>(weather.temperature));
            snprintf(humidityText, sizeof(humidityText), "%u", weather.humidity);
            snprintf(windSpeedText, sizeof(windSpeedText), "%.1f", static_cast<double>(weather.windSpeed));
            if (weather.apparentTemperatureAvailable)
            {
                snprintf(
                    apparentTemperatureText,
                    sizeof(apparentTemperatureText),
                    "%.1f",
                    static_cast<double>(weather.apparentTemperature));
            }
        }

        char body[1800];
        snprintf(
            body,
            sizeof(body),
            "{\"time\":\"%s\",\"wifiConnected\":%s,\"ip\":\"%s\",\"rssi\":%ld,"
            "\"uptimeSeconds\":%lu,\"freeHeap\":%lu,\"minimumFreeHeap\":%lu,"
            "\"firmwareVersion\":\"%s\",\"resetReason\":\"%s\",\"runningPartition\":\"%s\","
            "\"wifiReconnectCount\":%lu,\"ntpSynchronized\":%s,\"timeSyncAgeValid\":%s,"
            "\"lastTimeSyncAgeSeconds\":%lu,\"weatherRequestCount\":%lu,"
            "\"weatherSuccessCount\":%lu,\"weatherFailureCount\":%lu,"
            "\"selectedMode\":\"%s\",\"effectiveMode\":\"%s\","
            "\"brightness\":%u,\"autoNightEnabled\":%s,\"autoNightActive\":%s,"
            "\"autoNightStart\":\"%s\",\"autoNightEnd\":\"%s\",\"manualOverride\":%s,"
            "\"otaReady\":%s,\"otaUpdating\":%s,"
            "\"weatherAvailable\":%s,\"weatherStale\":%s,\"temperature\":%s,"
            "\"apparentTemperature\":%s,\"condition\":\"%s\",\"humidity\":%s,"
            "\"windSpeed\":%s,\"weatherLastUpdate\":%lu,\"weatherError\":\"%s\"}",
            timeText,
            boolText(WiFi.status() == WL_CONNECTED),
            ipAddress.c_str(),
            static_cast<long>(WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0),
            static_cast<unsigned long>(diagnostics.uptimeSeconds),
            static_cast<unsigned long>(diagnostics.freeHeap),
            static_cast<unsigned long>(diagnostics.minimumFreeHeap),
            diagnostics.firmwareVersion,
            diagnostics.resetReason,
            diagnostics.runningPartition,
            static_cast<unsigned long>(diagnostics.wifiReconnectCount),
            boolText(diagnostics.ntpSynchronized),
            boolText(diagnostics.timeSyncAgeValid),
            static_cast<unsigned long>(diagnostics.lastTimeSyncAgeSeconds),
            static_cast<unsigned long>(diagnostics.weatherRequestCount),
            static_cast<unsigned long>(diagnostics.weatherSuccessCount),
            static_cast<unsigned long>(diagnostics.weatherFailureCount),
            DisplayModes::name(SettingsService::displayMode()),
            DisplayModes::name(Clock::effectiveMode()),
            SettingsService::brightness(),
            boolText(autoNight.enabled),
            boolText(AutoNightService::isActive()),
            startText,
            endText,
            boolText(AutoNightService::isManualOverride()),
            boolText(OtaService::isReady()),
            boolText(OtaService::isUpdating()),
            boolText(weather.valid),
            boolText(weather.stale),
            temperatureText,
            apparentTemperatureText,
            weather.condition,
            humidityText,
            windSpeedText,
            static_cast<unsigned long>(weather.lastSuccessfulUpdateEpoch),
            weather.error);
        sendJson(200, body);
    }

    void handleDiagnostics()
    {
        const DiagnosticsData diagnostics = DiagnosticsService::snapshot();
        JsonDocument document;

        JsonObject system = document["system"].to<JsonObject>();
        system["uptimeSeconds"] = diagnostics.uptimeSeconds;
        system["freeHeap"] = diagnostics.freeHeap;
        system["minimumFreeHeap"] = diagnostics.minimumFreeHeap;
        system["resetReason"] = diagnostics.resetReason;

        JsonObject wifi = document["wifi"].to<JsonObject>();
        wifi["connected"] = diagnostics.wifiConnected;
        wifi["reconnectCount"] = diagnostics.wifiReconnectCount;
        wifi["rssi"] = diagnostics.wifiRssi;

        JsonObject time = document["time"].to<JsonObject>();
        time["ntpSynchronized"] = diagnostics.ntpSynchronized;
        time["syncAgeValid"] = diagnostics.timeSyncAgeValid;
        time["lastSuccessfulSyncAgeSeconds"] = diagnostics.lastTimeSyncAgeSeconds;

        JsonObject weather = document["weather"].to<JsonObject>();
        weather["requestCount"] = diagnostics.weatherRequestCount;
        weather["successCount"] = diagnostics.weatherSuccessCount;
        weather["failureCount"] = diagnostics.weatherFailureCount;
        weather["lastError"] = diagnostics.lastWeatherError;

        JsonObject ota = document["ota"].to<JsonObject>();
        ota["ready"] = diagnostics.otaReady;
        ota["updating"] = diagnostics.otaUpdating;
        ota["runningPartition"] = diagnostics.runningPartition;

        JsonObject firmware = document["firmware"].to<JsonObject>();
        firmware["version"] = diagnostics.firmwareVersion;
        firmware["buildDate"] = diagnostics.buildDate;
        firmware["buildTime"] = diagnostics.buildTime;

        String body;
        body.reserve(1024);
        serializeJson(document, body);
        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "application/json", body);
    }

    void handleWeatherRefresh()
    {
        Serial.println("WEB REQUEST: POST /api/weather/refresh");
        if (!changesAllowed()) return;

        switch (WeatherService::requestRefresh())
        {
            case WeatherRefreshResult::ACCEPTED:
                sendJson(202, "{\"ok\":true}");
                return;
            case WeatherRefreshResult::RATE_LIMITED:
                sendError(429, "weather refresh rate limited");
                return;
            case WeatherRefreshResult::BUSY:
                sendError(409, "weather refresh already in progress");
                return;
            case WeatherRefreshResult::OFFLINE:
                sendError(503, "weather refresh unavailable while offline");
                return;
        }
    }

    void handleMode()
    {
        Serial.println("WEB REQUEST: POST /api/mode");
        if (!changesAllowed()) return;
        if (!server.hasArg("mode"))
        {
            sendError(400, "missing mode");
            return;
        }

        const String value = server.arg("mode");
        DisplayMode mode;
        if (value == "CLASSIC") mode = DisplayMode::CLASSIC;
        else if (value == "MINIMAL") mode = DisplayMode::MINIMAL;
        else if (value == "NIGHT") mode = DisplayMode::NIGHT;
        else
        {
            sendError(400, "invalid mode");
            return;
        }

        Halo::setDisplayMode(mode);
        Serial.printf("WEB SET MODE: %s\r\n", DisplayModes::name(mode));
        sendJson(200, "{\"ok\":true}");
    }

    void handleBrightness()
    {
        Serial.println("WEB REQUEST: POST /api/brightness");
        if (!changesAllowed()) return;
        uint8_t brightness = 0;
        if (!parseByteArgument("value", 255, brightness) || !Config::isSupportedBrightness(brightness))
        {
            sendError(400, "invalid brightness");
            return;
        }

        Halo::setBrightness(brightness);
        Serial.printf("WEB SET BRIGHTNESS: %u\r\n", brightness);
        sendJson(200, "{\"ok\":true}");
    }

    void handleAutoNight()
    {
        Serial.println("WEB REQUEST: POST /api/auto-night");
        if (!changesAllowed()) return;
        if (!server.hasArg("enabled"))
        {
            sendError(400, "missing automatic NIGHT state");
            return;
        }

        const String enabledValue = server.arg("enabled");
        if (enabledValue != "true" && enabledValue != "false")
        {
            sendError(400, "invalid automatic NIGHT state");
            return;
        }

        AutoNightSettings settings = {};
        settings.enabled = enabledValue == "true";
        if (!parseByteArgument("startHour", 23, settings.startHour) ||
            !parseByteArgument("startMinute", 59, settings.startMinute) ||
            !parseByteArgument("endHour", 23, settings.endHour) ||
            !parseByteArgument("endMinute", 59, settings.endMinute))
        {
            sendError(400, "invalid automatic NIGHT schedule");
            return;
        }

        const AutoNightSettings previous = SettingsService::autoNight();
        if (!SettingsService::saveAutoNight(settings))
        {
            sendError(500, "automatic NIGHT settings could not be saved");
            return;
        }

        const bool changed = settings.enabled != previous.enabled ||
            settings.startHour != previous.startHour || settings.startMinute != previous.startMinute ||
            settings.endHour != previous.endHour || settings.endMinute != previous.endMinute;
        if (changed)
        {
            AutoNightService::settingsChanged();
        }
        Serial.printf("WEB AUTO NIGHT: %s\r\n", settings.enabled ? "ENABLED" : "DISABLED");
        Serial.printf(
            "WEB AUTO NIGHT SCHEDULE: %02u:%02u-%02u:%02u\r\n",
            settings.startHour,
            settings.startMinute,
            settings.endHour,
            settings.endMinute);
        sendJson(200, "{\"ok\":true}");
    }

    void handleReboot()
    {
        Serial.println("WEB REQUEST: POST /api/reboot");
        if (!changesAllowed()) return;
        if (!server.hasArg("confirm") || server.arg("confirm") != "true")
        {
            sendError(400, "reboot confirmation required");
            return;
        }

        sendJson(200, "{\"ok\":true}");
        rebootRequestedAt = millis();
        rebootScheduled = true;
        Serial.println("WEB REBOOT REQUESTED");
    }

    void configureRoutes()
    {
        server.on("/", HTTP_GET, handleRoot);
        server.on("/api/status", HTTP_GET, handleStatus);
        server.on("/api/diagnostics", HTTP_GET, handleDiagnostics);
        server.on("/api/mode", HTTP_POST, handleMode);
        server.on("/api/brightness", HTTP_POST, handleBrightness);
        server.on("/api/auto-night", HTTP_POST, handleAutoNight);
        server.on("/api/weather/refresh", HTTP_POST, handleWeatherRefresh);
        server.on("/api/reboot", HTTP_POST, handleReboot);
        server.onNotFound([]() {
            Serial.printf("WEB REQUEST: %s %s\r\n", server.method() == HTTP_POST ? "POST" : "GET", server.uri().c_str());
            sendError(404, "not found");
        });
    }
}

void WebService::begin()
{
    started = false;
    rebootScheduled = false;
}

void WebService::update()
{
    if (!started && WiFi.status() == WL_CONNECTED)
    {
        configureRoutes();
        server.begin();
        started = true;
        Serial.println("WEB UI: STARTED");
        Serial.printf("WEB UI: http://%s.local/\r\n", Product::HOSTNAME);
    }

    if (started && WiFi.status() == WL_CONNECTED)
    {
        server.handleClient();
    }

    if (rebootScheduled && millis() - rebootRequestedAt >= Config::WEB_REBOOT_DELAY_MS)
    {
        rebootScheduled = false;
        ESP.restart();
    }
}
