"use strict";

const $ = (id) => document.getElementById(id);
const POLL_INTERVAL_MS = 3000;
const state = {
  busy: false,
  diagnostics: null,
  networkLoaded: false,
  customizationLoaded: false,
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
  document.querySelectorAll("[data-custom-control]").forEach((control) => {
    control.disabled = state.busy || !state.customizationLoaded;
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
  const lastCheck = status.redAlertHasSuccessfulCheck ? ` Last successful check ${Number(status.redAlertLastCheckAgeSeconds || 0)}s ago.` : "";
  const failureSuffix = Number(status.redAlertFailureCount || 0) > 1 ? ` (${status.redAlertFailureCount} failed checks)` : "";
  const alertState = !status.redAlertEnabled ? "Disabled" : (status.redAlertActive ? `ACTIVE: ${status.redAlertAreas || "selected area"}` : (status.redAlertStale ? `Unavailable: ${status.redAlertError || "source stale"}${failureSuffix}.${lastCheck}` : (status.redAlertUpdating ? `Checking alert source.${lastCheck}` : `Monitoring selected areas.${lastCheck}`)));
  text("redAlertState", alertState);
  if (!state.busy) $("redAlertEnabled").checked = Boolean(status.redAlertEnabled);
  if (!state.busy) {
    $("mode").value = status.selectedMode;
    $("brightness").value = String(status.brightness);
    $("autoEnabled").checked = Boolean(status.autoNightEnabled);
    $("autoStart").value = status.autoNightStart || "";
    $("autoEnd").value = status.autoNightEnd || "";
  }
}

function selectedRedAlertAreas() {
  return Array.from($("redAlertLocations").selectedOptions).map((option) => option.value);
}

function redAlertSettingsData(enabled = $("redAlertEnabled").checked) {
  return { enabled: String(enabled), locations: selectedRedAlertAreas().join("\n"), relayUrl: $("redAlertRelayUrl").value.trim() };
}

function renderRedAlertAreas(filter = "") {
  const select = $("redAlertLocations");
  const selected = new Set(selectedRedAlertAreas());
  const query = filter.trim().toLocaleLowerCase();
  const catalog = window.RED_ALERT_AREAS || [];
  const selectedAreas = catalog.filter((area) => selected.has(area.he));
  const matches = selectedAreas.concat(catalog.filter((area) => !selected.has(area.he) && (!query || area.search.includes(query))).slice(0, Math.max(0, 250 - selectedAreas.length)));
  select.replaceChildren(...matches.map((area) => {
    const option = new Option(`${area.en} — ${area.he}`, area.he, false, selected.has(area.he));
    option.dir = "auto";
    return option;
  }));
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

async function loadCustomization() {
  try {
    const settings = await api("/api/customization");
    const calibration = settings.calibration || {};
    const location = settings.weatherLocation || {};
    const colors = settings.customColors || {};
    const redAlert = settings.redAlert || {};
    $("calibrationOffset").value = String(calibration.zeroOffset ?? 0);
    $("calibrationDirection").value = String(calibration.clockwise !== false);
    $("weatherLatitude").value = Number(location.latitude).toFixed(4);
    $("weatherLongitude").value = Number(location.longitude).toFixed(4);
    const colorFields = {
      colorHourTicks: "hourTicks", colorMinuteProgress: "minuteProgress",
      colorHourCenter: "hourCenter", colorHourSides: "hourSides",
      colorMinuteMarker: "minuteMarker", colorSecondMarker: "secondMarker"
    };
    Object.entries(colorFields).forEach(([id, key]) => {
      if (/^#[0-9a-f]{6}$/i.test(colors[key] || "")) $(id).value = colors[key];
    });
    $("redAlertEnabled").checked = Boolean(redAlert.enabled);
    $("redAlertRelayUrl").value = redAlert.relayUrl || "";
    const savedAreas = new Set((redAlert.locations || "").split("\n").filter(Boolean));
    renderRedAlertAreas();
    Array.from($("redAlertLocations").options).forEach((option) => { option.selected = savedAreas.has(option.value); });
    state.customizationLoaded = true;
    syncControlState();
  } catch (error) {
    state.customizationLoaded = false;
    syncControlState();
    showFeedback(`Customization unavailable: ${error.message}`, "error");
  }
}

function calibrationData(active) {
  return {
    zeroOffset: $("calibrationOffset").value,
    clockwise: $("calibrationDirection").value,
    active: String(active)
  };
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

$("customColorForm").addEventListener("submit", (event) => {
  event.preventDefault();
  void post("/api/custom-colors", {
    hourTicks: $("colorHourTicks").value,
    minuteProgress: $("colorMinuteProgress").value,
    hourCenter: $("colorHourCenter").value,
    hourSides: $("colorHourSides").value,
    minuteMarker: $("colorMinuteMarker").value,
    secondMarker: $("colorSecondMarker").value
  }, "CUSTOM colors saved");
});
$("calibrationTest").addEventListener("click", () => {
  void post("/api/calibration", calibrationData(true), "Calibration test active");
});
$("calibrationForm").addEventListener("submit", (event) => {
  event.preventDefault();
  void post("/api/calibration", calibrationData(false), "Ring calibration saved; clock restored");
});
$("weatherLocationForm").addEventListener("submit", (event) => {
  event.preventDefault();
  void post("/api/weather/location", {
    latitude: $("weatherLatitude").value,
    longitude: $("weatherLongitude").value
  }, "Weather location saved; refresh scheduled");
});
$("redAlertForm").addEventListener("submit", (event) => {
  event.preventDefault();
  void post("/api/red-alert", redAlertSettingsData(), "Red Alert settings saved");
});
$("redAlertEnabled").addEventListener("change", (event) => {
  void post("/api/red-alert", redAlertSettingsData(event.target.checked), event.target.checked ? "Red Alert monitoring enabled" : "Red Alert monitoring disabled");
});
$("redAlertSimulate").addEventListener("click", () => { void post("/api/red-alert/simulate", {}, "Red Alert visual test started"); });
$("redAlertFilter").addEventListener("input", (event) => renderRedAlertAreas(event.target.value));

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
renderRedAlertAreas();
void refreshStatus();
void loadCustomization();
window.setInterval(refreshStatus, POLL_INTERVAL_MS);

}

if (document.readyState === "loading") {
  document.addEventListener("DOMContentLoaded", initializeUi);
} else {
  initializeUi();
}
