#include <WiFi.h>
#include <WebServer.h>
#include <esp_timer.h>
#include <esp_random.h>
#include <stdio.h>
#include <string.h>

// Single-file Wi-Fi configuration: no extra config headers required.
// Change these values before uploading, especially the password.
#define CONTROL_OS_WIFI_SSID "My_ControlOS"
#define CONTROL_OS_WIFI_PASSWORD "ControlOS-826-ChangeMe"

#ifndef CONTROL_OS_LED_PIN
#define CONTROL_OS_LED_PIN 2
#endif

#ifndef CONTROL_OS_LED_ACTIVE_LOW
#define CONTROL_OS_LED_ACTIVE_LOW 0
#endif

namespace
{
constexpr uint32_t kSampleIntervalMs = 5000UL;
constexpr size_t kLogCapacity = 100;
constexpr char kPlaceholderPassword[] = "";
constexpr uint8_t kLedPin = CONTROL_OS_LED_PIN;
constexpr bool kLedActiveLow = CONTROL_OS_LED_ACTIVE_LOW != 0;

const char* const kAccessPointSsid = CONTROL_OS_WIFI_SSID;
const char* const kAccessPointPassword = CONTROL_OS_WIFI_PASSWORD;

struct LogEntry
{
  uint64_t uptimeSeconds;
  uint32_t freeHeapKb;
  uint8_t connectedClients;
};

WebServer server(80);
LogEntry logs[kLogCapacity];
size_t nextLogIndex = 0;
size_t logCount = 0;
uint32_t lastSampleMillis = 0;
bool serverReady = false;
bool ledIsOn = false;
String controlToken;

uint64_t uptimeSeconds()
{
  const int64_t elapsedMicroseconds = esp_timer_get_time();
  return elapsedMicroseconds > 0
           ? static_cast<uint64_t>(elapsedMicroseconds) / 1000000ULL
           : 0ULL;
}

String formatUnsigned64(uint64_t value)
{
  char buffer[21];
  snprintf(buffer, sizeof(buffer), "%llu",
           static_cast<unsigned long long>(value));
  return String(buffer);
}

String formatUptime(uint64_t seconds)
{
  const uint64_t days = seconds / 86400ULL;
  const unsigned int hours = static_cast<unsigned int>((seconds / 3600ULL) % 24ULL);
  const unsigned int minutes = static_cast<unsigned int>((seconds / 60ULL) % 60ULL);
  const unsigned int remainingSeconds = static_cast<unsigned int>(seconds % 60ULL);
  char buffer[64];

  if (days > 0)
  {
    snprintf(buffer, sizeof(buffer), "%llu d %02u:%02u:%02u",
             static_cast<unsigned long long>(days), hours, minutes,
             remainingSeconds);
  }
  else
  {
    snprintf(buffer, sizeof(buffer), "%02u:%02u:%02u", hours, minutes,
             remainingSeconds);
  }

  return String(buffer);
}

bool hasValidAccessPointConfig()
{
  if (kAccessPointSsid == nullptr || kAccessPointPassword == nullptr)
  {
    return false;
  }

  const size_t ssidLength = strlen(kAccessPointSsid);
  const size_t passwordLength = strlen(kAccessPointPassword);

  return ssidLength > 0 && ssidLength <= 32 &&
         kAccessPointPassword[0] != '\0' &&
         passwordLength >= 8 && passwordLength <= 63;
}

void setLed(bool on)
{
  ledIsOn = on;
  const bool outputHigh = kLedActiveLow ? !on : on;
  digitalWrite(kLedPin, outputHigh ? HIGH : LOW);
}

String createControlToken()
{
  char token[33];
  const uint8_t randomBytes[16] = {
    static_cast<uint8_t>(esp_random()), static_cast<uint8_t>(esp_random()),
    static_cast<uint8_t>(esp_random()), static_cast<uint8_t>(esp_random()),
    static_cast<uint8_t>(esp_random()), static_cast<uint8_t>(esp_random()),
    static_cast<uint8_t>(esp_random()), static_cast<uint8_t>(esp_random()),
    static_cast<uint8_t>(esp_random()), static_cast<uint8_t>(esp_random()),
    static_cast<uint8_t>(esp_random()), static_cast<uint8_t>(esp_random()),
    static_cast<uint8_t>(esp_random()), static_cast<uint8_t>(esp_random()),
    static_cast<uint8_t>(esp_random()), static_cast<uint8_t>(esp_random())
  };

  for (size_t index = 0; index < sizeof(randomBytes); ++index)
  {
    snprintf(token + index * 2, sizeof(token) - index * 2, "%02x",
             static_cast<unsigned int>(randomBytes[index]));
  }
  return String(token);
}

String controlTokenField()
{
  String field;
  field.reserve(100);
  field += F("<input type=\"hidden\" name=\"token\" value=\"");
  field += controlToken;
  field += F("\">");
  return field;
}

bool hasValidControlToken()
{
  return server.hasArg("token") && server.arg("token") == controlToken;
}

void rejectInvalidControlRequest()
{
  server.send(403, "text/plain; charset=utf-8",
              "This control request is invalid. Reload the page and try again.");
}

String escapeHtml(const String& input)
{
  String escaped = input;
  escaped.replace("&", "&amp;");
  escaped.replace("<", "&lt;");
  escaped.replace(">", "&gt;");
  escaped.replace("\"", "&quot;");
  escaped.replace("'", "&#39;");
  return escaped;
}

String pageHeader()
{
  String page;
  page.reserve(5200);
  page += F(R"HTML(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="theme-color" content="#0b1220">
  <meta name="description" content="A private, local dashboard for an ESP32 development board.">
  <title>ESP32 ControlOS</title>
  <style>
    :root {
      color-scheme: dark;
      --page: #0b1220;
      --panel: #131f32;
      --panel-raised: #19283e;
      --line: #293b54;
      --text: #edf4ff;
      --muted: #a9b8cd;
      --blue: #63b3ff;
      --green: #64d9a0;
      --red: #ff8585;
      --focus: #ffd166;
    }
    * { box-sizing: border-box; }
    body {
      min-width: 320px;
      min-height: 100vh;
      margin: 0;
      background: radial-gradient(ellipse at top, #162640 0, var(--page) 52rem);
      color: var(--text);
      font: 16px/1.55 system-ui, -apple-system, "Segoe UI", sans-serif;
    }
    a { color: inherit; }
    button { font: inherit; }
    .skip-link {
      position: absolute; left: -9999px; top: 0; z-index: 10;
      padding: .7rem 1rem; background: var(--focus); color: #101827;
    }
    .skip-link:focus { left: 1rem; top: 1rem; }
    .site-header {
      display: flex; align-items: center; justify-content: space-between;
      gap: 1rem; max-width: 1180px; margin: 0 auto; padding: 1.35rem 1.25rem .9rem;
    }
    .brand { color: var(--text); text-decoration: none; }
    .brand strong { display: block; font-size: clamp(1.3rem, 3vw, 1.85rem); letter-spacing: .01em; }
    .brand span { display: block; color: var(--muted); font-size: .9rem; }
    .connection {
      display: inline-flex; align-items: center; gap: .5rem; white-space: nowrap;
      border: 1px solid var(--line); border-radius: 999px; padding: .45rem .75rem;
      color: var(--muted); background: rgba(19,31,50,.8); font-size: .86rem;
    }
    .connection::before {
      width: .55rem; height: .55rem; border-radius: 50%; background: #9aa9bc;
      content: "";
    }
    .connection[data-state="online"] { color: var(--green); }
    .connection[data-state="online"]::before { background: var(--green); }
    .connection[data-state="offline"] { color: var(--red); }
    .connection[data-state="offline"]::before { background: var(--red); }
    nav {
      display: flex; flex-wrap: wrap; justify-content: center; gap: .45rem;
      padding: .7rem 1rem; border-top: 1px solid rgba(169,184,205,.12);
      border-bottom: 1px solid rgba(169,184,205,.12); background: rgba(11,18,32,.65);
    }
    nav a {
      padding: .5rem .8rem; border-radius: .7rem; color: var(--muted);
      text-decoration: none; font-weight: 650;
    }
    nav a:hover { background: var(--panel-raised); color: var(--text); }
    .container { width: min(100% - 2rem, 1100px); margin: 1.5rem auto 3rem; }
    .intro { margin-bottom: 1rem; }
    .intro h1 { margin: 0 0 .3rem; font-size: clamp(1.5rem, 4vw, 2.1rem); }
    .intro p, .muted { color: var(--muted); }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(min(100%, 220px), 1fr)); gap: 1rem; }
    .card {
      min-width: 0; margin: 0 0 1rem; padding: 1.2rem;
      border: 1px solid var(--line); border-radius: 1rem;
      background: linear-gradient(145deg, rgba(25,40,62,.92), rgba(19,31,50,.96));
      box-shadow: 0 12px 30px rgba(0,0,0,.13);
    }
    .grid .card { margin: 0; }
    .card h2, .card h3 { margin: 0 0 .45rem; line-height: 1.3; }
    .card p { margin: .35rem 0; }
    .metric-label { color: var(--muted); font-size: .78rem; font-weight: 750; letter-spacing: .08em; }
    .metric-value { margin-top: .3rem; color: var(--blue); font-size: clamp(1.35rem, 4vw, 1.85rem); font-weight: 750; overflow-wrap: anywhere; }
    .metric-unit { color: var(--muted); font-size: .9rem; font-weight: 600; }
    .actions { display: flex; flex-wrap: wrap; gap: .65rem; margin-top: 1rem; }
    .button {
      display: inline-flex; min-height: 2.75rem; align-items: center; justify-content: center;
      padding: .6rem 1rem; border: 1px solid transparent; border-radius: .7rem;
      background: #244e7a; color: white; text-decoration: none; font-weight: 700;
      cursor: pointer;
    }
    .button:hover { filter: brightness(1.14); }
    .button-green { background: #176344; }
    .button-red { background: #793d48; }
    .button-quiet { border-color: var(--line); background: var(--panel-raised); }
    :focus-visible { outline: 3px solid var(--focus); outline-offset: 3px; }
    .table-wrap { overflow-x: auto; }
    table { width: 100%; border-collapse: collapse; text-align: left; }
    caption { padding: 0 0 .7rem; color: var(--muted); text-align: left; }
    th, td { padding: .72rem .65rem; border-bottom: 1px solid var(--line); }
    th { color: var(--muted); font-size: .82rem; }
    td { overflow-wrap: anywhere; }
    .notice {
      border-left: 3px solid var(--blue); padding: .75rem 1rem;
      background: rgba(99,179,255,.08); color: var(--muted);
    }
    .chart { display: block; width: 100%; height: auto; }
    .legend { display: flex; flex-wrap: wrap; gap: 1rem; color: var(--muted); }
    .footer { padding: 1.5rem 1rem 2rem; border-top: 1px solid rgba(169,184,205,.12); color: var(--muted); text-align: center; font-size: .9rem; }
    @media (max-width: 560px) {
      .site-header { align-items: flex-start; flex-direction: column; }
      nav { justify-content: flex-start; }
      .container { width: min(100% - 1.25rem, 1100px); margin-top: 1rem; }
      th, td { padding: .6rem .45rem; }
    }
    @media (prefers-reduced-motion: reduce) {
      *, *::before, *::after { scroll-behavior: auto !important; transition-duration: .01ms !important; }
    }
  </style>
</head>
<body>
  <a class="skip-link" href="#main-content">Skip to content</a>
  <header class="site-header">
    <a class="brand" href="/" aria-label="ESP32 ControlOS home">
      <strong>ESP32 ControlOS</strong>
      <span>A private dashboard running on your board</span>
    </a>
    <span id="connection-status" class="connection" data-state="loading" role="status">Connecting…</span>
  </header>
  <nav aria-label="Main navigation">
    <a href="/">Dashboard</a>
    <a href="/wifi">Wi-Fi scan</a>
    <a href="/system">Device details</a>
    <a href="/charts">History</a>
    <a href="/logs">Logs</a>
    <a href="/settings">Settings</a>
    <a href="/about">About</a>
  </nav>
  <main id="main-content" class="container">
)HTML");
  return page;
}

String pageFooter()
{
  return F(R"HTML(  </main>
  <footer class="footer">
    ESP32 ControlOS · Built by Gnandeep Yenigella · Arduino C++
  </footer>
</body>
</html>
)HTML");
}

String metricCard(const char* label, const char* id, const char* unit)
{
  String card;
  card.reserve(220);
  card += F("<article class=\"card\"><div class=\"metric-label\">");
  card += label;
  card += F("</div><div class=\"metric-value\"><span id=\"");
  card += id;
  card += F("\">—</span>");
  if (unit != nullptr && unit[0] != '\0')
  {
    card += F(" <span class=\"metric-unit\">");
    card += unit;
    card += F("</span>");
  }
  card += F("</div></article>");
  return card;
}

void sendPage(const String& page)
{
  server.sendHeader("Cache-Control", "no-store, max-age=0");
  server.send(200, "text/html; charset=utf-8", page);
}

void dashboardPage()
{
  String page = pageHeader();
  page += F(R"HTML(
    <section class="intro">
      <h1>Device dashboard</h1>
      <p>Live readings from this ESP32, refreshed every five seconds.</p>
    </section>
    <section class="grid" aria-label="Live device readings">
)HTML");
  page += metricCard("FREE HEAP", "free-heap", "KB");
  page += metricCard("UPTIME", "uptime", "");
  page += metricCard("CPU FREQUENCY", "cpu-frequency", "MHz");
  page += metricCard("FLASH SIZE", "flash-size", "MB");
  page += metricCard("FIRMWARE SIZE", "firmware-size", "KB");
  page += metricCard("CONNECTED CLIENTS", "connected-clients", "");
  page += metricCard("ACCESS POINT ADDRESS", "access-point-ip", "");
  page += F(R"HTML(    </section>
    <section class="card" aria-labelledby="led-heading" style="margin-top:1rem">
      <h2 id="led-heading">Built-in LED</h2>
      <p>Current state: <strong id="led-state">—</strong></p>
      <div class="actions">
        <form method="post" action="/on">)HTML");
  page += controlTokenField();
  page += F(R"HTML(<button class="button button-green" type="submit">Turn LED on</button></form>
        <form method="post" action="/off">)HTML");
  page += controlTokenField();
  page += F(R"HTML(<button class="button button-red" type="submit">Turn LED off</button></form>
      </div>
      <p class="muted">The LED pin and polarity depend on the ESP32 board. See the setup guide if it does not respond.</p>
    </section>
    <section class="card">
      <h2>Recent activity</h2>
      <p class="muted">Readings are kept in memory and disappear when the ESP32 restarts.</p>
      <div class="actions">
        <a class="button button-quiet" href="/charts">View history</a>
        <a class="button button-quiet" href="/logs">View logs</a>
      </div>
    </section>
    <script>
    (() => {
      const statusBadge = document.getElementById('connection-status');
      const setText = (id, value) => {
        const element = document.getElementById(id);
        if (element) element.textContent = value;
      };
      const formatUptime = (totalSeconds) => {
        const seconds = Math.max(0, Math.floor(totalSeconds));
        const days = Math.floor(seconds / 86400);
        const hours = Math.floor((seconds % 86400) / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        const remainder = seconds % 60;
        const pad = (number) => String(number).padStart(2, '0');
        return (days ? days + 'd ' : '') + pad(hours) + ':' + pad(minutes) + ':' + pad(remainder);
      };
      async function refreshStatus() {
        try {
          const response = await fetch('/status', { cache: 'no-store' });
          if (!response.ok) throw new Error('Status request failed');
          const data = await response.json();
          if (!Number.isFinite(data.heap_kb) || !Number.isFinite(data.uptime_seconds)) {
            throw new Error('Status response was incomplete');
          }
          setText('free-heap', data.heap_kb);
          setText('uptime', formatUptime(data.uptime_seconds));
          setText('cpu-frequency', data.cpu_mhz);
          setText('flash-size', data.flash_mb);
          setText('firmware-size', data.firmware_kb);
          setText('connected-clients', data.connected_clients);
          setText('access-point-ip', data.ip);
          setText('led-state', data.led_on ? 'On' : 'Off');
          statusBadge.dataset.state = 'online';
          statusBadge.textContent = 'ESP32 connected';
        } catch (error) {
          statusBadge.dataset.state = 'offline';
          statusBadge.textContent = 'Waiting for ESP32';
        }
      }
      refreshStatus();
      window.setInterval(refreshStatus, 5000);
    })();
    </script>
)HTML");
  page += pageFooter();
  sendPage(page);
}

void wifiPage()
{
  String page = pageHeader();
  page += F(R"HTML(
    <section class="intro">
      <h1>Nearby Wi-Fi networks</h1>
      <p>This page starts a scan of visible 2.4 GHz networks. Scanning can take a few seconds.</p>
    </section>
)HTML");

  const int networkCount = WiFi.scanNetworks(false, true);

  if (networkCount < 0)
  {
    page += F("<section class=\"card\"><h2>Scan unavailable</h2><p>Please wait a moment, then scan again.</p></section>");
  }
  else if (networkCount == 0)
  {
    page += F("<section class=\"card\"><h2>No networks found</h2><p>Try scanning again from a location with Wi-Fi coverage.</p></section>");
  }
  else
  {
    page += F(R"HTML(
    <section class="card">
      <h2>)HTML");
    page += String(networkCount);
    page += F(R"HTML( networks found</h2>
      <div class="table-wrap">
        <table>
          <caption>Signal values are the measurements reported by the ESP32 radio.</caption>
          <thead><tr><th scope="col">Network</th><th scope="col">Signal</th><th scope="col">Channel</th><th scope="col">Security</th></tr></thead>
          <tbody>
)HTML");

    for (int index = 0; index < networkCount; ++index)
    {
      const String scannedName = WiFi.SSID(index);
      const String displayName = scannedName.length() == 0
                                   ? String("Hidden network")
                                   : escapeHtml(scannedName);
      const String security = WiFi.encryptionType(index) == WIFI_AUTH_OPEN
                                ? String("Open")
                                : String("Secured");

      page += F("<tr><td>");
      page += displayName;
      page += F("</td><td>");
      page += String(WiFi.RSSI(index));
      page += F(" dBm</td><td>");
      page += String(WiFi.channel(index));
      page += F("</td><td>");
      page += security;
      page += F("</td></tr>");
    }

    page += F("</tbody></table></div></section>");
  }

  WiFi.scanDelete();
  page += F("<section class=\"card\"><p class=\"muted\">This board does not connect to the networks shown here. To scan again, reload this page.</p></section>");
  page += pageFooter();
  sendPage(page);
}

void systemPage()
{
  String page = pageHeader();
  page += F(R"HTML(
    <section class="intro">
      <h1>Device details</h1>
      <p>A snapshot of this board’s hardware and current access point.</p>
    </section>
    <section class="grid">
)HTML");
  page += metricCard("FREE HEAP", "detail-heap", "KB");
  page += metricCard("UPTIME", "detail-uptime", "");
  page += metricCard("CPU FREQUENCY", "detail-cpu", "MHz");
  page += metricCard("FLASH SIZE", "detail-flash", "MB");
  page += metricCard("FIRMWARE SIZE", "detail-sketch", "KB");
  page += metricCard("CONNECTED CLIENTS", "detail-clients", "");
  page += metricCard("ACCESS POINT ADDRESS", "detail-ip", "");
  page += F(R"HTML(    </section>
    <section class="card" style="margin-top:1rem">
      <h2>About these readings</h2>
      <p class="muted">These values come from the ESP32 Arduino core. Available flash depends on the board, and the built-in LED is board-specific.</p>
    </section>
    <script>
    (() => {
      const setText = (id, value) => {
        const element = document.getElementById(id);
        if (element) element.textContent = value;
      };
      const formatUptime = (value) => {
        const seconds = Math.max(0, Math.floor(value));
        const days = Math.floor(seconds / 86400);
        const hours = Math.floor((seconds % 86400) / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        const remainder = seconds % 60;
        const pad = (number) => String(number).padStart(2, '0');
        return (days ? days + 'd ' : '') + pad(hours) + ':' + pad(minutes) + ':' + pad(remainder);
      };
      fetch('/status', { cache: 'no-store' })
        .then((response) => {
          if (!response.ok) throw new Error('Status request failed');
          return response.json();
        })
        .then((data) => {
          setText('detail-heap', data.heap_kb);
          setText('detail-uptime', formatUptime(data.uptime_seconds));
          setText('detail-cpu', data.cpu_mhz);
          setText('detail-flash', data.flash_mb);
          setText('detail-sketch', data.firmware_kb);
          setText('detail-clients', data.connected_clients);
          setText('detail-ip', data.ip);
        })
        .catch(() => {
          const heading = document.createElement('p');
          heading.className = 'notice';
          heading.textContent = 'The ESP32 readings could not be loaded. Return to the dashboard and try again.';
          document.getElementById('main-content').appendChild(heading);
        });
    })();
    </script>
)HTML");
  page += pageFooter();
  sendPage(page);
}

void settingsPage()
{
  String page = pageHeader();
  page += F(R"HTML(
    <section class="intro">
      <h1>Settings</h1>
      <p>Local maintenance for this ESP32.</p>
    </section>
    <section class="card">
      <h2>Restart the device</h2>
      <p class="muted">The access point and dashboard will be unavailable for a short time while the board restarts.</p>
      <form method="post" action="/restart" onsubmit="return confirm('Restart the ESP32 now?')">)HTML");
  page += controlTokenField();
  page += F(R"HTML(<button class="button button-red" type="submit">Restart ESP32</button>
      </form>
    </section>
    <section class="card">
      <h2>Access point</h2>
      <p class="muted">The network name and passphrase are set at the top of this sketch. The passphrase is never shown on this page.</p>
    </section>
)HTML");
  page += pageFooter();
  sendPage(page);
}

String svgTextElement(const char* text)
{
  String markup;
  markup.reserve(150);
  markup += F("<text x=\"380\" y=\"150\" text-anchor=\"middle\" fill=\"#a9b8cd\" font-size=\"14\">");
  markup += text;
  markup += F("</text>");
  return markup;
}

void chartsPage()
{
  String page = pageHeader();
  page += F(R"HTML(
    <section class="intro">
      <h1>Recent history</h1>
      <p>Free heap and connected clients sampled about every five seconds.</p>
    </section>
    <section class="card">
      <p id="history-status" class="muted" role="status" aria-live="polite">Loading readings…</p>
      <p class="notice">Each line is scaled independently so you can compare its trend. The chart does not use one shared numeric scale.</p>
    </section>
    <section class="card">
      <div class="legend" aria-label="Chart legend">
        <span style="color:#63b3ff">● Free heap (KB)</span>
        <span style="color:#64d9a0">● Connected clients</span>
      </div>
      <svg id="history-chart" class="chart" viewBox="0 0 760 300" role="img" aria-labelledby="chart-title chart-description">
        <title id="chart-title">Recent ESP32 readings</title>
        <desc id="chart-description">Independent normalized trend lines for free heap and connected clients.</desc>
        <text x="380" y="150" text-anchor="middle" fill="#a9b8cd">Waiting for readings</text>
      </svg>
    </section>
    <script>
    (() => {
      const status = document.getElementById('history-status');
      const chart = document.getElementById('history-chart');
      const svgNamespace = 'http://www.w3.org/2000/svg';
      let loading = false;

      function makeSvgElement(name, attributes, text) {
        const element = document.createElementNS(svgNamespace, name);
        Object.keys(attributes || {}).forEach((key) => element.setAttribute(key, attributes[key]));
        if (text !== undefined) element.textContent = text;
        return element;
      }

      function formatWindow(seconds) {
        const safeSeconds = Math.max(0, Math.floor(seconds));
        if (safeSeconds < 60) return safeSeconds + ' seconds';
        const minutes = Math.floor(safeSeconds / 60);
        const remaining = safeSeconds % 60;
        return remaining ? minutes + ' min ' + remaining + ' sec' : minutes + ' min';
      }

      function render(readings) {
        while (chart.firstChild) chart.removeChild(chart.firstChild);
        chart.appendChild(makeSvgElement('title', { id: 'chart-title' }, 'Recent ESP32 readings'));
        chart.appendChild(makeSvgElement('desc', { id: 'chart-description' },
          'Independent normalized trend lines for free heap and connected clients.'));

        const left = 58;
        const right = 20;
        const top = 25;
        const bottom = 42;
        const width = 760;
        const height = 300;
        const plotWidth = width - left - right;
        const plotHeight = height - top - bottom;

        [0, 0.25, 0.5, 0.75, 1].forEach((fraction) => {
          const y = top + fraction * plotHeight;
          chart.appendChild(makeSvgElement('line', {
            x1: left, y1: y, x2: width - right, y2: y,
            stroke: '#293b54', 'stroke-width': 1
          }));
        });

        if (readings.length === 0) {
          chart.appendChild(makeSvgElement('text', {
            x: width / 2, y: height / 2, 'text-anchor': 'middle',
            fill: '#a9b8cd', 'font-size': 14
          }, 'No readings yet'));
          return;
        }

        function addSeries(key, color) {
          const values = readings.map((reading) => {
            const value = Number(reading[key]);
            return Number.isFinite(value) ? value : 0;
          });
          const minimum = Math.min.apply(null, values);
          const maximum = Math.max.apply(null, values);
          const range = maximum - minimum;
          const path = values.map((value, index) => {
            const x = left + (values.length < 2 ? plotWidth / 2 : index * plotWidth / (values.length - 1));
            const normalized = range === 0 ? 0.5 : (value - minimum) / range;
            const y = top + plotHeight - normalized * plotHeight;
            return (index === 0 ? 'M' : 'L') + x.toFixed(1) + ',' + y.toFixed(1);
          }).join(' ');
          chart.appendChild(makeSvgElement('path', {
            d: path, fill: 'none', stroke: color, 'stroke-width': 3,
            'stroke-linecap': 'round', 'stroke-linejoin': 'round'
          }));
        }

        chart.appendChild(makeSvgElement('text', {
          x: left - 8, y: top + 5, 'text-anchor': 'end', fill: '#a9b8cd', 'font-size': 11
        }, 'high'));
        chart.appendChild(makeSvgElement('text', {
          x: left - 8, y: top + plotHeight, 'text-anchor': 'end', fill: '#a9b8cd', 'font-size': 11
        }, 'low'));
        addSeries('heap_kb', '#63b3ff');
        addSeries('clients', '#64d9a0');
        chart.appendChild(makeSvgElement('text', {
          x: left, y: height - 10, fill: '#a9b8cd', 'font-size': 12
        }, 'Oldest'));
        chart.appendChild(makeSvgElement('text', {
          x: width - right, y: height - 10, 'text-anchor': 'end',
          fill: '#a9b8cd', 'font-size': 12
        }, 'Newest'));
      }

      async function refreshHistory() {
        if (loading) return;
        loading = true;
        try {
          const response = await fetch('/history', { cache: 'no-store' });
          if (!response.ok) throw new Error('History request failed');
          const payload = await response.json();
          const readings = Array.isArray(payload.entries) ? payload.entries : [];
          render(readings);
          if (readings.length === 0) {
            status.textContent = 'No readings yet. The first sample is recorded after five seconds.';
          } else {
            const first = Number(readings[0].seconds) || 0;
            const last = Number(readings[readings.length - 1].seconds) || first;
            const latest = readings[readings.length - 1];
            status.textContent = 'Latest: ' + latest.heap_kb + ' KB free heap · ' +
              latest.clients + ' connected client(s) · ' + readings.length +
              ' readings across ' + formatWindow(last - first) + '.';
          }
        } catch (error) {
          status.textContent = 'History could not be refreshed. Check the ESP32 connection and try again.';
        } finally {
          loading = false;
        }
      }

      refreshHistory();
      window.setInterval(refreshHistory, 5000);
    })();
    </script>
)HTML");
  page += pageFooter();
  sendPage(page);
}

void logsPage()
{
  String page = pageHeader();
  page += F(R"HTML(
    <section class="intro">
      <h1>Recent logs</h1>
      <p>Up to 100 recent memory and client-count samples. These are cleared on restart.</p>
    </section>
    <section class="card table-wrap">
      <table>
        <caption>Latest sample appears at the bottom.</caption>
        <thead><tr><th scope="col">Uptime</th><th scope="col">Free heap</th><th scope="col">Clients</th></tr></thead>
        <tbody>
)HTML");

  const size_t oldestIndex = (nextLogIndex + kLogCapacity - logCount) % kLogCapacity;
  for (size_t offset = 0; offset < logCount; ++offset)
  {
    const size_t index = (oldestIndex + offset) % kLogCapacity;
    page += F("<tr><td>");
    page += formatUptime(logs[index].uptimeSeconds);
    page += F("</td><td>");
    page += String(logs[index].freeHeapKb);
    page += F(" KB</td><td>");
    page += String(logs[index].connectedClients);
    page += F("</td></tr>");
  }

  if (logCount == 0)
  {
    page += F("<tr><td colspan=\"3\">No readings recorded yet. The first sample is recorded after five seconds.</td></tr>");
  }

  page += F("</tbody></table></section>");
  page += pageFooter();
  sendPage(page);
}

void aboutPage()
{
  String page = pageHeader();
  page += F(R"HTML(
    <section class="intro">
      <h1>About ESP32 ControlOS</h1>
      <p>A small, local dashboard for monitoring and controlling an ESP32 board.</p>
    </section>
    <section class="grid">
      <article class="card"><div class="metric-label">PROJECT</div><div class="metric-value">ESP32 ControlOS</div></article>
      <article class="card"><div class="metric-label">VERSION</div><div class="metric-value">6.0</div></article>
      <article class="card"><div class="metric-label">PLATFORM</div><div class="metric-value">ESP32</div></article>
      <article class="card"><div class="metric-label">LANGUAGE</div><div class="metric-value">Arduino C++</div></article>
    </section>
    <section class="card" style="margin-top:1rem">
      <h2>What it can do</h2>
      <ul>
        <li>Show live memory, uptime, processor, storage, and access point readings.</li>
        <li>Scan nearby 2.4 GHz Wi-Fi networks when asked.</li>
        <li>Keep a short history in memory and draw it without a cloud service.</li>
        <li>Control the board’s configured built-in LED and restart the board.</li>
      </ul>
      <p class="notice">This project uses the ESP32 itself. It does not need external sensors, an OLED display, or an internet connection.</p>
    </section>
    <section class="card">
      <h2>Privacy and access</h2>
      <p>The ESP32 creates its own password-protected Wi-Fi network. It does not join your home router and sends no readings to a cloud service. History stays in memory until restart.</p>
      <p class="muted">Anyone who knows the access point password can use the controls, including the restart button. Share that password only with people you trust.</p>
    </section>
    <section class="card">
      <h2>Developer</h2>
      <p>Gnandeep Yenigella</p>
    </section>
)HTML");
  page += pageFooter();
  sendPage(page);
}

void statusApi()
{
  String json;
  json.reserve(220);
  json += F("{\"heap_kb\":");
  json += String(ESP.getFreeHeap() / 1024UL);
  json += F(",\"uptime_seconds\":");
  json += formatUnsigned64(uptimeSeconds());
  json += F(",\"cpu_mhz\":");
  json += String(getCpuFrequencyMhz());
  json += F(",\"flash_mb\":");
  json += String(ESP.getFlashChipSize() / (1024UL * 1024UL));
  json += F(",\"firmware_kb\":");
  json += String(ESP.getSketchSize() / 1024UL);
  json += F(",\"connected_clients\":");
  json += String(WiFi.softAPgetStationNum());
  json += F(",\"led_on\":");
  json += ledIsOn ? F("true") : F("false");
  json += F(",\"ip\":\"");
  json += WiFi.softAPIP().toString();
  json += F("\"}");

  server.sendHeader("Cache-Control", "no-store, max-age=0");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "application/json; charset=utf-8", json);
}

void historyApi()
{
  String json;
  json.reserve(24 + logCount * 72);
  json += F("{\"entries\":[");
  const size_t oldestIndex = (nextLogIndex + kLogCapacity - logCount) % kLogCapacity;

  for (size_t offset = 0; offset < logCount; ++offset)
  {
    const size_t index = (oldestIndex + offset) % kLogCapacity;
    if (offset > 0)
    {
      json += ',';
    }

    json += F("{\"seconds\":");
    json += formatUnsigned64(logs[index].uptimeSeconds);
    json += F(",\"heap_kb\":");
    json += String(logs[index].freeHeapKb);
    json += F(",\"clients\":");
    json += String(logs[index].connectedClients);
    json += '}';
  }

  json += F("]}");
  server.sendHeader("Cache-Control", "no-store, max-age=0");
  server.sendHeader("Pragma", "no-cache");
  server.send(200, "application/json; charset=utf-8", json);
}

void saveLog()
{
  logs[nextLogIndex].uptimeSeconds = uptimeSeconds();
  logs[nextLogIndex].freeHeapKb = ESP.getFreeHeap() / 1024UL;
  logs[nextLogIndex].connectedClients = WiFi.softAPgetStationNum();

  nextLogIndex = (nextLogIndex + 1) % kLogCapacity;
  if (logCount < kLogCapacity)
  {
    ++logCount;
  }
}

void redirectToDashboard()
{
  server.sendHeader("Location", "/");
  server.send(303, "text/plain; charset=utf-8", "");
}

void ledOn()
{
  if (!hasValidControlToken())
  {
    rejectInvalidControlRequest();
    return;
  }

  setLed(true);
  redirectToDashboard();
}

void ledOff()
{
  if (!hasValidControlToken())
  {
    rejectInvalidControlRequest();
    return;
  }

  setLed(false);
  redirectToDashboard();
}

void restartEsp()
{
  if (!hasValidControlToken())
  {
    rejectInvalidControlRequest();
    return;
  }

  server.send(200, "text/html; charset=utf-8",
              "<!doctype html><html lang=\"en\"><meta charset=\"utf-8\">"
              "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
              "<title>Restarting</title><body><p>Restarting the ESP32. Reconnect in a few seconds.</p></body></html>");
  delay(250);
  ESP.restart();
}

void notFound()
{
  String page = pageHeader();
  page += F(R"HTML(
    <section class="card">
      <h1>Page not found</h1>
      <p>The address does not match a page on this ESP32.</p>
      <a class="button" href="/">Return to dashboard</a>
    </section>
)HTML");
  page += pageFooter();
  server.sendHeader("Cache-Control", "no-store, max-age=0");
  server.send(404, "text/html; charset=utf-8", page);
}
} // namespace

void setup()
{
  Serial.begin(115200);
  pinMode(kLedPin, OUTPUT);
  setLed(false);

  Serial.println();
  Serial.println(F("ESP32 ControlOS starting"));

  if (!hasValidAccessPointConfig())
  {
    Serial.println(F("Access point not started. Set a private SSID and a WPA2 passphrase of 8-63 characters in the Wi-Fi settings at the top of the sketch."));
    return;
  }

  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(kAccessPointSsid, kAccessPointPassword))
  {
    Serial.println(F("Access point could not be started. Check the board and Wi-Fi configuration."));
    return;
  }

  controlToken = createControlToken();

  server.on("/", HTTP_GET, dashboardPage);
  server.on("/wifi", HTTP_GET, wifiPage);
  server.on("/system", HTTP_GET, systemPage);
  server.on("/charts", HTTP_GET, chartsPage);
  server.on("/logs", HTTP_GET, logsPage);
  server.on("/settings", HTTP_GET, settingsPage);
  server.on("/about", HTTP_GET, aboutPage);
  server.on("/status", HTTP_GET, statusApi);
  server.on("/history", HTTP_GET, historyApi);
  server.on("/on", HTTP_POST, ledOn);
  server.on("/off", HTTP_POST, ledOff);
  server.on("/restart", HTTP_POST, restartEsp);
  server.onNotFound(notFound);
  server.begin();

  serverReady = true;
  lastSampleMillis = millis();

  Serial.print(F("Wi-Fi network: "));
  Serial.println(kAccessPointSsid);
  Serial.print(F("Dashboard: http://"));
  Serial.println(WiFi.softAPIP());
  Serial.println(F("The Wi-Fi passphrase is not printed."));
}

void loop()
{
  if (!serverReady)
  {
    delay(10);
    return;
  }

  server.handleClient();

  const uint32_t now = millis();
  if (static_cast<uint32_t>(now - lastSampleMillis) >= kSampleIntervalMs)
  {
    saveLog();
    lastSampleMillis = now;
  }
}
