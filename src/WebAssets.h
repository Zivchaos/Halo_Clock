#pragma once

#include <Arduino.h>

namespace WebAssets
{
    const char LOGO_SVG[] PROGMEM = R"SVG(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 256 256"><defs><linearGradient id="g" x1="0" y1="0" x2="1" y2="1"><stop stop-color="#70e1ff"/><stop offset=".48" stop-color="#8c62ff"/><stop offset="1" stop-color="#ff4f9a"/></linearGradient></defs><circle cx="128" cy="128" r="92" fill="none" stroke="url(#g)" stroke-width="18" stroke-linecap="round" stroke-dasharray="480 98" transform="rotate(-58 128 128)"/><circle cx="128" cy="128" r="55" fill="#111426"/><circle cx="128" cy="128" r="7" fill="#f7f8ff"/><path d="M128 128V84M128 128l33 20" stroke="#f7f8ff" stroke-width="8" stroke-linecap="round"/></svg>)SVG";

    const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="theme-color" content="#0b0d18"><title>HALO CST</title>
<style>
:root{color-scheme:dark;--bg:#090b14;--panel:#121522;--line:#252a3d;--text:#f5f7ff;--muted:#939bb7;--cyan:#63dcff;--violet:#8a67ff;--pink:#ff5598;--good:#55e6a5;--warn:#ffcc66}*{box-sizing:border-box}body{margin:0;background:radial-gradient(circle at 50% -10%,#20264a 0,#0b0d18 42%,#070912 100%);font:15px/1.45 system-ui,-apple-system,Segoe UI,sans-serif;color:var(--text);min-height:100vh}.shell{width:min(1080px,calc(100% - 28px));margin:auto;padding:34px 0 56px}.top{display:flex;align-items:center;gap:18px;margin-bottom:28px}.brand{width:76px;height:76px;filter:drop-shadow(0 0 20px #7865ff66)}h1{font-size:30px;letter-spacing:.16em;margin:0}.tag{color:var(--muted);margin-top:3px}.live{margin-left:auto;padding:8px 12px;border:1px solid #2c725d;border-radius:999px;color:var(--good);background:#13271f}.grid{display:grid;grid-template-columns:repeat(12,1fr);gap:16px}.card{grid-column:span 4;background:linear-gradient(145deg,#151827ee,#10131fee);border:1px solid var(--line);border-radius:20px;padding:20px;box-shadow:0 18px 45px #0005}.card.wide{grid-column:span 8}.card.full{grid-column:1/-1}.label{font-size:11px;letter-spacing:.16em;text-transform:uppercase;color:var(--muted);margin-bottom:7px}.value{font-size:30px;font-weight:650}.sub{color:var(--muted);margin-top:4px}.section-title{font-size:18px;margin:0 0 18px}.fields{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:16px}.field{display:flex;flex-direction:column;gap:7px}.field label{color:#b8bfd5;font-size:13px}.row{display:flex;align-items:center;gap:12px}input,select,button{font:inherit}input[type=number],input[type=text],select{width:100%;border:1px solid #31374d;border-radius:11px;padding:11px 12px;background:#0c0f1a;color:var(--text);outline:none}input:focus,select:focus{border-color:var(--violet)}input[type=range]{width:100%;accent-color:var(--violet)}.range-value{min-width:42px;text-align:right;color:var(--cyan)}.switch{position:relative;width:44px;height:25px}.switch input{opacity:0}.switch span{position:absolute;inset:0;border-radius:99px;background:#30354a;transition:.2s}.switch span:after{content:"";position:absolute;width:19px;height:19px;left:3px;top:3px;border-radius:50%;background:#fff;transition:.2s}.switch input:checked+span{background:linear-gradient(90deg,var(--violet),var(--pink))}.switch input:checked+span:after{transform:translateX(19px)}.checkrow{display:flex;justify-content:space-between;align-items:center;border-bottom:1px solid #202538;padding:11px 0}.buttons{display:flex;flex-wrap:wrap;gap:10px;margin-top:20px}button,.button{border:0;border-radius:11px;padding:11px 16px;color:#fff;background:#282d40;cursor:pointer;text-decoration:none;display:inline-block}button.primary{background:linear-gradient(115deg,var(--violet),var(--pink));font-weight:650}button.danger{background:#4a202b;color:#ffb8c8}.toast{position:fixed;right:18px;bottom:18px;padding:13px 16px;border-radius:12px;background:#20263a;border:1px solid #39415c;opacity:0;transform:translateY(12px);transition:.2s;pointer-events:none}.toast.show{opacity:1;transform:none}.dot{display:inline-block;width:8px;height:8px;border-radius:50%;background:currentColor;margin-right:6px}.weather-icon{font-size:28px}.footer{text-align:center;color:#68708d;margin-top:30px;font-size:12px}@media(max-width:760px){.shell{padding-top:22px}.brand{width:58px;height:58px}h1{font-size:22px}.live{display:none}.card,.card.wide{grid-column:1/-1}.fields{grid-template-columns:1fr}.value{font-size:25px}}
</style></head>
<body><main class="shell">
<header class="top"><img class="brand" src="/logo.svg" alt="HALO CST"><div><h1>HALO CST</h1><div class="tag">Connected Smart Timepiece</div></div><div class="live"><span class="dot"></span>Live</div></header>
<section class="grid">
<article class="card"><div class="label">Local time</div><div class="value" id="time">--:--:--</div><div class="sub" id="date">Synchronizing</div></article>
<article class="card"><div class="label">Weather</div><div class="value"><span class="weather-icon">☀</span> <span id="temperature">--°</span></div><div class="sub" id="weather">Waiting for data</div></article>
<article class="card"><div class="label">Network</div><div class="value" id="signal">Offline</div><div class="sub" id="ip">--</div></article>

<article class="card wide"><h2 class="section-title">Light &amp; display</h2><div class="fields">
<div class="field"><label>Theme</label><select id="theme"><option value="0">Classic</option><option value="1">Nordic</option><option value="2">Fire</option><option value="3">Matrix</option><option value="4">Cyberpunk</option><option value="5">Sunset</option></select></div>
<div class="field"><label>Display mode</label><select id="mode"><option value="0">Progress</option><option value="1">Analog</option><option value="2">Minimal</option><option value="3">Ambient</option><option value="4">Rainbow</option></select></div>
<div class="field"><label>Day brightness</label><div class="row"><input id="dayBrightness" type="range" min="1" max="150"><span class="range-value" id="dayBrightnessValue"></span></div></div>
<div class="field"><label>Night brightness</label><div class="row"><input id="nightBrightness" type="range" min="1" max="50"><span class="range-value" id="nightBrightnessValue"></span></div></div>
<div class="field"><label>Hour marker width</label><select id="hourWidth"><option value="1">1 LED</option><option value="3">3 LEDs</option><option value="5">5 LEDs</option></select></div>
<div class="field"><label>12 o'clock LED offset</label><input id="ledOffset" type="number" min="0" max="59"></div>
<div class="field"><label>Night starts</label><input id="nightStartHour" type="number" min="0" max="23"></div>
<div class="field"><label>Day starts</label><input id="dayStartHour" type="number" min="0" max="23"></div>
</div></article>

<article class="card"><h2 class="section-title">Face options</h2>
<div class="checkrow"><span>Hour ticks</span><label class="switch"><input id="showTicks" type="checkbox"><span></span></label></div>
<div class="checkrow"><span>Second indicator</span><label class="switch"><input id="showSeconds" type="checkbox"><span></span></label></div>
<div class="checkrow"><span>Clockwise strip</span><label class="switch"><input id="clockwise" type="checkbox"><span></span></label></div>
<div class="checkrow"><span>Weather</span><label class="switch"><input id="weatherEnabled" type="checkbox"><span></span></label></div>
</article>

<article class="card full"><h2 class="section-title">Location &amp; system</h2><div class="fields">
<div class="field"><label>Latitude</label><input id="latitude" type="number" min="-90" max="90" step="0.0001"></div>
<div class="field"><label>Longitude</label><input id="longitude" type="number" min="-180" max="180" step="0.0001"></div>
<div class="field"><label>POSIX timezone</label><input id="timezone" type="text"></div>
<div class="field"><label>Firmware</label><input id="firmware" type="text" disabled></div>
</div><div class="buttons"><button class="primary" onclick="saveSettings()">Save settings</button><button onclick="refreshWeather()">Refresh weather</button><a class="button" href="/update">Firmware update</a><button onclick="reboot()">Restart</button><button class="danger" onclick="resetWifi()">Reset Wi-Fi</button></div></article>
</section><div class="footer">HALO CST · Connected Smart Timepiece</div></main><div class="toast" id="toast"></div>
<script>
const ids=['theme','mode','dayBrightness','nightBrightness','hourWidth','ledOffset','nightStartHour','dayStartHour','showTicks','showSeconds','clockwise','weatherEnabled','latitude','longitude','timezone'];
const $=id=>document.getElementById(id); const toast=m=>{const t=$('toast');t.textContent=m;t.classList.add('show');setTimeout(()=>t.classList.remove('show'),2400)};
function syncRanges(){ $('dayBrightnessValue').textContent=$('dayBrightness').value; $('nightBrightnessValue').textContent=$('nightBrightness').value }
$('dayBrightness').oninput=syncRanges;$('nightBrightness').oninput=syncRanges;
async function loadSettings(){const r=await fetch('/api/settings');const s=await r.json();ids.forEach(id=>{const e=$(id);if(e.type==='checkbox')e.checked=!!s[id];else e.value=s[id]});$('firmware').value=s.firmware;syncRanges()}
async function loadStatus(){try{const r=await fetch('/api/status');const s=await r.json();$('time').textContent=s.time;$('date').textContent=s.date;$('temperature').textContent=s.weather.valid?s.weather.temperature.toFixed(1)+'°C':'--°';$('weather').textContent=s.weather.valid?s.weather.description:'Waiting for data';$('signal').textContent=s.wifi.connected?s.wifi.rssi+' dBm':'Offline';$('ip').textContent=s.wifi.connected?s.wifi.ip:(s.wifi.portal?'Setup portal active':'Disconnected')}catch(e){$('signal').textContent='Offline'}}
async function saveSettings(){const s={};ids.forEach(id=>{const e=$(id);s[id]=e.type==='checkbox'?e.checked:(e.type==='number'||e.type==='range'||e.tagName==='SELECT'?Number(e.value):e.value)});const r=await fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(s)});toast(r.ok?'Settings saved':'Save failed')}
async function action(path,message){const r=await fetch(path,{method:'POST'});toast(r.ok?message:'Request failed')}
const refreshWeather=()=>action('/api/weather/refresh','Weather refresh requested');const reboot=()=>action('/api/reboot','HALO CST is restarting');const resetWifi=()=>confirm('Forget saved Wi-Fi and open setup portal?')&&action('/api/wifi/reset','Wi-Fi settings cleared');
loadSettings();loadStatus();setInterval(loadStatus,2000);
</script></body></html>)HTML";

    const char UPDATE_HTML[] PROGMEM = R"HTML(<!doctype html><html><head><meta name="viewport" content="width=device-width"><title>HALO CST Update</title><style>body{margin:0;background:#0a0c16;color:#f5f7ff;font:16px system-ui;display:grid;place-items:center;min-height:100vh}.box{width:min(440px,90%);padding:28px;border:1px solid #292e43;border-radius:20px;background:#121522}h1{letter-spacing:.12em}input{display:block;width:100%;margin:20px 0;padding:12px;border:1px solid #353b52;border-radius:10px;background:#090b14;color:#fff}button,a{display:inline-block;padding:11px 16px;border:0;border-radius:10px;background:#765dff;color:#fff;text-decoration:none;font:inherit}</style></head><body><form class="box" method="POST" action="/update" enctype="multipart/form-data"><h1>HALO CST</h1><p>Select a PlatformIO <b>firmware.bin</b> file. The clock restarts automatically after a successful update.</p><input type="file" name="firmware" accept=".bin" required><button type="submit">Install firmware</button> <a href="/">Cancel</a></form></body></html>)HTML";
}
