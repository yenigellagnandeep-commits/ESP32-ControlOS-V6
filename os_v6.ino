#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_Control_OS";
const char* password = "12345678";

WebServer server(80);
struct LogEntry
{
  unsigned long time;
  int heap;
  int rssi;
};

LogEntry logs[100];

int logIndex = 0;

const int LED_PIN = 2;

String getHeader()
 {

String html = R"rawliteral(

<!DOCTYPE html>
<html>

<head>

<meta charset="UTF-8">

<meta name="viewport" content="width=device-width, initial-scale=1">

<title>ESP32 Control OS Pro</title>

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
c:\Users\yenig\OneDrive\Desktop\phonix
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

.footer{

text-align:center;
padding:20px;
color:#94a3b8;

}

</style>

</head>

<body>

<div class="header">

🚀 ESP32 CONTROL OS PRO

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

ESP32 Control OS Pro v4.0
<br>
Developer: Gnandeep Yenigella
<br>
Powered by ESP32 DevKit V1

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

<p>Real-time ESP32 information</p>

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
page += "<h2>⚡ Quick Statistics</h2><br>";

page += "<b>Networks Found:</b> ";
page += String(WiFi.scanNetworks());

page += "<br><br>";

page += "<b>Flash Size:</b> ";
page += String(ESP.getFlashChipSize()/1024/1024);
page += " MB";

page += "<br><br>";

page += "<b>Sketch Size:</b> ";
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

page += "<a href='/on'>";
page += "<button class='green'>LED ON</button>";
page += "</a>";

page += "&nbsp;";

page += "<a href='/off'>";
page += "<button class='red'>LED OFF</button>";
page += "</a>";

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

page += "<h2>📈 Analytics</h2><br>";

page += "<b>WiFi Networks Found:</b> ";
page += String(WiFi.scanNetworks());

page += "<br><br>";

page += "<b>Free Heap:</b> ";
page += String(ESP.getFreeHeap()/1024);
page += " KB";

page += "<br><br>";

page += "<b>Flash Size:</b> ";
page += String(ESP.getFlashChipSize()/1024/1024);
page += " MB";

page += "<br><br>";

page += "<b>Sketch Size:</b> ";
page += String(ESP.getSketchSize()/1024);
page += " KB";

page += "<br><br>";

page += "<b>CPU Frequency:</b> ";
page += String(getCpuFrequencyMhz());
page += " MHz";

page += "</div>";
page += "<div class='card'>";

page += "<a href='/'>";

page += "<button class='blue'>";

page += "🔄 Refresh Dashboard";

page += "</button>";

page += "</a>";

page += "</div>";page += getFooter();

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

page += String(n);

page += " Networks Detected";

page += "</h3>";

page += "</div>";

if(n == 0){

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
page += WiFi.SSID(i);
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

<a href="/wifi">

<button class="blue">

🔄 Scan Again

</button>

</a>

</div>

)rawliteral";

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
unsigned long totalSeconds = millis()/1000;

int hours = totalSeconds / 3600;
int minutes = (totalSeconds % 3600) / 60;
int seconds = totalSeconds % 60;


page += "<div class='card'>";
page += "<div class='label'>IP ADDRESS</div>";page += "<div class='card'>";

page += "<div class='label'>CONNECTED DEVICES</div>";

page += "<div class='stat'>";

page += String(WiFi.softAPgetStationNum());

page += "</div>";

page += "</div>";
page += "<div class='stat'>";
page += WiFi.softAPIP().toString();
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

<p>System Management</p>

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

v3.0

</div>

</div>

</div>

<div class="card">

<h3>System Actions</h3>

<br>

<a href="/restart">

<button class="blue">

🔄 Restart ESP32

</button>

</a>

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

<h2>📈 Live Charts</h2>

<p>Charts module is under development.</p>

<p>Developer: Gnandeep Yenigella</p>

</div>

)rawliteral";

  page += getFooter();

  server.send(200,"text/html",page);
}
void logsPage(){

String page = getHeader();

page += R"rawliteral(

<div class="card">

<h2>📝 System Logs</h2>

)rawliteral";

for(int i=0;i<logIndex;i++)
{
  page += "Time: ";
  page += String(logs[i].time);
  page += "s | RAM: ";
  page += String(logs[i].heap);
  page += "KB | RSSI: ";
  page += String(logs[i].rssi);
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

<h2>ℹ️ About ESP32 Control OS Pro</h2>

<p>
A professional ESP32 web dashboard designed for
monitoring, networking, and device management.
</p>

</div>

<div class="grid">

<div class="card">

<div class="label">
PROJECT
</div>

<div class="stat">
ESP32 OS
</div>

</div>

<div class="card">

<div class="label">
VERSION
</div>

<div class="stat">
3.0
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

<h3>Future Features</h3>

<ul>

<li>Network Tools</li>

<li>Live Charts</li>

<li>OTA Updates</li>

<li>GPIO Manager</li>

<li>Data Logging</li>

<li>Advanced Analytics</li>

</ul>

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

<a href="/">

<button class="blue">

🏠 Go Home

</button>

</a>

</div>

)rawliteral";

page += getFooter();

server.send(404,"text/html",page);

}void statusAPI()
{
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
  logs[logIndex].time = millis()/1000;
  logs[logIndex].heap = ESP.getFreeHeap()/1024;
  logs[logIndex].rssi = WiFi.RSSI();

  logIndex++;

  if(logIndex >= 100)
  {
    logIndex = 0;
  }
}

void setup(){

Serial.begin(115200);

pinMode(LED_PIN, OUTPUT);
digitalWrite(LED_PIN, LOW);

Serial.println();
Serial.println("=================================");
Serial.println("ESP32 Control OS Pro v4.0");
Serial.println("Developer: Gnandeep Yenigella");
Serial.println("Powered by ESP32 DevKit V1");
Serial.println("Starting...");
Serial.println("=================================");

WiFi.mode(WIFI_AP);

WiFi.softAP(ssid, password);

Serial.print("SSID: ");
Serial.println(ssid);

Serial.print("Password: ");
Serial.println(password);

Serial.print("IP Address: ");
Serial.println(WiFi.softAPIP());

server.on("/", dashboardPage);

server.on("/wifi", wifiPage);

server.on("/system", systemPage);

server.on("/charts", chartsPage);

server.on("/status", statusAPI);

server.on("/settings", settingsPage);

server.on("/logs", logsPage);

server.on("/about", aboutPage);

server.on("/on", ledON);

server.on("/off", ledOFF);

server.on("/restart", restartESP);

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
