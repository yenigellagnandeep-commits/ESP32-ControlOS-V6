#include <WiFi.h>
#include <WebServer.h>
#include <string.h>

const char* ssid = "ESP32_Control_OS";
// Set a private WPA2 passphrase before uploading; the placeholder blocks insecure startup.
const char* password = "CHANGE_ME_PRIVATE_PASSWORD";

WebServer server(80);
struct LogEntry
{
  unsigned long time;
  int heap;
  int clients;
};

const int LOG_CAPACITY = 100;
LogEntry logs[LOG_CAPACITY];

int nextLogIndex = 0;
int logCount = 0;

const int LED_PIN = 2;

bool isValidAccessPointPassword()
{
  const size_t passwordLength = strlen(password);
  return strcmp(password, "CHANGE_ME_PRIVATE_PASSWORD") != 0 &&
         passwordLength >= 8 && passwordLength <= 63;
}

String escapeHtml(String value)
{
  value.replace("&", "&amp;");
  value.replace("<", "&lt;");
  value.replace(">", "&gt;");
  value.replace("\"", "&quot;");
  value.replace("'", "&#39;");
  return value;
}

String getHeader()
 {

String html = R"rawliteral(

<!DOCTYPE html>
<html>

<head>

<meta charset="UTF-8">

<meta name="viewport" content="width=device-width, initial-scale=1">

<title>ESP32 ControlOS V6</title>

<style>

*{
margin:0;
padding:0;
box-sizing:border-box;
font-family:Segoe UI,Arial,sans-serif;
}

body{
background:#0f172a;
color:white;
}

.header{

background:linear-gradient(90deg,#2563eb,#06b6d4);
padding:22px;
text-align:center;
font-size:30px;
font-weight:bold;
box-shadow:0 4px 20px rgba(0,0,0,.3);

}

.nav{

background:#111827;
padding:18px;
text-align:center;
border-bottom:1px solid #334155;
box-shadow:0 4px 20px rgba(0,0,0,.3);

}

.nav a{

color:white;
text-decoration:none;
margin:6px;
padding:12px 18px;
display:inline-block;
border-radius:12px;
background:#1e293b;
transition:0.3s;
font-weight:bold;
box-shadow:0 0 10px rgba(37,99,235,.15);

}

.nav a:hover{

background:#2563eb;
transform:translateY(-2px);
box-shadow:0 0 20px rgba(37,99,235,.5);

}

.container{

width:95%;
max-width:1100px;
margin:auto;
padding:20px;

}

.card{

background:#1e293b;
padding:20px;
border-radius:20px;
margin-bottom:20px;
box-shadow:0 0 25px rgba(37,99,235,.35);
transition:0.3s;

}
.card:hover{

transform:translateY(-4px);
box-shadow:0 0 35px rgba(37,99,235,.55);
}

.grid{

display:grid;
grid-template-columns:repeat(auto-fit,minmax(220px,1fr));
gap:20px;

}

.stat{

font-size:32px;
font-weight:bold;
margin-top:10px;
color:#38bdf8;

}

.label{
color:#94a3b8;
font-size:14px;

}

.green{
background:#16a34a;
color:white;
}

.red{
background:#dc2626;
color:white;
}

.blue{
background:#2563eb;
color:white;
}

button{

padding:12px;
width:180px;
border:none;
border-radius:10px;
margin-top:10px;
cursor:pointer;
font-size:16px;

}

.action-link {
display:inline-block;
min-width:180px;
padding:12px;
margin-top:10px;
border-radius:10px;
color:white;
text-align:center;
text-decoration:none;
font-weight:bold;
}

.action-link:hover {
filter:brightness(1.12);
}

button:focus-visible,
.nav a:focus-visible,
.action-link:focus-visible {
outline:3px solid #facc15;
outline-offset:3px;
}

.footer{

text-align:center;
padding:20px;
color:#94a3b8;

}

</style>

</head>

<body>

<div class="header">

🚀 ESP32 CONTROL OS V6

</div>

<div class="nav">

<a href="/">🏠 Dashboard</a>

<a href="/wifi">📶 WiFi Analyzer</a>

<a href="/system">🖥 System</a>

<a href="/charts">📈 Charts</a>

<a href="/logs">📝 Logs</a>

<a href="/settings">⚙ Settings</a>

<a href="/about">👨‍💻 About</a>

</div>

<div class="container">

)rawliteral";

return html;
}

String getFooter(){

return R"rawliteral(

</div>

<div class="footer">

ESP32 ControlOS V6
<br>
Built by Gnandeep Yenigella
<br>
Built for ESP32 • Arduino C++

</div>

</body>
</html>

)rawliteral";

}
void dashboardPage(){

String page = getHeader();

page += R"rawliteral(

<div class="card">

<h2>📊 System Dashboard</h2>

<p>Current device readings captured when this page loads.</p>

</div>

<div class="grid">

)rawliteral";

page += "<div class='card'>";
page += "<div class='label'>FREE RAM</div>";
page += "<div class='stat'>";
page += String(ESP.getFreeHeap()/1024);
page += " KB</div>";
page += "</div>";

page += "<div class='card'>";
page += "<div class='label'>UPTIME</div>";
page += "<div class='stat'>";
page += String(millis()/1000);
page += " s</div>";
page += "</div>";page += "<div class='card'>";
page += "<h2>💾 Device & Storage</h2><br>";
page += "<b>Connected Clients:</b> ";
page += String(WiFi.softAPgetStationNum());
page += "<br><br>";
page += "<b>Flash Size:</b> ";
page += String(ESP.getFlashChipSize()/1024/1024);
page += " MB";
page += "<br><br>";
page += "<b>Firmware Size:</b> ";
page += String(ESP.getSketchSize()/1024);
page += " KB";
page += "</div>";

page += "<div class='card'>";
page += "<div class='label'>CPU</div>";
page += "<div class='stat'>";
page += String(getCpuFrequencyMhz());
page += " MHz</div>";
page += "</div>";

page += "<div class='card'>";
page += "<div class='label'>IP ADDRESS</div>";
page += "<div class='stat'>";
page += WiFi.softAPIP().toString();
page += "</div>";
page += "</div>";

page += "</div>";

page += "<div class='card'>";

page += "<h2>💡 Device Control</h2>";

page += "<p>Built-in LED Status: ";

if(digitalRead(LED_PIN))
page += "🟢 ON";
else
page += "🔴 OFF";

page += "</p><br>";

page += "<form method='post' action='/on' style='display:inline'>";
page += "<button class='green' type='submit'>LED ON</button>";
page += "</form>&nbsp;";
page += "<form method='post' action='/off' style='display:inline'>";
page += "<button class='red' type='submit'>LED OFF</button>";
page += "</form>";

page += "</div>";
String health;

if(ESP.getFreeHeap()/1024 > 150)
{
  health = "🟢 HEALTHY";
}
else if(ESP.getFreeHeap()/1024 > 100)
{
  health = "🟡 WARNING";
}
else
{
  health = "🔴 CRITICAL";
}

page += "<div class='card'>";
page += "<div class='label'>SYSTEM HEALTH</div>";
page += "<div class='stat'>";
page += health;
page += "</div>";
page += "</div>";
page += "<div class='card'>";
page += "<h2>📈 History & Logs</h2><br>";
page += "<p>Review recent free-heap and access-point client samples.</p><br>";
page += "<a class='action-link blue' href='/charts'>Open device history</a> ";
page += "<a class='action-link blue' href='/logs'>View system logs</a>";
page += "</div>";

page += "<div class='card'>";
page += "<a class='action-link blue' href='/'>🔄 Refresh Dashboard</a>";
page += "</div>";
page += getFooter();

server.send(200,"text/html",page);

}

void wifiPage()
{
String page = getHeader();

page += R"rawliteral(

<div class="card">

<h2>📶 Wi-Fi Analyzer</h2>

<p>Nearby wireless networks</p>

</div>

)rawliteral";

int n = WiFi.scanNetworks();
page += "<div class='card'>";

page += "<h3>";

page += String(n < 0 ? 0 : n);

page += " Networks Detected";

page += "</h3>";

page += "</div>";

if (n < 0)
{
  page += "<div class='card'><h3>Scan unavailable</h3><p>Try opening the analyzer again.</p></div>";
}
else if(n == 0){

page += R"rawliteral(

<div class="card">

<h3>No Networks Found</h3>

</div>

)rawliteral";

}
else{

for(int i=0;i<n;i++){

String security;
int rssi = WiFi.RSSI(i);

int quality;

if(rssi >= -50)
{
  quality = 100;
}
else if(rssi >= -60)
{
  quality = 80;
}
else if(rssi >= -70)
{
  quality = 60;
}
else if(rssi >= -80)
{
  quality = 40;
}
else
{
  quality = 20;
}

String status;

if(quality >= 80)
{
  status = "🟢 Excellent";
}
else if(quality >= 60)
{
  status = "🟡 Good";
}
else
{
  status = "🔴 Weak";
}

switch(WiFi.encryptionType(i)){

case WIFI_AUTH_OPEN:
security = "Open";
break;

case WIFI_AUTH_WPA_PSK:
security = "WPA";
break;

case WIFI_AUTH_WPA2_PSK:
security = "WPA2";
break;

case WIFI_AUTH_WPA_WPA2_PSK:
security = "WPA/WPA2";
break;

default:
security = "Other";

}

page += "<div class='card'>";

page += "<h3>";
page += escapeHtml(WiFi.SSID(i));
page += "</h3>";

page += "<br>";

page += "<b>Signal:</b> ";
page += String(rssi);
page += " dBm";
page += "<br><br>";

page += "<b>Quality:</b> ";

page += String(quality);

page += "%";

page += "<br><br>";

page += "<b>Channel:</b> ";
page += String(WiFi.channel(i));

page += "<br><br>";

page += "<b>Security:</b> ";
page += security;
page += "<br><br>";

page += "<b>Status:</b> ";

page += status;

page += "</div>";

}

}

page += R"rawliteral(

<div class="card">

<a class="action-link blue" href="/wifi">🔄 Scan Again</a>

</div>

)rawliteral";

WiFi.scanDelete();

page += getFooter();

server.send(200,"text/html",page);

}
void systemPage(){

String page = getHeader();

page += R"rawliteral(

<div class="card">

<h2>📊 System Monitor</h2>

<p>Live ESP32 Hardware Information</p>

</div>

<div class="grid">

)rawliteral";

page += "<div class='card'>";
page += "<div class='label'>CPU FREQUENCY</div>";
page += "<div class='stat'>";
page += String(getCpuFrequencyMhz());
page += " MHz</div>";
page += "</div>";

page += "<div class='card'>";
page += "<div class='label'>FLASH SIZE</div>";
page += "<div class='stat'>";
page += String(ESP.getFlashChipSize()/1024/1024);
page += " MB</div>";
page += "</div>";

page += "<div class='card'>";
page += "<div class='label'>FREE HEAP</div>";
page += "<div class='stat'>";
page += String(ESP.getFreeHeap()/1024);
page += " KB</div>";
page += "</div>";





page += "<div class='card'>";
page += "<div class='label'>UPTIME</div>";
page += "<div class='stat'>";
page += String(millis()/1000);
page += "s</div>";
page += "</div>";
page += "<div class='card'>";
page += "<div class='label'>IP ADDRESS</div>";
page += "<div class='stat'>";
page += WiFi.softAPIP().toString();
page += "</div>";
page += "</div>";

page += "<div class='card'>";
page += "<div class='label'>CONNECTED CLIENTS</div>";
page += "<div class='stat'>";
page += String(WiFi.softAPgetStationNum());
page += "</div>";
page += "</div>";

page += "</div>";

page += getFooter();

server.send(200,"text/html",page);

}
void restartESP(){

server.send(
200,
"text/html",
"<h2>Restarting ESP32...</h2>"
);

delay(1000);

ESP.restart();

}

void settingsPage(){

String page = getHeader();

page += R"rawliteral(

<div class="card">

<h2>⚙️ Settings</h2>

<p>Device information and local maintenance</p>

</div>

<div class="grid">

<div class="card">

<div class="label">

DEVICE

</div>

<div class="stat">

ESP32

</div>

</div>

<div class="card">

<div class="label">

VERSION

</div>

<div class="stat">

v6.0

</div>

</div>

</div>

<div class="card">

<h3>System Actions</h3>

<br>

<form method="post" action="/restart">
<button class="blue" type="submit">Restart ESP32</button>
</form>

</div>

)rawliteral";

page += getFooter();

server.send(200,"text/html",page);

}

void chartsPage()
{
  String page = getHeader();

  page += R"rawliteral(
<div class="card">
  <h2>Recent device history</h2>
  <p>Free memory and connected clients, sampled every five seconds.</p>
  <p id="chart-status" role="status">Loading recent readings...</p>
  <p class="label">Each line is scaled to show its trend over the same time window.</p>
</div>

<div class="card">
  <div style="display:flex;gap:1.2rem;flex-wrap:wrap;margin-bottom:1rem">
    <span><span style="color:#38bdf8">●</span> Free heap</span>
    <span><span style="color:#4ade80">●</span> Connected clients</span>
  </div>
  <svg id="history-chart" viewBox="0 0 760 300" role="img" aria-labelledby="history-title history-desc"
       style="display:block;width:100%;height:auto;min-height:220px">
    <title id="history-title">Recent device history</title>
    <desc id="history-desc">A line chart of free heap and connected access point clients.</desc>
    <text x="50%" y="50%" text-anchor="middle" fill="#94a3b8">Waiting for readings</text>
  </svg>
</div>

<script>
(async function () {
  const status = document.getElementById('chart-status');
  const chart = document.getElementById('history-chart');
  try {
    const response = await fetch('/history', { cache: 'no-store' });
    if (!response.ok) throw new Error('History request failed');
    const payload = await response.json();
    const readings = Array.isArray(payload.entries) ? payload.entries : [];
    if (!readings.length) {
      status.textContent = 'No readings yet. New samples are recorded every five seconds.';
      return;
    }

    const width = 760;
    const height = 300;
    const left = 54;
    const right = 22;
    const top = 24;
    const bottom = 38;
    const plotWidth = width - left - right;
    const plotHeight = height - top - bottom;
    const x = (index) => left + (readings.length < 2 ? plotWidth / 2 : index * plotWidth / (readings.length - 1));
    const points = (key) => {
      const values = readings.map((item) => Number(item[key]) || 0);
      const minimum = Math.min(...values);
      const maximum = Math.max(...values);
      const range = maximum - minimum;
      return values.map((value, index) => {
        const normalized = range === 0 ? 0.5 : (value - minimum) / range;
        const y = top + plotHeight - normalized * plotHeight;
        return (index === 0 ? 'M' : 'L') + x(index).toFixed(1) + ',' + y.toFixed(1);
      }).join(' ');
    };

    const grid = [0, 1, 2, 3, 4].map((line) => {
      const y = top + line * plotHeight / 4;
      return '<line x1="' + left + '" y1="' + y + '" x2="' + (width - right) +
        '" y2="' + y + '" stroke="#334155" stroke-width="1" />';
    }).join('');

    chart.innerHTML =
      '<title id="history-title">Recent device history</title>' +
      '<desc id="history-desc">Normalized trends for free heap and connected clients across ' +
      readings.length + ' readings.</desc>' + grid +
      '<path d="' + points('heap_kb') + '" fill="none" stroke="#38bdf8" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" />' +
      '<path d="' + points('clients') + '" fill="none" stroke="#4ade80" stroke-width="3" stroke-linecap="round" stroke-linejoin="round" />' +
      '<text x="' + left + '" y="' + (height - 10) + '" fill="#94a3b8" font-size="12">Oldest</text>' +
      '<text x="' + (width - right) + '" y="' + (height - 10) + '" text-anchor="end" fill="#94a3b8" font-size="12">Newest</text>';

    const latest = readings[readings.length - 1];
    status.textContent = 'Latest: ' + latest.heap_kb + ' KB free heap · ' +
      latest.clients + ' connected client(s) · ' + readings.length + ' readings';
  } catch (error) {
    status.textContent = 'History could not be loaded. Refresh the page to try again.';
  }
})();
</script>
)rawliteral";

  page += getFooter();
  server.send(200, "text/html", page);
}

void historyAPI()
{
  String json = "{\"entries\":[";
  const int oldestLogIndex = (nextLogIndex - logCount + LOG_CAPACITY) % LOG_CAPACITY;

  for (int offset = 0; offset < logCount; offset++)
  {
    const int index = (oldestLogIndex + offset) % LOG_CAPACITY;
    if (offset > 0) json += ",";
    json += "{\"seconds\":" + String(logs[index].time);
    json += ",\"heap_kb\":" + String(logs[index].heap);
    json += ",\"clients\":" + String(logs[index].clients) + "}";
  }

  json += "]}";
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", json);
}

void logsPage(){

String page = getHeader();

page += R"rawliteral(

<div class="card">

<h2>📝 System Logs</h2>

)rawliteral";

const int oldestLogIndex = (nextLogIndex - logCount + LOG_CAPACITY) % LOG_CAPACITY;
for (int offset = 0; offset < logCount; offset++)
{
  const int index = (oldestLogIndex + offset) % LOG_CAPACITY;
  page += "Time: ";
  page += String(logs[index].time);
  page += "s | RAM: ";
  page += String(logs[index].heap);
  page += "KB | Clients: ";
  page += String(logs[index].clients);
  page += "<br>";
}

page += "</div>";

page += getFooter();

server.send(200,"text/html",page);

}
void aboutPage(){

String page = getHeader();
page += R"rawliteral(

<div class="card">

<h2>ℹ️ About ESP32 ControlOS V6</h2>

<p>
A local web dashboard for device monitoring, Wi-Fi analysis, and built-in LED control.
</p>

</div>

<div class="grid">

<div class="card">

<div class="label">
PROJECT
</div>

<div class="stat">
ESP32 ControlOS
</div>

</div>

<div class="card">

<div class="label">
VERSION
</div>

<div class="stat">
6.0
</div>

</div>

<div class="card">

<div class="label">
PLATFORM
</div>

<div class="stat">
ESP32
</div>

</div>

<div class="card">

<div class="label">
LANGUAGE
</div>

<div class="stat">
Arduino C++
</div>

</div>

</div>

<div class="card">

<h3>Developer</h3>

<br>

<p>
Gnandeep Yenigella
</p>

</div>

<div class="card">

<h3>Included features</h3>

<ul>

<li>On-demand Wi-Fi scan</li>

<li>Heap and uptime history</li>

<li>Access point client count</li>

<li>GPIO 2 LED control</li>

<li>Local restart action</li>

<li>JSON status and history endpoints</li>

</ul>

<p>Anyone connected to this access point can operate the dashboard, including controlling the LED and restarting the ESP32. Share its passphrase only with trusted users.</p>

</div>

)rawliteral";

page += getFooter();

server.send(200,"text/html",page);

}
void ledON(){

digitalWrite(LED_PIN, HIGH);

server.sendHeader("Location","/");
server.send(303);

}

void ledOFF(){

digitalWrite(LED_PIN, LOW);

server.sendHeader("Location","/");
server.send(303);

}

void notFound(){




String page = getHeader();

page += R"rawliteral(

<div class="card">

<h2>🚫 404 - Page Not Found</h2>

<p>

The page you requested does not exist.

</p>

<br>

<a class="action-link blue" href="/">🏠 Go Home</a>

</div>

)rawliteral";

page += getFooter();

server.send(404,"text/html",page);

}void statusAPI()
{
  server.sendHeader("Cache-Control", "no-store");
  String json = "{";

  json += "\"heap\":" + String(ESP.getFreeHeap()/1024) + ",";
  json += "\"uptime\":" + String(millis()/1000) + ",";
  json += "\"cpu\":" + String(getCpuFrequencyMhz()) + ",";
  json += "\"ip\":\"" + WiFi.softAPIP().toString() + "\"";

  json += "}";

  server.send(200, "application/json", json);
}

void saveLog()
{
  logs[nextLogIndex].time = millis()/1000;
  logs[nextLogIndex].heap = ESP.getFreeHeap()/1024;
  logs[nextLogIndex].clients = WiFi.softAPgetStationNum();

  nextLogIndex = (nextLogIndex + 1) % LOG_CAPACITY;
  if (logCount < LOG_CAPACITY)
  {
    logCount++;
  }
}

void setup(){

Serial.begin(115200);

pinMode(LED_PIN, OUTPUT);
digitalWrite(LED_PIN, LOW);

Serial.println();
Serial.println("=================================");
Serial.println("ESP32 ControlOS V6");
Serial.println("Built by Gnandeep Yenigella");
Serial.println("Built for ESP32 • Arduino C++");
Serial.println("Starting...");
Serial.println("=================================");

if (!isValidAccessPointPassword())
{
  Serial.println("Set a private WPA2 passphrase (8-63 characters) before uploading.");
  return;
}

WiFi.mode(WIFI_AP);

if (!WiFi.softAP(ssid, password))
{
  Serial.println("Access point could not be started.");
  return;
}

Serial.print("SSID: ");
Serial.println(ssid);

Serial.print("IP Address: ");
Serial.println(WiFi.softAPIP());

server.on("/", dashboardPage);

server.on("/wifi", wifiPage);

server.on("/system", systemPage);

server.on("/charts", chartsPage);
server.on("/history", historyAPI);

server.on("/status", statusAPI);

server.on("/settings", settingsPage);

server.on("/logs", logsPage);

server.on("/about", aboutPage);

server.on("/on", HTTP_POST, ledON);

server.on("/off", HTTP_POST, ledOFF);

server.on("/restart", HTTP_POST, restartESP);

server.onNotFound(notFound);

server.begin();

Serial.println();
Serial.println("Web Server Started!");
Serial.println("Open Browser:");
Serial.println("http://192.168.4.1");
Serial.println("=================================");

}

void loop(){

server.handleClient();

static unsigned long lastLog = 0;

if(millis() - lastLog > 5000)
{
  saveLog();
  lastLog = millis();
}

}
