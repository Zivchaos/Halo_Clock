// GENERATED FILE — DO NOT EDIT.
// Source: webui/index.html, webui/styles.css, webui/app.js, and the HALO CST favicon.
// Deterministic content SHA-256: fd97081d5185e60acb71d014270ef3657874712e1c51149fe024aef83430deb4
#pragma once

#include <Arduino.h>

namespace WebUiAssets
{
    inline constexpr size_t SOURCE_BYTES = 56226U;
    inline constexpr size_t INDEX_HTML_BYTES = 51438U;
    inline constexpr size_t FAVICON_SVG_BYTES = 274U;
    inline const char INDEX_HTML[] PROGMEM = R"HALOWEB(<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover"><meta name="theme-color" content="#0e0e0e"><meta name="description" content="Local control and status interface for HALO CST — Connected Smart Timepiece"><title>HALO CST</title><link rel="icon" href="/favicon.svg" type="image/svg+xml"><style>:root{color-scheme:dark;--bg:#070b0e;--bg-soft:#0b1116;--panel:#101820;--panel-strong:#131e27;--panel-inset:#0a1117;--line:#25333c;--line-soft:#1b2830;--text:#f2f2f2;--muted:#9caeb8;--cyan:#22d3ee;--cyan-soft:#7dd3fc;--green:#22c55e;--amber:#f59e0b;--red:#ef4444;--shadow:0 24px 70px rgba(0,0,0,.34);--radius:14px;--sidebar:250px;font-family:Inter,ui-sans-serif,system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif}*{box-sizing:border-box}html{scroll-behavior:smooth;scroll-padding-top:24px}body{margin:0;min-width:300px;overflow-x:hidden;background:radial-gradient(circle at 74% 14%,rgba(34,211,238,.07),transparent 32rem),var(--bg);color:var(--text)}body::before{content:"";position:fixed;inset:0;pointer-events:none;opacity:.025;background-image:linear-gradient(rgba(255,255,255,.5) 1px,transparent 1px),linear-gradient(90deg,rgba(255,255,255,.4) 1px,transparent 1px);background-size:40px 40px}button,input,select{font:inherit}button,a,input,select,summary{-webkit-tap-highlight-color:transparent}a{color:inherit}button{color:inherit}strong{font-weight:650}small{color:var(--muted)}.skip-link{position:fixed;z-index:100;top:12px;left:12px;padding:10px 14px;transform:translateY(-150%);background:var(--cyan);color:var(--bg);border-radius:6px;font-weight:700}.skip-link:focus{transform:translateY(0)}.sr-only{position:absolute;width:1px;height:1px;padding:0;margin:-1px;overflow:hidden;clip:rect(0,0,0,0);white-space:nowrap;border:0}.app-shell{min-height:100vh;display:grid;grid-template-columns:var(--sidebar) minmax(0,1fr)}.sidebar{position:sticky;top:0;height:100vh;display:flex;flex-direction:column;gap:28px;padding:28px 20px 22px;border-right:1px solid var(--line);background:rgba(8,13,17,.94);backdrop-filter:blur(18px)}.brand,.mobile-brand{display:flex;align-items:center;gap:13px;text-decoration:none}.brand strong,.mobile-brand strong{display:block;font-family:"Arial Narrow","Roboto Condensed",Arial,sans-serif;font-size:1.08rem;letter-spacing:.16em;font-weight:500}.brand small,.mobile-brand small{display:block;margin-top:3px;font-size:.67rem;letter-spacing:.06em}.brand-icon{position:relative;width:52px;aspect-ratio:1;flex:0 0 auto;display:grid;place-items:center;background:var(--cyan);clip-path:polygon(29% 0,71% 0,100% 29%,100% 71%,71% 100%,29% 100%,0 71%,0 29%);filter:drop-shadow(0 0 9px rgba(34,211,238,.35))}.brand-icon::before{content:"";position:absolute;inset:3px;background:var(--bg);clip-path:inherit}.brand-oled{z-index:1;width:25px;height:13px;border:1px solid var(--text);background:#030507}.brand-button{position:absolute;z-index:1;width:4px;height:4px;bottom:9px;border-radius:50%;background:var(--cyan)}.nav-list{display:grid;gap:5px}.nav-link{display:flex;align-items:center;gap:12px;min-height:44px;padding:0 13px;color:var(--muted);text-decoration:none;border:1px solid transparent;border-radius:9px;font-size:.86rem}.nav-link span{width:20px;color:var(--cyan-soft);text-align:center;font-size:1.1rem}.nav-link:hover,.nav-link.active{color:var(--text);background:rgba(34,211,238,.08);border-color:rgba(34,211,238,.15);box-shadow:inset 2px 0 var(--cyan)}.sidebar-status{display:flex;align-items:center;gap:11px;margin-top:auto;padding:14px;border:1px solid var(--line);border-radius:10px;background:var(--panel-inset)}.sidebar-status strong,.sidebar-status small{display:block}.sidebar-status strong{font-size:.84rem}.sidebar-status small{max-width:150px;margin-top:2px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;font:.72rem ui-monospace,SFMono-Regular,Consolas,monospace}.sidebar-version{padding-left:4px;font:.7rem ui-monospace,SFMono-Regular,Consolas,monospace}.status-dot{display:inline-block;width:8px;height:8px;flex:0 0 auto;border-radius:50%;background:var(--amber);box-shadow:0 0 10px currentColor;color:var(--amber)}.status-dot.healthy{background:var(--green);color:var(--green)}.status-dot.error{background:var(--red);color:var(--red)}.status-dot.cyan{width:6px;height:6px;background:var(--cyan);color:var(--cyan)}.mobile-header,.mobile-nav{display:none}.content{width:min(1420px,100%);min-width:0;margin:0 auto;padding:30px clamp(22px,3vw,46px) 80px}.page-header{display:flex;align-items:flex-end;justify-content:space-between;gap:20px;margin-bottom:22px}.page-header h1{margin:2px 0 0;font-family:"Arial Narrow","Roboto Condensed",Arial,sans-serif;font-size:clamp(1.7rem,3vw,2.5rem);font-weight:500;letter-spacing:.08em}.eyebrow,.panel-label{margin:0;color:var(--cyan-soft);font-size:.68rem;letter-spacing:.14em;text-transform:uppercase}.header-health,.health-pill{display:flex;align-items:center;gap:9px;min-height:40px;padding:0 13px;border:1px solid var(--line);border-radius:9px;background:rgba(16,24,32,.84);color:var(--muted);font-size:.78rem}.section{margin-bottom:72px;scroll-margin-top:20px}.overview-section{display:grid;grid-template-columns:minmax(400px,1.35fr) minmax(280px,.8fr);gap:14px}.panel{min-width:0;border:1px solid var(--line);border-radius:var(--radius);background:linear-gradient(145deg,rgba(18,28,36,.96),rgba(11,18,24,.96));box-shadow:var(--shadow)}.panel-label{display:flex;align-items:center;gap:8px;color:var(--muted)}.clock-panel{grid-row:span 2;min-height:520px;padding:20px;display:flex;flex-direction:column}.octagon-clock{position:relative;width:min(430px,88%);aspect-ratio:1;margin:auto;display:grid;place-items:center;isolation:isolate}.octagon-glow{position:absolute;inset:0;background:var(--cyan);clip-path:polygon(29% 0,71% 0,100% 29%,100% 71%,71% 100%,29% 100%,0 71%,0 29%);filter:drop-shadow(0 0 9px rgba(34,211,238,.78)) drop-shadow(0 0 28px rgba(34,211,238,.28))}.octagon-glow::after{content:"";position:absolute;inset:7px;background:radial-gradient(circle at 50% 45%,#101c23 0,#071015 58%,#05090c 100%);clip-path:inherit}.octagon-face{position:absolute;inset:8px;z-index:1;clip-path:polygon(29% 0,71% 0,100% 29%,100% 71%,71% 100%,29% 100%,0 71%,0 29%)}.cardinal{position:absolute;color:#dfe9ed;font-family:"Arial Narrow","Roboto Condensed",Arial,sans-serif;font-size:clamp(1rem,2.2vw,1.45rem)}.cardinal-12{top:9%;left:50%;transform:translateX(-50%)}.cardinal-3{top:50%;right:9%;transform:translateY(-50%)}.cardinal-6{bottom:9%;left:50%;transform:translateX(-50%)}.cardinal-9{top:50%;left:9%;transform:translateY(-50%)}.oled-display{position:absolute;top:50%;left:50%;width:47%;min-width:150px;transform:translate(-50%,-56%);padding:14px 12px 11px;text-align:center;border:1px solid #1d2d35;border-radius:3px;background:#020405;box-shadow:inset 0 0 18px rgba(125,211,252,.04),0 8px 30px rgba(0,0,0,.5)}.oled-display time{display:block;font:clamp(1.55rem,4.3vw,2.6rem) ui-monospace,SFMono-Regular,Consolas,monospace;letter-spacing:-.04em}.oled-display span{display:block;margin-top:6px;color:var(--muted);font:.62rem ui-monospace,SFMono-Regular,Consolas,monospace;letter-spacing:.08em;text-transform:uppercase}.prototype-button{position:absolute;left:50%;top:68%;width:10px;height:10px;transform:translateX(-50%);border-radius:50%;border:1px solid #43535b;background:#050708;box-shadow:0 0 8px rgba(34,211,238,.16)}.clock-caption{display:flex;justify-content:space-between;gap:12px;color:var(--muted);font-size:.74rem}.clock-caption span:last-child{text-align:right}.overview-rail{display:grid;gap:14px}.summary-card,.weather-card{padding:18px}.summary-row{display:flex;align-items:center;gap:13px;margin-top:16px}.summary-row div{flex:1}.summary-row strong,.summary-row small{display:block}.summary-row strong{font-size:1.05rem}.summary-icon{width:38px;height:38px;display:grid;place-items:center;border:1px solid rgba(34,211,238,.45);border-radius:50%;color:var(--cyan);font-size:1.2rem}.chip{padding:5px 8px;color:var(--cyan-soft);border:1px solid rgba(34,211,238,.22);border-radius:999px;background:rgba(34,211,238,.07);font-size:.66rem}.level-track{height:4px;margin:15px 0 2px 51px;overflow:hidden;border-radius:3px;background:#293138}.level-track span{display:block;width:0;height:100%;background:linear-gradient(90deg,var(--cyan),#3b82f6);box-shadow:0 0 8px var(--cyan);transition:width .25s ease}.weather-primary{display:flex;align-items:center;gap:16px;margin:13px 0 7px}.weather-primary strong,.weather-primary span{display:block}.weather-primary strong{font:2rem "Arial Narrow",Arial,sans-serif}.weather-primary span{margin-top:2px;color:var(--text);font-size:.85rem}.weather-icon{color:var(--cyan-soft);font-size:2.5rem;line-height:1}.weather-card p{margin:5px 0;color:var(--muted);font-size:.78rem}.meta{padding-top:9px;border-top:1px solid var(--line-soft)}.overview-strip{grid-column:1 / -1;display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:14px}.compact-card{padding:16px}.compact-card strong,.compact-card>span{display:block}.compact-card strong{margin:10px 0 4px;font-size:.94rem}.compact-card>span{overflow:hidden;color:var(--muted);font-size:.74rem;text-overflow:ellipsis;white-space:nowrap}.quick-controls{grid-column:1 / -1;display:grid;grid-template-columns:210px 1fr;align-items:center;gap:18px;padding:17px}.quick-controls p{margin:7px 0 0;color:var(--muted);font-size:.75rem}.quick-grid{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:9px}.quick-button{min-width:0;min-height:58px;display:flex;align-items:center;gap:10px;padding:9px 12px;text-align:left;border:1px solid var(--line);border-radius:9px;background:var(--panel-inset);cursor:pointer}.quick-button:hover{border-color:rgba(34,211,238,.48);background:rgba(34,211,238,.06)}.quick-button>span:first-child{color:var(--cyan-soft);font-size:1.25rem}.quick-button strong,.quick-button small{display:block;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}.quick-button strong{font-size:.78rem}.quick-button small{margin-top:2px;font-size:.65rem}.section-heading{display:flex;align-items:end;justify-content:space-between;gap:24px;margin-bottom:18px}.section-heading h2{margin:3px 0 0;font-family:"Arial Narrow","Roboto Condensed",Arial,sans-serif;font-size:clamp(1.45rem,2.5vw,2rem);font-weight:500;letter-spacing:.05em}.section-heading>p{max-width:540px;margin:0;color:var(--muted);font-size:.8rem;text-align:right}.two-column{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:14px}.form-panel,.system-card,.weather-detail{padding:20px}.form-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:14px;margin:18px 0}label{color:var(--muted);font-size:.76rem}label span{color:#748893}select,input[type="time"],input[type="text"]{width:100%;min-height:44px;margin-top:7px;padding:9px 11px;color:var(--text);border:1px solid #30414b;border-radius:8px;outline:none;background:#081016}select:focus,input:focus{border-color:var(--cyan);box-shadow:0 0 0 3px rgba(34,211,238,.14)}select:disabled,input:disabled,button:disabled{opacity:.52;cursor:not-allowed}.readout-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:8px;margin-top:17px}.readout-grid>div{padding:11px;border:1px solid var(--line-soft);border-radius:8px;background:var(--panel-inset)}.readout-grid span,.readout-grid strong{display:block}.readout-grid span{color:var(--muted);font-size:.65rem;text-transform:uppercase;letter-spacing:.08em}.readout-grid strong{margin-top:5px;font-size:.86rem}.switch-row{display:flex;align-items:center;justify-content:space-between;gap:18px;margin:18px 0}.switch-row strong,.switch-row small{display:block}.switch-row strong{color:var(--text);font-size:.86rem}.switch-row small{margin-top:3px;font-size:.68rem}.switch-row input{width:44px;height:24px;flex:0 0 auto;accent-color:var(--cyan)}.button{min-height:42px;display:inline-flex;align-items:center;justify-content:center;padding:9px 15px;border:1px solid transparent;border-radius:8px;text-decoration:none;font-weight:650;font-size:.77rem;cursor:pointer}.button.primary{color:#031013;border-color:var(--cyan);background:var(--cyan)}.button.primary:hover{background:#68e8f8}.button.secondary{border-color:#344650;background:#111d24}.button.secondary:hover{border-color:var(--cyan)}.button.danger{border-color:rgba(239,68,68,.55);background:rgba(239,68,68,.12);color:#fecaca}.button.danger:hover{border-color:var(--red);background:rgba(239,68,68,.2)}.button-row{display:flex;flex-wrap:wrap;gap:9px;margin-top:18px}.network-summary{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:14px;margin-bottom:14px}.warning-banner{display:grid;gap:5px;padding:13px 15px;border:1px solid rgba(245,158,11,.38);border-left:3px solid var(--amber);border-radius:8px;background:rgba(245,158,11,.07)}.warning-banner strong{color:#fcd48b;font-size:.8rem}.warning-banner span{color:#c6ad80;font-size:.73rem}.lazy-row{display:flex;align-items:center;justify-content:space-between;gap:18px;margin:18px 0 0}.lazy-row p{margin:0;color:var(--muted);font-size:.75rem}.network-fields{grid-template-columns:repeat(3,minmax(0,1fr))}.recovery-note{margin-top:19px;padding-top:16px;border-top:1px solid var(--line)}.recovery-note summary{color:var(--cyan-soft);cursor:pointer;font-size:.76rem;font-weight:650}.recovery-note p{margin-bottom:0;color:var(--muted);font-size:.74rem;line-height:1.65}.weather-detail{display:grid;grid-template-columns:minmax(220px,.7fr) minmax(380px,1.3fr);gap:20px;align-items:center}.weather-hero{display:flex;align-items:center;gap:22px}.weather-icon.large{font-size:4rem}.weather-hero strong,.weather-hero span{display:block}.weather-hero strong{font:3.1rem "Arial Narrow",Arial,sans-serif}.weather-hero span{color:var(--muted)}.weather-readouts{grid-template-columns:repeat(4,minmax(0,1fr));margin:0}.weather-state{grid-column:1 / -1;padding:10px 12px;color:var(--muted);border-left:2px solid var(--cyan);background:var(--panel-inset);font-size:.74rem}.weather-detail>.button{grid-column:1 / -1;justify-self:start}.diagnostics-panel{padding:0;overflow:hidden}.diagnostics-panel>summary{min-height:82px;display:flex;align-items:center;justify-content:space-between;gap:18px;padding:18px 20px;cursor:pointer;list-style:none}.diagnostics-panel>summary::-webkit-details-marker{display:none}.diagnostics-panel>summary strong,.diagnostics-panel>summary small{display:block}.diagnostics-panel>summary small{margin-top:4px}.diagnostics-panel[open]>summary{border-bottom:1px solid var(--line)}.diagnostics-panel[open]>summary>span:last-child{transform:rotate(45deg)}.diagnostic-overview{padding:17px 20px 0;color:var(--muted);font-size:.76rem}.diagnostic-overview p{margin:0}.diagnostics{max-height:420px;margin:16px 20px;overflow:auto;padding:15px;color:#c7e4ec;border:1px solid var(--line-soft);border-radius:8px;background:#05090c;font:.72rem/1.55 ui-monospace,SFMono-Regular,Consolas,monospace;white-space:pre-wrap;overflow-wrap:anywhere}.diagnostic-actions{padding:0 20px 20px}.system-card p{color:var(--muted);font-size:.78rem;line-height:1.65}.system-card .button{margin-top:18px}.toast{position:fixed;z-index:80;right:22px;bottom:22px;max-width:min(390px,calc(100vw - 44px));padding:12px 15px;transform:translateY(150%);opacity:0;color:var(--text);border:1px solid var(--cyan);border-radius:9px;background:#0d2027;box-shadow:var(--shadow);font-size:.78rem;transition:transform .2s ease,opacity .2s ease}.toast.visible{transform:translateY(0);opacity:1}.toast.error{border-color:var(--red);background:#281015}.toast.warning{border-color:var(--amber);background:#261b0b}dialog{width:min(430px,calc(100% - 28px));padding:0;color:var(--text);border:1px solid var(--line);border-radius:14px;background:var(--panel);box-shadow:0 30px 100px #000}dialog::backdrop{background:rgba(0,0,0,.78);backdrop-filter:blur(4px)}dialog form{padding:24px;text-align:center}dialog h2{margin:15px 0 8px;font-family:"Arial Narrow",Arial,sans-serif;font-weight:500;letter-spacing:.05em}dialog p{margin:0;color:var(--muted);font-size:.8rem;line-height:1.55}.dialog-mark{width:48px;height:48px;display:grid;place-items:center;margin:0 auto;color:var(--cyan);border:1px solid var(--cyan);border-radius:50%;font-size:1.5rem}.dialog-mark.warning{color:var(--amber);border-color:var(--amber)}.dialog-actions{display:grid;grid-template-columns:1fr 1fr;gap:9px;margin-top:22px}:focus-visible{outline:2px solid var(--cyan);outline-offset:3px}@media (max-width:1120px){:root{--sidebar:220px}.overview-section{grid-template-columns:minmax(360px,1.15fr) minmax(260px,.85fr)}.overview-strip,.network-summary{grid-template-columns:repeat(2,minmax(0,1fr))}.quick-controls{grid-template-columns:1fr}.weather-detail{grid-template-columns:1fr}.weather-detail>.button,.weather-state{grid-column:auto}.weather-readouts{grid-template-columns:repeat(2,minmax(0,1fr))}.network-fields{grid-template-columns:repeat(2,minmax(0,1fr))}}@media (max-width:820px){html{scroll-padding-top:116px}.app-shell{display:block}.sidebar{display:none}.mobile-header{position:sticky;z-index:50;top:0;min-height:64px;display:flex;align-items:center;justify-content:space-between;gap:12px;padding:8px 14px;border-bottom:1px solid var(--line);background:rgba(7,11,14,.95);backdrop-filter:blur(18px)}.mobile-brand strong{font-size:.9rem}.mobile-brand small{font-size:.58rem}.mini-octagon{width:34px;aspect-ratio:1;background:var(--cyan);clip-path:polygon(29% 0,71% 0,100% 29%,100% 71%,71% 100%,29% 100%,0 71%,0 29%);box-shadow:inset 0 0 0 3px #081015}.health-pill{min-height:34px;padding:0 10px;font-size:.68rem}.mobile-nav{position:sticky;z-index:49;top:64px;display:flex;gap:6px;overflow-x:auto;padding:7px 10px 9px;border-bottom:1px solid var(--line);background:rgba(8,13,17,.96);scrollbar-width:none}.mobile-nav::-webkit-scrollbar{display:none}.mobile-nav a{flex:0 0 auto;padding:7px 10px;color:var(--muted);border-radius:6px;text-decoration:none;font-size:.68rem}.mobile-nav a.active{color:var(--bg);background:var(--cyan)}.content{padding:20px 14px 68px}.page-header{display:none}.section{margin-bottom:54px;scroll-margin-top:116px}.overview-section{display:grid;grid-template-columns:1fr}.clock-panel{grid-row:auto;min-height:440px}.octagon-clock{width:min(390px,92%)}.overview-strip,.quick-controls{grid-column:auto}.two-column{grid-template-columns:1fr}.section-heading{align-items:start}.section-heading>p{max-width:44%}}@media (max-width:520px){.mobile-header{min-height:62px}.mobile-nav{top:62px}.health-pill>span:last-child{display:none}.content{padding-inline:9px}.clock-panel{min-height:380px;padding:15px}.octagon-clock{width:min(340px,96%)}.oled-display{min-width:128px;padding:11px 8px 9px}.prototype-button{top:69%;width:8px;height:8px}.clock-caption{font-size:.65rem}.overview-strip,.network-summary{grid-template-columns:1fr 1fr;gap:8px}.compact-card{padding:13px}.quick-grid{grid-template-columns:repeat(2,minmax(0,1fr))}.quick-button{padding:8px 9px}.section-heading{display:block}.section-heading>p{max-width:none;margin-top:7px;text-align:left}.form-panel,.system-card,.weather-detail{padding:15px}.form-grid,.network-fields{grid-template-columns:1fr;gap:10px}.readout-grid,.weather-readouts{grid-template-columns:repeat(2,minmax(0,1fr))}.weather-hero strong{font-size:2.5rem}.weather-icon.large{font-size:3rem}.lazy-row{align-items:stretch;flex-direction:column}.lazy-row .button{width:100%}.button-row{display:grid;grid-template-columns:1fr}.button-row .button{width:100%}.diagnostic-actions{display:grid;grid-template-columns:1fr}.toast{right:9px;bottom:9px;max-width:calc(100vw - 18px)}}@media (max-width:360px){.overview-strip{grid-template-columns:1fr}.clock-panel{min-height:350px}.quick-grid{grid-template-columns:1fr}}@media (prefers-reduced-motion:reduce){*,*::before,*::after{scroll-behavior:auto !important;transition-duration:.01ms !important;animation-duration:.01ms !important;animation-iteration-count:1 !important}}</style><script>"use strict";
const $ = (id) => document.getElementById(id);
const POLL_INTERVAL_MS = 3000;
const state = {
busy: false,
diagnostics: null,
networkLoaded: false,
networkAction: "save",
pollFailures: 0,
statusInFlight: false,
toastTimer: 0
};
function text(id, value) {
const element = $(id);
if (element) element.textContent = value ?? "—";
}
function formatDuration(seconds) {
const value = Number(seconds) || 0;
const days = Math.floor(value / 86400);
const hours = Math.floor((value % 86400) / 3600);
const minutes = Math.floor((value % 3600) / 60);
return `${days ? `${days}d ` : ""}${hours}h ${minutes}m`;
}
function formatBytes(value) {
const bytes = Number(value);
if (!Number.isFinite(bytes)) return "—";
return bytes >= 1024 ? `${(bytes / 1024).toFixed(1)} KiB` : `${bytes} B`;
}
function showFeedback(message, kind = "success") {
const feedback = $("feedback");
window.clearTimeout(state.toastTimer);
feedback.textContent = message;
feedback.className = `toast visible${kind === "success" ? "" : ` ${kind}`}`;
state.toastTimer = window.setTimeout(() => feedback.classList.remove("visible"), 4500);
}
function syncControlState() {
document.querySelectorAll("[data-control]").forEach((control) => {
const isNetworkControl = $("networkForm").contains(control) || control.id === "networkReset";
control.disabled = state.busy || (isNetworkControl && !state.networkLoaded);
});
if (state.networkLoaded) toggleStaticFields();
}
function setBusy(value) {
state.busy = value;
document.body.toggleAttribute("aria-busy", value);
syncControlState();
}
async function api(path, options = {}) {
const response = await fetch(path, { cache: "no-store", ...options });
const raw = await response.text();
let body = {};
if (raw) {
try { body = JSON.parse(raw); }
catch { throw new Error(`Invalid JSON from ${path}`); }
}
if (!response.ok || body.ok === false) throw new Error(body.error || `${path} returned HTTP ${response.status}`);
return body;
}
function setConnectionState(connected, stale = false) {
const label = stale ? "Status temporarily unavailable" : (connected ? "System healthy" : "Wi-Fi disconnected");
[$("sidebarDot"), $("healthStatus").querySelector(".status-dot"), $("mobileHealth").querySelector(".status-dot")].forEach((dot) => {
dot.className = `status-dot ${connected && !stale ? "healthy" : "error"}`;
});
text("connectionStatus", stale ? "Status stale" : (connected ? "Online" : "Offline"));
$("healthStatus").lastElementChild.textContent = label;
$("mobileHealth").lastElementChild.textContent = connected && !stale ? "Healthy" : "Offline";
}
function applyStatus(status) {
if (!status || typeof status !== "object" || typeof status.time !== "string") throw new Error("Status response is incomplete");
state.pollFailures = 0;
setConnectionState(Boolean(status.wifiConnected));
text("clock", status.time || "--:--:--");
text("clockDate", new Date().toLocaleDateString(undefined, { weekday: "short", month: "short", day: "numeric" }));
text("clockModeCaption", `${status.effectiveMode || "—"} rendering`);
text("clockNightCaption", `NIGHT ${status.autoNightActive ? "active" : "scheduled"} · ${status.autoNightStart || "—"}–${status.autoNightEnd || "—"}`);
text("wifi", status.wifiConnected ? "Connected" : "Disconnected");
text("ip", status.ip || "—");
text("sidebarAddress", status.wifiConnected ? status.ip : "Waiting for network");
text("rssi", Number.isFinite(Number(status.rssi)) ? `${status.rssi} dBm` : "—");
text("uptime", formatDuration(status.uptimeSeconds));
text("firmware", status.firmwareVersion);
text("sidebarFirmware", status.firmwareVersion);
text("ota", status.otaUpdating ? "Updating" : (status.otaReady ? "Ready" : "Waiting"));
text("otaDetail", status.otaUpdating ? "Updating" : (status.otaReady ? "Ready on LAN" : "Waiting for Wi-Fi"));
text("runningPartition", status.runningPartition);
text("freeHeap", formatBytes(status.freeHeap));
text("minimumHeap", formatBytes(status.minimumFreeHeap));
text("selected", status.selectedMode);
text("effective", status.effectiveMode);
text("effectiveDisplay", status.effectiveMode);
text("brightnessText", status.brightness);
text("quickMode", status.selectedMode);
text("quickBrightness", `Level ${status.brightness}`);
$("brightnessTrack").style.width = `${Math.min(100, Math.max(0, Number(status.brightness) / 80 * 100))}%`;
text("override", status.manualOverride ? "Active" : "None");
text("autoState", status.autoNightEnabled ? (status.autoNightActive ? "Active" : "Enabled") : "Disabled");
text("schedule", `${status.autoNightStart || "—"}–${status.autoNightEnd || "—"}`);
text("networkConfigured", status.networkConfiguredMode || "—");
text("networkConfiguredDetail", status.networkConfiguredMode || "—");
text("networkEffective", `${status.networkEffectiveMode || "—"}${status.dhcpFallbackActive ? " (fallback)" : ""}`);
text("fallbackStatus", status.dhcpFallbackActive ? "Temporary DHCP fallback active" : "No fallback active");
text("networkFailures", status.staticConnectionFailureCount ?? "—");
text("quickNetwork", `${status.networkEffectiveMode || "—"} · ${status.ip || "—"}`);
const weatherAvailable = Boolean(status.weatherAvailable);
const temperature = weatherAvailable && Number.isFinite(Number(status.temperature)) ? `${Number(status.temperature).toFixed(1)}°C` : "—";
const apparent = weatherAvailable && Number.isFinite(Number(status.apparentTemperature)) ? `${Number(status.apparentTemperature).toFixed(1)}°C` : "—";
const condition = weatherAvailable ? (status.condition || "Current conditions") : "Weather unavailable";
text("weatherTemperature", temperature);
text("weatherTemperatureDetail", temperature);
text("weatherCondition", condition);
text("weatherConditionDetail", condition);
text("apparentTemperature", apparent);
text("humidity", weatherAvailable ? `${status.humidity}%` : "—");
text("windSpeed", weatherAvailable ? `${Number(status.windSpeed).toFixed(1)} km/h` : "—");
text("weatherDetails", weatherAvailable ? `Humidity ${status.humidity}% · Wind ${Number(status.windSpeed).toFixed(1)} km/h` : (status.weatherError || "Waiting for data"));
const lastUpdate = status.weatherLastUpdate ? new Date(Number(status.weatherLastUpdate) * 1000).toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" }) : "—";
text("weatherLastUpdate", lastUpdate);
const weatherState = status.weatherStale ? "Cached conditions are stale" : (status.weatherError && weatherAvailable ? `Using cached conditions · ${status.weatherError}` : (weatherAvailable ? "Current conditions available" : (status.weatherError || "Waiting for current conditions")));
text("weatherState", weatherState);
text("weatherAge", `${status.weatherStale ? "Stale" : (weatherAvailable ? "Current" : "Unavailable")}${lastUpdate !== "—" ? ` · ${lastUpdate}` : ""}`);
text("diagnosticsSummary", status.wifiConnected && status.otaReady && !status.weatherStale ? "All systems normal" : "Review status");
if (!state.busy) {
$("mode").value = status.selectedMode;
$("brightness").value = String(status.brightness);
$("autoEnabled").checked = Boolean(status.autoNightEnabled);
$("autoStart").value = status.autoNightStart || "";
$("autoEnd").value = status.autoNightEnd || "";
}
}
async function refreshStatus() {
if (state.statusInFlight) return;
state.statusInFlight = true;
try { applyStatus(await api("/api/status")); }
catch (error) {
state.pollFailures += 1;
setConnectionState(false, true);
if (state.pollFailures === 1) showFeedback(`${error.message}; keeping last known values`, "warning");
} finally {
state.statusInFlight = false;
}
}
async function post(path, data, successMessage = "Saved") {
setBusy(true);
try {
await api(path, { method: "POST", headers: { "Content-Type": "application/x-www-form-urlencoded" }, body: new URLSearchParams(data) });
showFeedback(successMessage);
await refreshStatus();
return true;
} catch (error) {
showFeedback(error.message, "error");
return false;
} finally {
setBusy(false);
}
}
function diagnosticSummary(snapshot) {
if (!snapshot?.system || !snapshot?.wifi || !snapshot?.ota) return "Diagnostic snapshot loaded with optional fields unavailable.";
return `Uptime ${formatDuration(snapshot.system.uptimeSeconds)} · Heap ${formatBytes(snapshot.system.freeHeap)} · Wi-Fi reconnects ${snapshot.wifi.reconnectCount} · OTA ${snapshot.ota.ready ? "ready" : "waiting"}`;
}
async function loadDiagnostics(force = false) {
if (state.diagnostics && !force) return state.diagnostics;
text("diagnosticsOutput", "Loading diagnostic snapshot…");
try {
state.diagnostics = await api("/api/diagnostics");
text("diagnosticsOutput", JSON.stringify(state.diagnostics, null, 2));
text("diagnosticOverview", diagnosticSummary(state.diagnostics));
return state.diagnostics;
} catch (error) {
state.diagnostics = null;
text("diagnosticsOutput", error.message);
text("diagnosticOverview", "Diagnostics could not be loaded; the last clock status remains visible.");
showFeedback(error.message, "error");
return null;
}
}
async function copyDiagnostics() {
const snapshot = await loadDiagnostics();
if (!snapshot) return;
const value = JSON.stringify(snapshot, null, 2);
try {
if (navigator.clipboard?.writeText) await navigator.clipboard.writeText(value);
else {
const area = document.createElement("textarea");
area.value = value;
area.setAttribute("readonly", "");
document.body.appendChild(area);
area.select();
document.execCommand("copy");
area.remove();
}
showFeedback("Diagnostics copied");
} catch { showFeedback("Copy failed; use Download JSON instead", "error"); }
}
async function downloadDiagnostics() {
const snapshot = await loadDiagnostics();
if (!snapshot) return;
const url = URL.createObjectURL(new Blob([JSON.stringify(snapshot, null, 2)], { type: "application/json" }));
const link = document.createElement("a");
link.href = url;
link.download = "halo-cst-diagnostics.json";
link.click();
URL.revokeObjectURL(url);
showFeedback("Diagnostics JSON prepared");
}
function toggleStaticFields() {
const staticMode = $("networkMode").value === "STATIC";
$("staticNetworkFields").querySelectorAll("input").forEach((input) => { input.disabled = state.busy || !state.networkLoaded || !staticMode; });
}
async function loadNetwork(force = false) {
if (state.networkLoaded && !force) return;
text("networkLoadState", "Loading saved network configuration…");
try {
const network = await api("/api/network");
state.networkLoaded = true;
text("networkConfiguredDetail", network.configured.mode);
text("networkEffective", `${network.effective.mode}${network.effective.dhcpFallbackActive ? " (fallback)" : ""}`);
text("networkActive", `${network.active.ip} / ${network.active.subnet}`);
text("networkFailures", network.effective.staticConnectionFailureCount);
text("fallbackStatus", network.effective.dhcpFallbackActive ? "Temporary DHCP fallback active" : "No fallback active");
$("networkMode").value = network.configured.mode;
$("networkIp").value = network.configured.staticIp || "";
$("networkGateway").value = network.configured.gateway || "";
$("networkSubnet").value = network.configured.subnet || "";
$("networkDns1").value = network.configured.primaryDns || "";
$("networkDns2").value = network.configured.secondaryDnsConfigured ? network.configured.secondaryDns : "";
text("networkLoadState", "Saved network configuration loaded. Changes require confirmation and reboot.");
syncControlState();
} catch (error) {
state.networkLoaded = false;
text("networkLoadState", error.message);
showFeedback(error.message, "error");
syncControlState();
}
}
function showNetworkConfirmation(action) {
state.networkAction = action;
text("networkConfirmText", action === "reset" ? "Reset the saved network mode to DHCP and reboot? Saved static addressing will be replaced." : "Save these settings and reboot? The current address may stop responding.");
$("networkDialog").showModal();
}
async function applyNetwork() {
$("networkDialog").close();
const reset = state.networkAction === "reset";
const path = reset ? "/api/network/reset" : "/api/network";
const data = reset ? { confirm: "true" } : {
confirm: "true",
mode: $("networkMode").value,
ip: $("networkIp").value.trim(),
gateway: $("networkGateway").value.trim(),
subnet: $("networkSubnet").value.trim(),
primaryDns: $("networkDns1").value.trim(),
secondaryDns: $("networkDns2").value.trim()
};
setBusy(true);
try {
await api(path, { method: "POST", headers: { "Content-Type": "application/x-www-form-urlencoded" }, body: new URLSearchParams(data) });
showFeedback("Saved. HALO CST is rebooting; reconnect using the configured address.");
} catch (error) {
showFeedback(error.message, "error");
setBusy(false);
}
}
function jumpToControl(target) {
if (target === "network") {
$("network").scrollIntoView({ behavior: "smooth" });
void loadNetwork();
return;
}
const control = $(target);
control?.scrollIntoView({ behavior: "smooth", block: "center" });
window.setTimeout(() => control?.focus(), 350);
}
function initializeUi() {
document.querySelectorAll("[data-section-link]").forEach((link) => link.addEventListener("click", () => {
const section = link.dataset.sectionLink;
document.querySelectorAll("[data-section-link]").forEach((item) => item.classList.toggle("active", item.dataset.sectionLink === section));
if (section === "network") void loadNetwork();
}));
document.querySelectorAll("[data-jump]").forEach((button) => button.addEventListener("click", () => jumpToControl(button.dataset.jump)));
$("mode").addEventListener("change", (event) => { void post("/api/mode", { mode: event.target.value }, `Display mode: ${event.target.value}`); });
$("brightness").addEventListener("change", (event) => { void post("/api/brightness", { value: event.target.value }, `Brightness saved: ${event.target.value}`); });
$("weatherRefresh").addEventListener("click", () => { void post("/api/weather/refresh", {}, "Weather refresh requested"); });
$("weatherRefreshQuick").addEventListener("click", () => { void post("/api/weather/refresh", {}, "Weather refresh requested"); });
$("autoForm").addEventListener("submit", (event) => {
event.preventDefault();
const start = $("autoStart").value.split(":");
const end = $("autoEnd").value.split(":");
void post("/api/auto-night", { enabled: String($("autoEnabled").checked), startHour: start[0] || "", startMinute: start[1] || "", endHour: end[0] || "", endMinute: end[1] || "" }, "Automatic NIGHT schedule saved");
});
$("networkLoad").addEventListener("click", () => { void loadNetwork(true); });
$("networkMode").addEventListener("change", toggleStaticFields);
$("networkForm").addEventListener("submit", (event) => { event.preventDefault(); showNetworkConfirmation("save"); });
$("networkReset").addEventListener("click", () => showNetworkConfirmation("reset"));
$("networkCancel").addEventListener("click", () => $("networkDialog").close());
$("networkConfirm").addEventListener("click", () => { void applyNetwork(); });
$("diagnosticsPanel").addEventListener("toggle", (event) => { if (event.target.open) void loadDiagnostics(); });
$("diagnosticsRefresh").addEventListener("click", () => { void loadDiagnostics(true); });
$("diagnosticsCopy").addEventListener("click", () => { void copyDiagnostics(); });
$("diagnosticsDownload").addEventListener("click", () => { void downloadDiagnostics(); });
$("reboot").addEventListener("click", () => $("rebootDialog").showModal());
$("rebootCancel").addEventListener("click", () => $("rebootDialog").close());
$("rebootConfirm").addEventListener("click", () => {
$("rebootDialog").close();
void post("/api/reboot", { confirm: "true" }, "HALO CST is rebooting");
});
syncControlState();
void refreshStatus();
window.setInterval(refreshStatus, POLL_INTERVAL_MS);
}
if (document.readyState === "loading") {
document.addEventListener("DOMContentLoaded", initializeUi);
} else {
initializeUi();
}</script></head><body><a class="skip-link" href="#overview">Skip to content</a><div class="app-shell"><aside class="sidebar" aria-label="Primary navigation"><a class="brand" href="#overview" aria-label="HALO CST overview"><span class="brand-icon" aria-hidden="true"><span class="brand-oled"></span><span class="brand-button"></span></span><span><strong>HALO CST</strong><small>Connected Smart Timepiece</small></span></a><nav class="nav-list"><a class="nav-link active" href="#overview" data-section-link="overview"><span aria-hidden="true">⌂</span>Overview</a><a class="nav-link" href="#time-display" data-section-link="time-display"><span aria-hidden="true">◷</span>Time &amp; Display</a><a class="nav-link" href="#network" data-section-link="network"><span aria-hidden="true">⌁</span>Network</a><a class="nav-link" href="#weather" data-section-link="weather"><span aria-hidden="true">☁</span>Weather</a><a class="nav-link" href="#diagnostics" data-section-link="diagnostics"><span aria-hidden="true">⌁</span>Diagnostics</a><a class="nav-link" href="#settings" data-section-link="settings"><span aria-hidden="true">⚙</span>Settings</a></nav><div class="sidebar-status"><span class="status-dot" id="sidebarDot" aria-hidden="true"></span><span><strong id="connectionStatus">Connecting</strong><small id="sidebarAddress">Waiting for device</small></span></div><small class="sidebar-version">Firmware <span id="sidebarFirmware">—</span></small></aside><header class="mobile-header"><a class="mobile-brand" href="#overview"><span class="mini-octagon" aria-hidden="true"></span><span><strong>HALO CST</strong><small>Connected Smart Timepiece</small></span></a><span class="health-pill" id="mobileHealth"><span class="status-dot" aria-hidden="true"></span><span>Connecting</span></span></header><nav class="mobile-nav" aria-label="Section navigation"><a href="#overview" data-section-link="overview">Overview</a><a href="#time-display" data-section-link="time-display">Display</a><a href="#network" data-section-link="network">Network</a><a href="#weather" data-section-link="weather">Weather</a><a href="#diagnostics" data-section-link="diagnostics">Diagnostics</a><a href="#settings" data-section-link="settings">Settings</a></nav><main class="content" id="main-content"><header class="page-header"><div><p class="eyebrow">Local device interface</p><h1>Overview</h1></div><div class="header-health" id="healthStatus"><span class="status-dot" aria-hidden="true"></span><span>Connecting to HALO CST</span></div></header><section id="overview" class="section overview-section" aria-labelledby="overview-title"><h2 class="sr-only" id="overview-title">Overview</h2><article class="panel clock-panel"><div class="panel-label"><span class="status-dot cyan" aria-hidden="true"></span>Live clock</div><div class="octagon-clock" data-testid="octagon-clock" aria-label="HALO CST live octagonal clock"><div class="octagon-glow" aria-hidden="true"></div><div class="octagon-face"><span class="cardinal cardinal-12">12</span><span class="cardinal cardinal-3">3</span><span class="cardinal cardinal-6">6</span><span class="cardinal cardinal-9">9</span><div class="oled-display"><time id="clock">--:--:--</time><span id="clockDate">Device local time</span></div><span class="prototype-button" aria-hidden="true"></span></div></div><div class="clock-caption"><span id="clockModeCaption">Waiting for synchronized time</span><span id="clockNightCaption">Schedule —</span></div></article><div class="overview-rail"><article class="panel summary-card"><div class="panel-label">Current mode</div><div class="summary-row"><span class="summary-icon" aria-hidden="true">◷</span><div><strong id="selected">—</strong><small>Selected mode</small></div><span class="chip" id="effective">—</span></div></article><article class="panel summary-card"><div class="panel-label">Brightness</div><div class="summary-row"><span class="summary-icon" aria-hidden="true">☼</span><div><strong><span id="brightnessText">—</span></strong><small>Normal LED level</small></div></div><div class="level-track" aria-hidden="true"><span id="brightnessTrack"></span></div></article><article class="panel weather-card"><div class="panel-label">Weather</div><div class="weather-primary"><span class="weather-icon" aria-hidden="true">☁</span><div><strong id="weatherTemperature">—</strong><span id="weatherCondition">Waiting for data</span></div></div><p id="weatherDetails">Humidity — · Wind —</p><p class="meta" id="weatherAge">Not updated</p></article></div><div class="overview-strip"><article class="panel compact-card"><div class="panel-label">Automatic NIGHT</div><strong id="autoState">—</strong><span id="schedule">—</span></article><article class="panel compact-card"><div class="panel-label">Network status</div><strong id="wifi">Connecting</strong><span><span id="networkConfigured">—</span> · <span id="ip">—</span></span></article><article class="panel compact-card"><div class="panel-label">Diagnostics summary</div><strong id="diagnosticsSummary">Waiting</strong><span><span id="uptime">—</span> uptime · <span id="rssi">—</span></span></article><article class="panel compact-card"><div class="panel-label">OTA state</div><strong id="ota">Waiting</strong><span>Partition <span id="runningPartition">—</span></span></article></div><article class="panel quick-controls" aria-labelledby="quick-controls-title"><div><div class="panel-label" id="quick-controls-title">Quick controls</div><p>Jump directly to the controls you need.</p></div><div class="quick-grid"><button type="button" class="quick-button" data-jump="mode"><span aria-hidden="true">◷</span><span><strong>Mode</strong><small id="quickMode">—</small></span></button><button type="button" class="quick-button" data-jump="brightness"><span aria-hidden="true">☼</span><span><strong>Brightness</strong><small id="quickBrightness">—</small></span></button><button type="button" class="quick-button control" id="weatherRefreshQuick" data-control><span aria-hidden="true">☁</span><span><strong>Refresh weather</strong><small>Request current conditions</small></span></button><button type="button" class="quick-button" data-jump="network"><span aria-hidden="true">⌁</span><span><strong>Network settings</strong><small id="quickNetwork">—</small></span></button></div></article></section><section id="time-display" class="section" aria-labelledby="display-title"><div class="section-heading"><div><p class="eyebrow">Time &amp; Display</p><h2 id="display-title">Clock presentation</h2></div><p>Selected and effective modes remain separate while automatic NIGHT is active.</p></div><div class="two-column"><article class="panel form-panel"><div class="panel-label">Display controls</div><div class="form-grid"><label for="mode">Selected mode<select id="mode" class="control" data-control><option>CLASSIC</option><option>MINIMAL</option><option>NIGHT</option></select></label><label for="brightness">Normal brightness<select id="brightness" class="control" data-control><option>10</option><option>25</option><option>40</option><option>80</option></select></label></div><div class="readout-grid"><div><span>Effective mode</span><strong id="effectiveDisplay">—</strong></div><div><span>Manual override</span><strong id="override">—</strong></div></div></article><article class="panel form-panel"><div class="panel-label">Automatic NIGHT schedule</div><form id="autoForm"><label class="switch-row" for="autoEnabled"><span><strong>Automatic NIGHT</strong><small>Use NIGHT rendering during the saved interval</small></span><input id="autoEnabled" class="control" data-control type="checkbox"></label><div class="form-grid"><label for="autoStart">Starts<input id="autoStart" class="control" data-control type="time" required></label><label for="autoEnd">Ends<input id="autoEnd" class="control" data-control type="time" required></label></div><button class="button primary control" data-control type="submit">Save schedule</button></form></article></div></section><section id="network" class="section" aria-labelledby="network-title"><div class="section-heading"><div><p class="eyebrow">Network configuration</p><h2 id="network-title">DHCP &amp; static IPv4</h2></div><p>WiFiManager continues to own SSID and password provisioning.</p></div><div class="network-summary"><article class="panel compact-card"><div class="panel-label">Configured mode</div><strong id="networkConfiguredDetail">—</strong><span>Saved preference</span></article><article class="panel compact-card"><div class="panel-label">Effective mode</div><strong id="networkEffective">—</strong><span id="fallbackStatus">Fallback —</span></article><article class="panel compact-card"><div class="panel-label">Active address</div><strong id="networkActive">—</strong><span>Current local interface</span></article><article class="panel compact-card"><div class="panel-label">Static failures</div><strong id="networkFailures">—</strong><span>Confirmed failed boots</span></article></div><article class="panel form-panel"><div class="warning-banner" role="note"><strong>Address changes can make HALO CST unreachable.</strong><span>Save and Apply validates every field, responds to this browser, then reboots. Record the new address first.</span></div><div class="lazy-row"><p id="networkLoadState">Open this section to load the saved network configuration.</p><button type="button" class="button secondary" id="networkLoad">Load configuration</button></div><form id="networkForm"><label for="networkMode">Network mode<select id="networkMode" class="control" data-control disabled><option value="DHCP">DHCP</option><option value="STATIC">Static IP</option></select></label><div id="staticNetworkFields" class="form-grid network-fields"><label for="networkIp">Static IP<input id="networkIp" class="control" data-control type="text" inputmode="decimal" autocomplete="off" placeholder="192.168.1.50" disabled></label><label for="networkGateway">Gateway<input id="networkGateway" class="control" data-control type="text" inputmode="decimal" autocomplete="off" placeholder="192.168.1.1" disabled></label><label for="networkSubnet">Subnet mask<input id="networkSubnet" class="control" data-control type="text" inputmode="decimal" autocomplete="off" placeholder="255.255.255.0" disabled></label><label for="networkDns1">Primary DNS<input id="networkDns1" class="control" data-control type="text" inputmode="decimal" autocomplete="off" placeholder="1.1.1.1" disabled></label><label for="networkDns2">Secondary DNS <span>(optional)</span><input id="networkDns2" class="control" data-control type="text" inputmode="decimal" autocomplete="off" placeholder="8.8.8.8" disabled></label></div><div class="button-row"><button class="button primary control" data-control type="submit" disabled>Save and Apply</button><button id="networkReset" class="button danger control" data-control type="button" disabled>Reset to DHCP</button></div></form><details class="recovery-note"><summary>GPIO27 and USB recovery</summary><p>Within 30 seconds after startup completes, hold GPIO27 for 10 seconds to save DHCP mode and reboot. If the device remains unreachable, install known-good firmware over USB. Full-flash erase is the last resort because it removes Wi-Fi credentials and Preferences.</p></details></article></section><section id="weather" class="section" aria-labelledby="weather-title"><div class="section-heading"><div><p class="eyebrow">Weather</p><h2 id="weather-title">Current conditions</h2></div><p>Keyless HTTPS data from Open-Meteo; cached results survive refresh failures.</p></div><article class="panel weather-detail"><div class="weather-hero"><span class="weather-icon large" aria-hidden="true">☁</span><div><strong id="weatherTemperatureDetail">—</strong><span id="weatherConditionDetail">Waiting for data</span></div></div><div class="readout-grid weather-readouts"><div><span>Feels like</span><strong id="apparentTemperature">—</strong></div><div><span>Humidity</span><strong id="humidity">—</strong></div><div><span>Wind</span><strong id="windSpeed">—</strong></div><div><span>Last update</span><strong id="weatherLastUpdate">—</strong></div></div><div class="weather-state" id="weatherState">Waiting for current conditions</div><button id="weatherRefresh" class="button primary control" data-control type="button">Refresh weather</button></article></section><section id="diagnostics" class="section" aria-labelledby="diagnostics-title"><div class="section-heading"><div><p class="eyebrow">Diagnostics</p><h2 id="diagnostics-title">Device health</h2></div><p>Detailed diagnostics load only when requested and remain RAM-only.</p></div><details class="panel diagnostics-panel" id="diagnosticsPanel"><summary><span><strong>Open diagnostic snapshot</strong><small>Heap, reset, Wi-Fi, NTP, weather, OTA, firmware, and network data</small></span><span aria-hidden="true">＋</span></summary><div class="diagnostic-overview" id="diagnosticOverview"><p>Open or refresh to load current diagnostics.</p></div><pre class="diagnostics" id="diagnosticsOutput" tabindex="0">No snapshot loaded.</pre><div class="button-row diagnostic-actions"><button id="diagnosticsRefresh" class="button secondary" type="button">Refresh</button><button id="diagnosticsCopy" class="button secondary" type="button">Copy diagnostics</button><button id="diagnosticsDownload" class="button secondary" type="button">Download diagnostics JSON</button></div></details></section><section id="settings" class="section" aria-labelledby="settings-title"><div class="section-heading"><div><p class="eyebrow">Settings</p><h2 id="settings-title">System &amp; security</h2></div><p>HALO CST is designed for a trusted local network.</p></div><div class="two-column"><article class="panel system-card"><div class="panel-label">Firmware</div><div class="readout-grid"><div><span>Version</span><strong id="firmware">—</strong></div><div><span>OTA</span><strong id="otaDetail">—</strong></div><div><span>Free heap</span><strong id="freeHeap">—</strong></div><div><span>Minimum heap</span><strong id="minimumHeap">—</strong></div></div><a class="button secondary" href="https://github.com/Zivchaos/Halo_Clock" target="_blank" rel="noreferrer">Project repository</a></article><article class="panel system-card"><div class="panel-label">Trusted-LAN security</div><p>The Web UI and ArduinoOTA service are unauthenticated. Keep HALO CST on a trusted local network and do not expose ports 80 or 3232 to the public Internet.</p><button id="reboot" class="button danger control" data-control type="button">Reboot HALO CST</button></article></div></section></main></div><div id="feedback" class="toast" role="status" aria-live="polite" aria-atomic="true"></div><dialog id="rebootDialog"><form method="dialog"><div class="dialog-mark" aria-hidden="true">↻</div><h2>Reboot HALO CST?</h2><p>The clock and local interface will be unavailable briefly.</p><div class="dialog-actions"><button id="rebootCancel" class="button secondary" value="cancel">Cancel</button><button id="rebootConfirm" class="button danger control" data-control value="default" type="button">Reboot now</button></div></form></dialog><dialog id="networkDialog"><form method="dialog"><div class="dialog-mark warning" aria-hidden="true">!</div><h2>Confirm network change</h2><p id="networkConfirmText">The clock will reboot and its current address may stop responding.</p><div class="dialog-actions"><button id="networkCancel" class="button secondary" value="cancel">Cancel</button><button id="networkConfirm" class="button danger control" data-control value="default" type="button">Save, apply, and reboot</button></div></form></dialog></body></html>)HALOWEB";
    inline const char FAVICON_SVG[] PROGMEM = R"HALOWEB(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64"><path d="M20 3h24l17 17v24L44 61H20L3 44V20Z" fill="#0e0e0e" stroke="#22d3ee" stroke-width="4"/><rect x="18" y="22" width="28" height="16" rx="2" fill="#f2f2f2"/><circle cx="32" cy="46" r="3" fill="#22d3ee"/></svg>)HALOWEB";
}
