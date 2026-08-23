#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_APDS9960.h>

// ---- ADDED: WiFi + Web Server ----
#include <WiFi.h>
#include <WebServer.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define BUZZER_PIN 25

// Hardware Instances
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_BMP085 bmp;
Adafruit_MPU6050 mpu;
Adafruit_APDS9960 apds;

// ---- ADDED: WiFi credentials (fill these in later) ----
const char* WIFI_SSID     = "ENTER THE SSID";
const char* WIFI_PASSWORD = "ENTER THE PASSWORD";

// ---- ADDED: Web server on port 80 ----
WebServer server(80);

// ---- ADDED: Shared state, updated each loop(), read by the /data endpoint ----
float   g_pressureHpa   = 0;
float   g_accelX        = 0;
float   g_accelY        = 0;
float   g_accelZ        = 0;
float   g_tempC         = 0;
String  g_status        = "Monitoring";
String  g_timer         = "OK";
bool    g_buzzerActive  = false;
String  g_lastGesture   = "";

// ---- ADDED: Embedded website (HTML + CSS + JS in one string, stored in flash) ----
const char INDEX_HTML[] PROGMEM = R"HTMLPAGE(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1, user-scalable=no">
<title>SENTRY // Sensor Console</title>
<style>
  :root{
    --bg:        #090c0b;
    --panel-fill:#10151399;
    --line:      #23302a;
    --phosphor:  #3dffa0;
    --phosphor-dim: #1f6b48;
    --amber:     #ffb627;
    --red:       #ff5c5c;
    --text:      #cdeadd;
    --text-dim:  #6f8b7d;
    --mono: ui-monospace, "SF Mono", "Cascadia Code", "JetBrains Mono", Consolas, "Courier New", monospace;
    --sans: -apple-system, "Segoe UI", "Inter", "Helvetica Neue", Arial, sans-serif;
  }
  *{ box-sizing:border-box; margin:0; padding:0; }
  html,body{ background: var(--bg); color: var(--text); font-family: var(--sans); min-height:100vh; width:100%; }
  body{
    background-image:
      radial-gradient(circle at 15% 0%, rgba(61,255,160,0.06), transparent 45%),
      radial-gradient(circle at 85% 100%, rgba(255,182,39,0.04), transparent 45%);
    padding: 18px 16px 48px;
  }
  body::before{
    content:"";
    position:fixed; inset:0;
    pointer-events:none;
    background: repeating-linear-gradient(to bottom, rgba(255,255,255,0.018) 0px, rgba(255,255,255,0.018) 1px, transparent 1px, transparent 3px);
    z-index: 50;
  }
  .wrap{ max-width: 720px; margin: 0 auto; }
  header{ display:flex; align-items:flex-end; justify-content:space-between; padding-bottom: 14px; border-bottom: 1px solid var(--line); margin-bottom: 20px; }
  .brand-eyebrow{ font-family: var(--mono); font-size: 11px; letter-spacing: 3px; color: var(--text-dim); text-transform: uppercase; }
  h1{ font-family: var(--mono); font-size: 22px; letter-spacing: 1px; color: var(--text); font-weight: 600; margin-top: 4px; }
  h1 span{ color: var(--phosphor); }
  .conn{ display:flex; align-items:center; gap:8px; font-family: var(--mono); font-size: 11px; letter-spacing: 1px; color: var(--text-dim); text-transform: uppercase; }
  .dot{ width:8px; height:8px; border-radius:50%; background: var(--red); box-shadow: 0 0 6px var(--red); transition: background .3s, box-shadow .3s; }
  .dot.live{ background: var(--phosphor); box-shadow: 0 0 8px var(--phosphor); animation: pulse 2s ease-in-out infinite; }
  @keyframes pulse{ 0%,100%{ opacity:1; } 50%{ opacity:.45; } }
  .oled-frame{ background: linear-gradient(180deg,#1a1f1d,#0d100f); border: 1px solid #2b332e; border-radius: 14px; padding: 14px; box-shadow: 0 10px 30px rgba(0,0,0,0.5), inset 0 0 0 1px rgba(255,255,255,0.02); margin-bottom: 22px; }
  .oled-frame .screw-row{ display:flex; justify-content:space-between; padding: 0 2px 10px; }
  .screw-row span{ font-family: var(--mono); font-size:9px; letter-spacing:2px; color: var(--text-dim); text-transform:uppercase; }
  .oled-screen{ background: #010302; border-radius: 4px; padding: 16px 18px; position: relative; overflow:hidden; box-shadow: inset 0 0 24px rgba(61,255,160,0.05); }
  .oled-screen::after{ content:""; position:absolute; inset:0; background: repeating-linear-gradient(to bottom, rgba(61,255,160,0.05) 0px, transparent 1px, transparent 2px); pointer-events:none; }
  .oled-line{ font-family: var(--mono); font-size: 15px; line-height: 1.9; color: var(--phosphor); text-shadow: 0 0 6px rgba(61,255,160,0.65), 0 0 14px rgba(61,255,160,0.25); white-space: pre; }
  .oled-line .lbl{ color: var(--phosphor-dim); text-shadow:none; }
  .oled-caption{ display:flex; justify-content:space-between; margin-top:10px; font-family: var(--mono); font-size: 10px; letter-spacing:1.5px; color: var(--text-dim); text-transform:uppercase; }
  .grid{ display:grid; grid-template-columns: repeat(2, 1fr); gap: 12px; }
  @media (max-width:480px){ .grid{ grid-template-columns: 1fr; } }
  .panel{ background: var(--panel-fill); border: 1px solid var(--line); border-radius: 10px; padding: 14px 16px; position: relative; }
  .panel-eyebrow{ font-family: var(--mono); font-size: 10px; letter-spacing: 2px; color: var(--text-dim); text-transform: uppercase; display:flex; justify-content:space-between; align-items:center; margin-bottom: 8px; }
  .panel-eyebrow .tag{ font-size:9px; padding: 2px 6px; border: 1px solid var(--line); border-radius: 3px; color: var(--text-dim); }
  .panel-value{ font-family: var(--mono); font-size: 26px; color: var(--text); letter-spacing: 0.5px; }
  .panel-value small{ font-size: 13px; color: var(--text-dim); margin-left: 4px; }
  .panel-sub{ margin-top: 6px; font-size: 11.5px; color: var(--text-dim); font-family: var(--mono); }
  .panel.wide{ grid-column: 1 / -1; }
  .spark{ display:flex; align-items:flex-end; gap:2px; height: 34px; margin-top: 10px; }
  .spark i{ flex:1; background: linear-gradient(180deg, var(--phosphor), var(--phosphor-dim)); opacity:.55; border-radius: 1px 1px 0 0; transition: height .3s ease; }
  .axis-row{ display:flex; flex-direction:column; gap:8px; margin-top:10px; }
  .axis{ display:grid; grid-template-columns: 20px 1fr 56px; align-items:center; gap: 8px; font-family: var(--mono); font-size: 11px; }
  .axis .ax-label{ color: var(--text-dim); }
  .axis .ax-track{ height: 6px; background: #171d1a; border-radius: 3px; overflow:hidden; position:relative; border: 1px solid var(--line); }
  .axis .ax-fill{ position:absolute; top:0; bottom:0; left:50%; background: var(--phosphor); box-shadow: 0 0 6px rgba(61,255,160,0.5); transition: width .3s ease, left .3s ease; }
  .axis .ax-val{ text-align:right; color: var(--text); }
  .badge{ display:inline-flex; align-items:center; gap:6px; font-family: var(--mono); font-size: 12px; letter-spacing: 1px; padding: 5px 10px; border-radius: 20px; border: 1px solid var(--line); text-transform: uppercase; }
  .badge.ok{ color: var(--phosphor); border-color: rgba(61,255,160,0.35); }
  .badge.warn{ color: var(--amber); border-color: rgba(255,182,39,0.35); }
  .badge.alert{ color: var(--red); border-color: rgba(255,92,92,0.4); }
  .badge .b-dot{ width:6px; height:6px; border-radius:50%; background: currentColor; box-shadow: 0 0 6px currentColor; }
  .row-between{ display:flex; align-items:center; justify-content:space-between; flex-wrap:wrap; gap:10px; }
  .snooze-hint{ font-size: 11px; color: var(--text-dim); font-family: var(--mono); margin-top: 8px; }
  footer{ margin-top: 24px; text-align:center; font-family: var(--mono); font-size: 10px; letter-spacing: 2px; color: var(--text-dim); text-transform: uppercase; }
  .demo-banner{ display:none; font-family: var(--mono); font-size: 11px; letter-spacing: 1px; color: var(--amber); border: 1px solid rgba(255,182,39,0.35); background: rgba(255,182,39,0.06); border-radius: 8px; padding: 8px 12px; margin-bottom: 16px; text-transform: uppercase; }
  .demo-banner.show{ display:block; }
</style>
</head>
<body>
<div class="wrap">
  <header>
    <div>
      <h1>RespiWave</h1>
    </div>
    <div class="conn">
      <span class="dot" id="connDot"></span>
      <span id="connLabel">CONNECTING</span>
    </div>
  </header>

  <div class="demo-banner" id="demoBanner">device not responding on /data</div>

  <div class="oled-frame">
    <div class="screw-row"><span>SSD1306 . 128x64</span><span id="oledClock">00:00</span></div>
    <div class="oled-screen">
      <div class="oled-line"><span class="lbl">Press: </span><span id="oledPress">--.--</span><span class="lbl"> hPa</span></div>
      <div class="oled-line"><span class="lbl">X: </span><span id="oledX">--.--</span><span class="lbl">  Y: </span><span id="oledY">--.--</span></div>
      <div class="oled-line"><span class="lbl">Status: </span><span id="oledStatus">Monitoring</span></div>
      <div class="oled-line"><span class="lbl">Timer: </span><span id="oledTimer">OK</span></div>
    </div>
    <div class="oled-caption">
      <span>Live mirror of device display</span>
      <span id="lastUpdate">updated --</span>
    </div>
  </div>

  <div class="grid">
    <div class="panel">
      <div class="panel-eyebrow"><span>Barometric Pressure</span><span class="tag">BMP180</span></div>
      <div class="panel-value" id="pressureVal">---.-<small>hPa</small></div>
      <div class="panel-sub" id="pressureAlt">est. altitude --- m</div>
      <div class="spark" id="pressureSpark"></div>
    </div>

    <div class="panel">
      <div class="panel-eyebrow"><span>Module Temp</span><span class="tag">MPU6050</span></div>
      <div class="panel-value" id="tempVal">--.-<small>C</small></div>
      <div class="panel-sub">onboard die temperature</div>
    </div>

    <div class="panel wide">
      <div class="panel-eyebrow"><span>Acceleration Vector</span><span class="tag">MPU6050</span></div>
      <div class="axis-row">
        <div class="axis"><span class="ax-label">X</span><span class="ax-track"><span class="ax-fill" id="axFillX"></span></span><span class="ax-val" id="axValX">0.00</span></div>
        <div class="axis"><span class="ax-label">Y</span><span class="ax-track"><span class="ax-fill" id="axFillY"></span></span><span class="ax-val" id="axValY">0.00</span></div>
        <div class="axis"><span class="ax-label">Z</span><span class="ax-track"><span class="ax-fill" id="axFillZ"></span></span><span class="ax-val" id="axValZ">0.00</span></div>
      </div>
      <div class="panel-sub" style="margin-top:12px;">values in m/s^2 . centered on 0</div>
    </div>

    <div class="panel">
      <div class="panel-eyebrow"><span>System Status</span></div>
      <span class="badge ok" id="statusBadge"><span class="b-dot"></span><span id="statusText">Monitoring</span></span>
      <div class="panel-sub" style="margin-top:10px;">Timer: <span id="timerText">OK</span></div>
    </div>

    <div class="panel">
      <div class="panel-eyebrow"><span>Alarm / Buzzer</span><span class="tag">APDS9960</span></div>
      <div class="row-between">
        <span class="badge" id="buzzerBadge"><span class="b-dot"></span><span id="buzzerText">Idle</span></span>
      </div>
      <div class="snooze-hint">Swipe up/down over sensor to snooze</div>
    </div>
  </div>

  <footer>polling /data every 500ms</footer>
</div>

<script>
(function(){
  "use strict";
  var POLL_MS = 500;
  var DATA_ENDPOINT = "/data";
  var pressureHistory = [];
  var HISTORY_LEN = 24;

  var el = {
    connDot: document.getElementById('connDot'),
    connLabel: document.getElementById('connLabel'),
    demoBanner: document.getElementById('demoBanner'),
    oledPress: document.getElementById('oledPress'),
    oledX: document.getElementById('oledX'),
    oledY: document.getElementById('oledY'),
    oledStatus: document.getElementById('oledStatus'),
    oledTimer: document.getElementById('oledTimer'),
    oledClock: document.getElementById('oledClock'),
    lastUpdate: document.getElementById('lastUpdate'),
    pressureVal: document.getElementById('pressureVal'),
    pressureAlt: document.getElementById('pressureAlt'),
    pressureSpark: document.getElementById('pressureSpark'),
    tempVal: document.getElementById('tempVal'),
    axFillX: document.getElementById('axFillX'),
    axFillY: document.getElementById('axFillY'),
    axFillZ: document.getElementById('axFillZ'),
    axValX: document.getElementById('axValX'),
    axValY: document.getElementById('axValY'),
    axValZ: document.getElementById('axValZ'),
    statusBadge: document.getElementById('statusBadge'),
    statusText: document.getElementById('statusText'),
    timerText: document.getElementById('timerText'),
    buzzerBadge: document.getElementById('buzzerBadge'),
    buzzerText: document.getElementById('buzzerText')
  };

  for (var i = 0; i < HISTORY_LEN; i++) {
    var bar = document.createElement('i');
    bar.style.height = '4px';
    el.pressureSpark.appendChild(bar);
  }

  function fmt(n, d) {
    if (n === null || n === undefined || isNaN(n)) return '--';
    return Number(n).toFixed(d === undefined ? 1 : d);
  }
  function clamp(n, min, max) { return Math.max(min, Math.min(max, n)); }

  function setAxis(fillEl, valEl, value, range) {
    range = range || 12;
    var pct = clamp((value / range) * 50, -50, 50);
    if (pct >= 0) { fillEl.style.left = '50%'; fillEl.style.width = pct + '%'; }
    else { fillEl.style.left = (50 + pct) + '%'; fillEl.style.width = (-pct) + '%'; }
    valEl.textContent = fmt(value, 2);
  }

  function updateSpark(pressureHpa) {
    pressureHistory.push(pressureHpa);
    if (pressureHistory.length > HISTORY_LEN) pressureHistory.shift();
    var min = Math.min.apply(null, pressureHistory);
    var max = Math.max.apply(null, pressureHistory);
    var span = (max - min) || 1;
    var bars = el.pressureSpark.children;
    for (var i = 0; i < bars.length; i++) {
      var v = pressureHistory[i];
      if (v === undefined) { bars[i].style.height = '4px'; continue; }
      var h = 4 + ((v - min) / span) * 28;
      bars[i].style.height = h + 'px';
    }
  }

  function setBadge(badgeEl, textEl, text, level) {
    badgeEl.classList.remove('ok', 'warn', 'alert');
    badgeEl.classList.add(level);
    textEl.textContent = text;
  }

  function render(data) {
    var pressureHpa = data.pressure;
    var ax = data.accelX, ay = data.accelY, az = data.accelZ;
    var temp = data.temp;
    var status = data.status || 'Monitoring';
    var timer = data.timer || 'OK';
    var buzzer = !!data.buzzerActive;
    var gesture = data.lastGesture || null;

    el.oledPress.textContent = fmt(pressureHpa, 2);
    el.oledX.textContent = fmt(ax, 2);
    el.oledY.textContent = fmt(ay, 2);
    el.oledStatus.textContent = status;
    el.oledTimer.textContent = timer;

    el.pressureVal.innerHTML = fmt(pressureHpa, 1) + '<small>hPa</small>';
    var altitude = 44330 * (1 - Math.pow((pressureHpa || 1013.25) / 1013.25, 1 / 5.255));
    el.pressureAlt.textContent = 'est. altitude ' + fmt(altitude, 0) + ' m';
    updateSpark(pressureHpa || 0);

    el.tempVal.innerHTML = fmt(temp, 1) + '<small>C</small>';

    setAxis(el.axFillX, el.axValX, ax || 0);
    setAxis(el.axFillY, el.axValY, ay || 0);
    setAxis(el.axFillZ, el.axValZ, az || 0);

    var lvl = /alert|fault|error/i.test(status) ? 'alert' : /warn/i.test(status) ? 'warn' : 'ok';
    setBadge(el.statusBadge, el.statusText, status, lvl);
    el.timerText.textContent = timer;

    if (buzzer) { setBadge(el.buzzerBadge, el.buzzerText, 'Alarm Active', 'alert'); }
    else if (gesture) { setBadge(el.buzzerBadge, el.buzzerText, 'Snoozed (' + gesture + ')', 'warn'); }
    else { setBadge(el.buzzerBadge, el.buzzerText, 'Idle', 'ok'); }

    el.lastUpdate.textContent = 'updated ' + new Date().toLocaleTimeString();
  }

  function setConnected(isLive) {
    el.connDot.classList.toggle('live', isLive);
    el.connLabel.textContent = isLive ? 'LIVE' : 'OFFLINE';
    el.demoBanner.classList.toggle('show', !isLive);
  }

  function poll() {
    fetch(DATA_ENDPOINT, { cache: 'no-store' })
      .then(function (res) { if (!res.ok) throw new Error('bad response'); return res.json(); })
      .then(function (data) { setConnected(true); render(data); })
      .catch(function () { setConnected(false); });
  }

  function tickClock() {
    el.oledClock.textContent = new Date().toLocaleTimeString([], {hour:'2-digit', minute:'2-digit'});
  }

  poll();
  tickClock();
  setInterval(poll, POLL_MS);
  setInterval(tickClock, 1000);
})();
</script>
</body>
</html>
)HTMLPAGE";

// ---- ADDED: Serve the embedded page ----
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

// ---- ADDED: Serve current sensor values as JSON for the page to poll ----
void handleData() {
  String json = "{";
  json += "\"pressure\":" + String(g_pressureHpa, 2) + ",";
  json += "\"accelX\":" + String(g_accelX, 3) + ",";
  json += "\"accelY\":" + String(g_accelY, 3) + ",";
  json += "\"accelZ\":" + String(g_accelZ, 3) + ",";
  json += "\"temp\":" + String(g_tempC, 2) + ",";
  json += "\"status\":\"" + g_status + "\",";
  json += "\"timer\":\"" + g_timer + "\",";
  json += "\"buzzerActive\":" + String(g_buzzerActive ? "true" : "false") + ",";
  json += "\"lastGesture\":" + (g_lastGesture.length() ? ("\"" + g_lastGesture + "\"") : "null");
  json += "}";
  server.send(200, "application/json", json);
}

// Hardware Instances
// (declared above, unchanged)

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // I2C Pins SDA=21, SCL=22
  pinMode(BUZZER_PIN, OUTPUT);
  // Initialize OLED Display (0x3C)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED init failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println("Initializing...");
  display.display();
  // Initialize BMP180 Pressure Sensor (0x77)
  if (!bmp.begin()) {
    Serial.println("BMP180 not found");
  }
  // Initialize MPU6050 Gyro/Accel (0x69)
  if (!mpu.begin(0x69)) {
    Serial.println("MPU6050 not found");
  }
  // Initialize APDS9960 Gesture Sensor (0x39)
  if (!apds.begin()) {
    Serial.println("APDS9960 not found");
  }
  apds.enableProximity(true);
  apds.enableGesture(true);

  // ---- ADDED: connect to WiFi and start the web server ----
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 15000) {
    delay(300);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Connected. IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("WiFi not connected — web console will be unreachable until it connects.");
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  // ---- ADDED: let the web server handle incoming requests ----
  server.handleClient();

  // Read Pressure Data
  float pressure = bmp.readPressure();
  // Read Motion/Orientation Data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  // Read Gesture for Touchless Alarm Snooze
  if (apds.gestureValid()) {
    uint8_t gesture = apds.readGesture();
    if (gesture == APDS9960_DOWN || gesture == APDS9960_UP) {
      digitalWrite(BUZZER_PIN, LOW); // Snooze Alarm
      // ---- ADDED: record gesture for the web console ----
      g_lastGesture = (gesture == APDS9960_DOWN) ? "DOWN" : "UP";
    }
  }
  // Display Status on OLED Screen
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Press: "); 
  display.print(pressure / 100.0); 
  display.println(" hPa");
  display.setCursor(0, 16);
  display.print("X: "); display.print(a.acceleration.x);
  display.print(" Y: "); display.print(a.acceleration.y);
  display.setCursor(0, 32);
  display.println("Status: Monitoring");
  display.setCursor(0, 48);
  display.println("Timer: OK");
  display.display();

  // ---- ADDED: mirror the same values into the shared state for the website ----
  g_pressureHpa  = pressure / 100.0;
  g_accelX       = a.acceleration.x;
  g_accelY       = a.acceleration.y;
  g_accelZ       = a.acceleration.z;
  g_tempC        = temp.temperature;
  g_status       = "Monitoring";
  g_timer        = "OK";
  g_buzzerActive = (digitalRead(BUZZER_PIN) == HIGH);

  delay(500);
}
