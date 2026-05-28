/*
 * ============================================================
 *  Smart Tunnel Light Controller
 *  West Yangon Technological University — Dept. of Electronic Engineering
 *  II B.E. EC — April 2024
 *
 *  Hardware : ESP32 Dev Kit V1
 *  LEDs     : 3 × 0.5 mm LED (via 220 Ω current-limiting resistors)
 *  IDE      : Arduino IDE 2.x  |  Board: DOIT ESP32 DEVKIT V1
 * ============================================================
 *
 *  Web Endpoints
 *  -------------
 *  GET /          → serve control page
 *  GET /on        → all LEDs ON
 *  GET /off       → all LEDs OFF
 *  GET /toggle?led=<1|2|3>  → toggle individual LED
 *  GET /seq       → start sequential mode
 *  GET /seqstop   → stop  sequential mode
 *  GET /set?timer=<ms>      → set auto-off timer (ms)
 *  GET /set?seqDelay=<ms>   → set sequential delay (ms)
 *  GET /status    → JSON status of all LEDs + settings
 * ============================================================
 */

#include <WiFi.h>
#include <WebServer.h>

// ── Wi-Fi credentials ──────────────────────────────────────
// ACCESS POINT MODE — no router needed.
// Connect your phone/PC to this network, then open http://192.168.4.1
const char* AP_SSID     = "TunnelLight_AP";
const char* AP_PASSWORD = "tunnel123";   // min 8 chars; set "" for open network

// ── GPIO pin assignments ───────────────────────────────────
const int LED_PINS[3] = {2, 4, 5};      // LED1 → GPIO2, LED2 → GPIO4, LED3 → GPIO5

// ── Runtime state ─────────────────────────────────────────
bool      ledState[3]     = {false, false, false};
bool      seqRunning      = false;
int       seqIndex        = 0;
unsigned long autoOffMs   = 5000;        // default auto-off: 5 s
unsigned long seqDelayMs  = 800;         // default sequential delay: 0.8 s
unsigned long lastSeqTime = 0;
unsigned long ledOnTime[3]= {0, 0, 0};

WebServer server(80);

// ── HTML page (stored in program memory) ──────────────────
// Kept minimal; a full styled page is in /web/index.html for reference.
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Tunnel Light Controller</title>
<style>
  :root{--bg:#0a0e1a;--card:#131929;--accent:#00e5ff;--on:#00e5ff;--off:#263040;--text:#e0eaf5}
  *{box-sizing:border-box;margin:0;padding:0}
  body{background:var(--bg);color:var(--text);font-family:'Segoe UI',sans-serif;min-height:100vh;display:flex;flex-direction:column;align-items:center;padding:24px 16px}
  h1{font-size:1.6rem;letter-spacing:.1em;text-transform:uppercase;margin-bottom:4px;color:var(--accent)}
  .sub{font-size:.8rem;opacity:.5;margin-bottom:32px}
  .grid{display:grid;grid-template-columns:repeat(3,1fr);gap:16px;width:100%;max-width:460px;margin-bottom:24px}
  .led-card{background:var(--card);border-radius:16px;padding:20px 12px;text-align:center;border:1px solid #1e2d40;transition:.3s}
  .led-card.on{border-color:var(--accent);box-shadow:0 0 20px #00e5ff33}
  .dot{width:32px;height:32px;border-radius:50%;margin:0 auto 12px;background:#1e2d40;transition:.4s}
  .led-card.on .dot{background:var(--accent);box-shadow:0 0 14px var(--accent)}
  .led-label{font-size:.75rem;letter-spacing:.08em;opacity:.6;margin-bottom:12px;text-transform:uppercase}
  button{cursor:pointer;border:none;border-radius:8px;padding:8px 0;width:100%;font-size:.8rem;font-weight:600;letter-spacing:.05em;transition:.2s}
  .btn-on{background:var(--accent);color:#0a0e1a}
  .btn-off{background:#1e2d40;color:var(--text)}
  .btn-on:hover{opacity:.85} .btn-off:hover{background:#263040}
  .controls{display:flex;flex-direction:column;gap:12px;width:100%;max-width:460px}
  .row{display:grid;grid-template-columns:1fr 1fr;gap:12px}
  .big-btn{padding:14px;border-radius:12px;font-size:.9rem;font-weight:700;letter-spacing:.08em}
  .btn-all-on{background:var(--accent);color:#0a0e1a}
  .btn-all-off{background:#1e2d40;color:var(--text)}
  .btn-seq{background:linear-gradient(135deg,#0050ff,#00e5ff);color:#fff}
  .btn-stop{background:#ff3860;color:#fff}
  .settings{background:var(--card);border-radius:16px;padding:20px;border:1px solid #1e2d40;margin-top:8px}
  .settings h3{font-size:.8rem;letter-spacing:.1em;text-transform:uppercase;opacity:.5;margin-bottom:16px}
  label{font-size:.78rem;opacity:.7;display:block;margin-bottom:4px}
  .row-s{display:flex;align-items:center;gap:10px;margin-bottom:12px}
  input[type=range]{flex:1;accent-color:var(--accent)}
  .val{font-size:.8rem;color:var(--accent);min-width:42px;text-align:right}
  .status-bar{font-size:.72rem;opacity:.4;margin-top:20px}
</style>
</head>
<body>
<h1>Tunnel Light</h1>
<p class="sub">ESP32 Web Controller</p>

<div class="grid" id="leds">
  <div class="led-card" id="c1"><div class="dot"></div><div class="led-label">LED 1</div>
    <button class="btn-on" onclick="toggle(1)">ON</button>
    <button class="btn-off" style="margin-top:6px" onclick="cmd('/off?led=1')">OFF</button>
  </div>
  <div class="led-card" id="c2"><div class="dot"></div><div class="led-label">LED 2</div>
    <button class="btn-on" onclick="toggle(2)">ON</button>
    <button class="btn-off" style="margin-top:6px" onclick="cmd('/off?led=2')">OFF</button>
  </div>
  <div class="led-card" id="c3"><div class="dot"></div><div class="led-label">LED 3</div>
    <button class="btn-on" onclick="toggle(3)">ON</button>
    <button class="btn-off" style="margin-top:6px" onclick="cmd('/off?led=3')">OFF</button>
  </div>
</div>

<div class="controls">
  <div class="row">
    <button class="big-btn btn-all-on" onclick="cmd('/on')">ALL ON</button>
    <button class="big-btn btn-all-off" onclick="cmd('/off')">ALL OFF</button>
  </div>
  <div class="row">
    <button class="big-btn btn-seq" onclick="cmd('/seq')">▶ SEQUENTIAL</button>
    <button class="big-btn btn-stop" onclick="cmd('/seqstop')">■ STOP</button>
  </div>

  <div class="settings">
    <h3>Settings</h3>
    <label>Auto-off timer</label>
    <div class="row-s">
      <input type="range" id="timerSlider" min="1000" max="30000" step="500" value="5000"
             oninput="document.getElementById('timerVal').textContent=(this.value/1000).toFixed(1)+'s'"
             onchange="cmd('/set?timer='+this.value)">
      <span class="val" id="timerVal">5.0s</span>
    </div>
    <label>Sequential delay</label>
    <div class="row-s">
      <input type="range" id="seqSlider" min="200" max="3000" step="100" value="800"
             oninput="document.getElementById('seqVal').textContent=(this.value/1000).toFixed(1)+'s'"
             onchange="cmd('/set?seqDelay='+this.value)">
      <span class="val" id="seqVal">0.8s</span>
    </div>
  </div>
</div>

<p class="status-bar" id="statusBar">Connecting…</p>

<script>
function cmd(url){fetch(url).then(()=>poll())}
function toggle(n){fetch('/toggle?led='+n).then(()=>poll())}
function poll(){
  fetch('/status').then(r=>r.json()).then(d=>{
    [1,2,3].forEach(i=>{
      const on=d['led'+i];
      document.getElementById('c'+i).classList.toggle('on',on);
    });
    document.getElementById('statusBar').textContent=
      'Auto-off: '+(d.timer/1000).toFixed(1)+'s  |  Seq delay: '+(d.seqDelay/1000).toFixed(1)+'s  |  Sequential: '+(d.seq?'ON':'OFF');
  });
}
setInterval(poll,2000);
poll();
</script>
</body>
</html>
)rawliteral";

// ── Helpers ───────────────────────────────────────────────
void applyLed(int idx, bool on) {
  ledState[idx] = on;
  digitalWrite(LED_PINS[idx], on ? HIGH : LOW);
  if (on) ledOnTime[idx] = millis();
}

void handleStatus() {
  String json = "{";
  for (int i = 0; i < 3; i++) {
    json += "\"led" + String(i + 1) + "\":" + (ledState[i] ? "true" : "false") + ",";
  }
  json += "\"timer\":" + String(autoOffMs) + ",";
  json += "\"seqDelay\":" + String(seqDelayMs) + ",";
  json += "\"seq\":" + String(seqRunning ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}

// ── Route handlers ────────────────────────────────────────
void handleRoot()    { server.send_P(200, "text/html", INDEX_HTML); }

void handleOn() {
  int led = server.hasArg("led") ? server.arg("led").toInt() - 1 : -1;
  if (led >= 0 && led < 3) applyLed(led, true);
  else for (int i = 0; i < 3; i++) applyLed(i, true);
  server.send(200, "text/plain", "OK");
}

void handleOff() {
  int led = server.hasArg("led") ? server.arg("led").toInt() - 1 : -1;
  if (led >= 0 && led < 3) applyLed(led, false);
  else { for (int i = 0; i < 3; i++) applyLed(i, false); seqRunning = false; }
  server.send(200, "text/plain", "OK");
}

void handleToggle() {
  if (!server.hasArg("led")) { server.send(400, "text/plain", "Missing led param"); return; }
  int led = server.arg("led").toInt() - 1;
  if (led < 0 || led >= 3) { server.send(400, "text/plain", "led must be 1-3"); return; }
  applyLed(led, !ledState[led]);
  server.send(200, "text/plain", "OK");
}

void handleSeq()     { seqRunning = true;  seqIndex = 0; server.send(200, "text/plain", "OK"); }
void handleSeqStop() { seqRunning = false; for (int i=0;i<3;i++) applyLed(i,false); server.send(200,"text/plain","OK"); }

void handleSet() {
  if (server.hasArg("timer"))    autoOffMs  = server.arg("timer").toInt();
  if (server.hasArg("seqDelay")) seqDelayMs = server.arg("seqDelay").toInt();
  server.send(200, "text/plain", "OK");
}

// ── Setup ─────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 3; i++) { pinMode(LED_PINS[i], OUTPUT); digitalWrite(LED_PINS[i], LOW); }

  // Start Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.println("\n=== Tunnel Light Controller ===");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Register routes
  server.on("/",        handleRoot);
  server.on("/on",      handleOn);
  server.on("/off",     handleOff);
  server.on("/toggle",  handleToggle);
  server.on("/seq",     handleSeq);
  server.on("/seqstop", handleSeqStop);
  server.on("/set",     handleSet);
  server.on("/status",  handleStatus);
  server.begin();
  Serial.println("Web server started. Open http://192.168.4.1 in your browser.");
}

// ── Loop ──────────────────────────────────────────────────
void loop() {
  server.handleClient();

  unsigned long now = millis();

  // Auto-off: turn off individual LEDs after timeout
  for (int i = 0; i < 3; i++) {
    if (ledState[i] && !seqRunning && (now - ledOnTime[i] >= autoOffMs)) {
      applyLed(i, false);
    }
  }

  // Sequential mode
  if (seqRunning && (now - lastSeqTime >= seqDelayMs)) {
    for (int i = 0; i < 3; i++) applyLed(i, false);   // turn all off
    applyLed(seqIndex, true);                           // light current
    seqIndex = (seqIndex + 1) % 3;
    lastSeqTime = now;
  }
}
