#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#if __has_include("fog_controller_config.h")
#include "fog_controller_config.h"
#else
#include "fog_controller_config.example.h"
#endif

// Arduino Nano ESP32 pin names.
static constexpr uint8_t PIN_BLOWER_PWM = D6;
static constexpr uint8_t PIN_MOSFET_B_PWM_RESERVED = D9;
static constexpr uint8_t PIN_MOSFET_C_PWM_RESERVED = D10;
static constexpr uint8_t PIN_ONEWIRE = D4;
static constexpr uint8_t PIN_BUTTON_START = D2;
static constexpr uint8_t PIN_BUTTON_MODE = D3;
static constexpr uint8_t PIN_BUTTON_TEST = D5;
static constexpr uint8_t PIN_FOG_TRIGGER_RESERVED = D7;
static constexpr uint8_t PIN_PIR_RESERVED = D8;

static constexpr uint8_t OLED_ADDRESS = 0x3C;
static constexpr int OLED_WIDTH = 128;
static constexpr int OLED_HEIGHT = 64;
static constexpr int OLED_RESET = -1;

static constexpr uint8_t MIN_RUN_PERCENT = 25;
static constexpr uint8_t MAX_RUN_PERCENT = 70;
static constexpr uint8_t START_BOOST_PERCENT = 60;
static constexpr uint32_t START_BOOST_MS = 750;
static constexpr uint32_t TEST_RUN_MS = 10000;
static constexpr uint32_t SENSOR_INTERVAL_MS = 2000;
static constexpr uint32_t DISPLAY_INTERVAL_MS = 1000;
static constexpr uint32_t SERIAL_STATUS_INTERVAL_MS = 5000;

enum class Mode { MANUAL, AUTO, TEST };

WebServer server(80);
OneWire oneWire(PIN_ONEWIRE);
DallasTemperature ds18b20(&oneWire);
Adafruit_BME280 bme;
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

Mode mode = Mode::MANUAL;
bool bmeOk = false;
bool oledOk = false;
bool apMode = false;
uint8_t blowerPercent = 0;
uint8_t requestedPercent = 0;
uint32_t boostUntil = 0;
uint32_t testUntil = 0;
uint32_t lastSensorRead = 0;
uint32_t lastDisplay = 0;
uint32_t lastSerialStatus = 0;

float ambientTempC = NAN;
float humidity = NAN;
float pressureHpa = NAN;
float coolerTempC = NAN;
float outletTempC = NAN;

bool lastStartButton = HIGH;
bool lastModeButton = HIGH;
bool lastTestButton = HIGH;
uint32_t lastButtonScan = 0;

const char *modeName(Mode value) {
  switch (value) {
    case Mode::MANUAL: return "manual";
    case Mode::AUTO: return "auto";
    case Mode::TEST: return "test";
  }
  return "unknown";
}

float cToF(float value) {
  return isnan(value) ? NAN : value * 9.0f / 5.0f + 32.0f;
}

String floatOrNull(float value, unsigned int decimals = 1) {
  return isnan(value) ? "null" : String(value, decimals);
}

uint8_t clampPercent(int value) {
  return static_cast<uint8_t>(constrain(value, 0, 100));
}

void writeBlowerRaw(uint8_t percent) {
  blowerPercent = clampPercent(percent);
  const uint8_t pwm = map(blowerPercent, 0, 100, 0, 255);
  analogWrite(PIN_BLOWER_PWM, pwm);
}

void setBlower(uint8_t percent, bool allowBoost = true) {
  requestedPercent = clampPercent(percent);

  if (requestedPercent == 0) {
    boostUntil = 0;
    writeBlowerRaw(0);
    return;
  }

  requestedPercent = constrain(requestedPercent, MIN_RUN_PERCENT, MAX_RUN_PERCENT);

  if (allowBoost && blowerPercent == 0 && requestedPercent < START_BOOST_PERCENT) {
    writeBlowerRaw(START_BOOST_PERCENT);
    boostUntil = millis() + START_BOOST_MS;
  } else {
    boostUntil = 0;
    writeBlowerRaw(requestedPercent);
  }
}

void updateBoost() {
  if (boostUntil != 0 && static_cast<int32_t>(millis() - boostUntil) >= 0) {
    boostUntil = 0;
    writeBlowerRaw(requestedPercent);
  }
}

void readSensors() {
  ds18b20.requestTemperatures();
  const int count = ds18b20.getDeviceCount();

  coolerTempC = count > 0 ? ds18b20.getTempCByIndex(COOLER_SENSOR_INDEX) : NAN;
  outletTempC = count > 1 ? ds18b20.getTempCByIndex(OUTLET_SENSOR_INDEX) : NAN;

  if (coolerTempC <= -126.0f || coolerTempC == 85.0f) coolerTempC = NAN;
  if (outletTempC <= -126.0f || outletTempC == 85.0f) outletTempC = NAN;

  if (bmeOk) {
    ambientTempC = bme.readTemperature();
    humidity = bme.readHumidity();
    pressureHpa = bme.readPressure() / 100.0f;
  }
}

void runAutomaticLogic() {
  if (mode != Mode::AUTO) return;

  if (isnan(coolerTempC)) {
    setBlower(0, false);
    return;
  }

  // Conservative initial rules; tune after cooler characterization.
  if (coolerTempC > 12.0f) {  // 53.6 F
    setBlower(MIN_RUN_PERCENT);
    return;
  }

  float delta = (!isnan(ambientTempC) && !isnan(outletTempC))
      ? ambientTempC - outletTempC
      : 5.0f;

  int command = 30 + static_cast<int>(delta * 1.5f);
  setBlower(constrain(command, MIN_RUN_PERCENT, MAX_RUN_PERCENT), false);
}

String statusJson() {
  String json = "{";
  json += "\"mode\":\"" + String(modeName(mode)) + "\",";
  json += "\"blowerPercent\":" + String(blowerPercent) + ",";
  json += "\"requestedPercent\":" + String(requestedPercent) + ",";
  json += "\"ambientTempF\":" + floatOrNull(cToF(ambientTempC)) + ",";
  json += "\"humidity\":" + floatOrNull(humidity) + ",";
  json += "\"pressureHpa\":" + floatOrNull(pressureHpa) + ",";
  json += "\"coolerTempF\":" + floatOrNull(cToF(coolerTempC)) + ",";
  json += "\"outletTempF\":" + floatOrNull(cToF(outletTempC)) + ",";
  json += "\"wifiRssi\":" + String(WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0) + ",";
  json += "\"apMode\":" + String(apMode ? "true" : "false") + ",";
  json += "\"uptimeSeconds\":" + String(millis() / 1000);
  json += "}";
  return json;
}

const char DASHBOARD[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Fog Controller</title>
<style>
:root{font-family:system-ui,sans-serif;color-scheme:dark;background:#101216;color:#f2f4f8}
body{margin:0;padding:18px;max-width:780px;margin:auto}
h1{font-size:1.45rem}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:12px}
.card{background:#1a1f27;border:1px solid #303845;border-radius:12px;padding:14px}
.value{font-size:1.7rem;font-weight:700}.muted{color:#aab3c0}
.controls{margin-top:16px}button,select,input{font:inherit}
button{padding:12px 16px;margin:4px;border:0;border-radius:9px;background:#dde4ee;color:#111;font-weight:700}
button.stop{background:#ffb1b1}input[type=range]{width:100%}
.status{margin-top:14px;font-size:.9rem}
</style>
</head>
<body>
<h1>Halloween Fog Controller</h1>
<div class="grid">
<div class="card"><div class="muted">Ambient</div><div id="ambient" class="value">--</div></div>
<div class="card"><div class="muted">Humidity</div><div id="humidity" class="value">--</div></div>
<div class="card"><div class="muted">Cooler</div><div id="cooler" class="value">--</div></div>
<div class="card"><div class="muted">Outlet</div><div id="outlet" class="value">--</div></div>
<div class="card"><div class="muted">Blower</div><div id="blower" class="value">--</div></div>
<div class="card"><div class="muted">Mode</div><div id="mode" class="value">--</div></div>
</div>
<div class="card controls">
<label for="speed">Manual blower speed: <span id="speedLabel">35%</span></label>
<input id="speed" type="range" min="0" max="70" value="35">
<div>
<button onclick="setSpeed()">Apply</button>
<button onclick="setMode('manual')">Manual</button>
<button onclick="setMode('auto')">Auto</button>
<button onclick="runTest()">10 s Test</button>
<button class="stop" onclick="stopBlower()">STOP</button>
</div>
</div>
<div id="connection" class="status muted">Connecting…</div>
<script>
const q=id=>document.getElementById(id);
const fmt=(v,suffix)=>v==null?'--':Number(v).toFixed(1)+suffix;
q('speed').oninput=()=>q('speedLabel').textContent=q('speed').value+'%';
async function post(path,body={}) {
  await fetch(path,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(body)});
  await refresh();
}
function setSpeed(){post('/api/blower',{percent:q('speed').value})}
function setMode(mode){post('/api/mode',{mode})}
function runTest(){post('/api/test')}
function stopBlower(){post('/api/stop')}
async function refresh(){
 try{
  const r=await fetch('/api/status',{cache:'no-store'}); const s=await r.json();
  q('ambient').textContent=fmt(s.ambientTempF,' °F');
  q('humidity').textContent=fmt(s.humidity,' %');
  q('cooler').textContent=fmt(s.coolerTempF,' °F');
  q('outlet').textContent=fmt(s.outletTempF,' °F');
  q('blower').textContent=s.blowerPercent+' %';
  q('mode').textContent=s.mode.toUpperCase();
  q('connection').textContent='Connected · RSSI '+s.wifiRssi+' dBm · uptime '+s.uptimeSeconds+' s';
 } catch(e){q('connection').textContent='Disconnected';}
}
setInterval(refresh,2000);refresh();
</script>
</body>
</html>
)HTML";

void handleRoot() {
  server.send_P(200, "text/html", DASHBOARD);
}

void handleStatus() {
  server.send(200, "application/json", statusJson());
}

void handleBlower() {
  if (!server.hasArg("percent")) {
    server.send(400, "application/json", "{\"error\":\"percent required\"}");
    return;
  }
  mode = Mode::MANUAL;
  setBlower(clampPercent(server.arg("percent").toInt()));
  server.send(200, "application/json", statusJson());
}

void handleMode() {
  if (!server.hasArg("mode")) {
    server.send(400, "application/json", "{\"error\":\"mode required\"}");
    return;
  }

  const String requested = server.arg("mode");
  if (requested == "manual") mode = Mode::MANUAL;
  else if (requested == "auto") mode = Mode::AUTO;
  else if (requested == "test") mode = Mode::TEST;
  else {
    server.send(400, "application/json", "{\"error\":\"invalid mode\"}");
    return;
  }

  server.send(200, "application/json", statusJson());
}

void handleStop() {
  mode = Mode::MANUAL;
  setBlower(0, false);
  server.send(200, "application/json", statusJson());
}

void handleTest() {
  mode = Mode::TEST;
  testUntil = millis() + TEST_RUN_MS;
  setBlower(35);
  server.send(200, "application/json", statusJson());
}

void setupRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/healthz", HTTP_GET, []() { server.send(200, "text/plain", "ok"); });
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/blower", HTTP_POST, handleBlower);
  server.on("/api/mode", HTTP_POST, handleMode);
  server.on("/api/stop", HTTP_POST, handleStop);
  server.on("/api/test", HTTP_POST, handleTest);
  server.onNotFound([]() { server.send(404, "application/json", "{\"error\":\"not found\"}"); });
  server.begin();
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(DEVICE_HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const uint32_t deadline = millis() + 15000;
  while (WiFi.status() != WL_CONNECTED && static_cast<int32_t>(millis() - deadline) < 0) {
    delay(250);
    Serial.print('.');
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWi-Fi connected: %s\n", WiFi.localIP().toString().c_str());
    apMode = false;
  } else {
    WiFi.mode(WIFI_AP);
    String ssid = String("FogController-Setup-") + String((uint32_t)ESP.getEfuseMac(), HEX);
    WiFi.softAP(ssid.c_str(), SETUP_AP_PASSWORD);
    Serial.printf("\nSetup AP: %s at %s\n", ssid.c_str(), WiFi.softAPIP().toString().c_str());
    apMode = true;
  }

  if (MDNS.begin(DEVICE_HOSTNAME)) {
    MDNS.addService("http", "tcp", 80);
  }
}

void setupOta() {
  ArduinoOTA.setHostname(DEVICE_HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.onStart([]() {
    setBlower(0, false);
  });
  ArduinoOTA.begin();
}

void logNetworkStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Network: connected, URL: http://%s.local/ or http://%s/\n",
                  DEVICE_HOSTNAME,
                  WiFi.localIP().toString().c_str());
  } else if (apMode) {
    Serial.printf("Network: setup AP active, URL: http://%s/\n",
                  WiFi.softAPIP().toString().c_str());
  } else {
    Serial.println("Network: disconnected");
  }
}

void drawDisplay() {
  if (!oledOk) return;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.printf("FOGBOX  %s\n", modeName(mode));
  display.printf("Amb: %sF  RH:%s%%\n",
                 isnan(ambientTempC) ? "--" : String(cToF(ambientTempC), 0).c_str(),
                 isnan(humidity) ? "--" : String(humidity, 0).c_str());
  display.printf("Cool:%sF Out:%sF\n",
                 isnan(coolerTempC) ? "--" : String(cToF(coolerTempC), 0).c_str(),
                 isnan(outletTempC) ? "--" : String(cToF(outletTempC), 0).c_str());
  display.printf("Blower: %u%%\n", blowerPercent);
  display.printf("%s\n", apMode ? WiFi.softAPIP().toString().c_str() : WiFi.localIP().toString().c_str());
  display.display();
}

bool buttonPressed(uint8_t pin, bool &lastState) {
  const bool current = digitalRead(pin);
  const bool pressed = (lastState == HIGH && current == LOW);
  lastState = current;
  return pressed;
}

void scanButtons() {
  if (millis() - lastButtonScan < 40) return;
  lastButtonScan = millis();

  if (buttonPressed(PIN_BUTTON_START, lastStartButton)) {
    mode = Mode::MANUAL;
    setBlower(blowerPercent == 0 ? 35 : 0);
  }

  if (buttonPressed(PIN_BUTTON_MODE, lastModeButton)) {
    if (mode == Mode::MANUAL) mode = Mode::AUTO;
    else if (mode == Mode::AUTO) mode = Mode::TEST;
    else mode = Mode::MANUAL;
  }

  if (buttonPressed(PIN_BUTTON_TEST, lastTestButton)) {
    mode = Mode::TEST;
    testUntil = millis() + TEST_RUN_MS;
    setBlower(35);
  }
}

void setup() {
  // Establish an OFF state before waiting for USB serial or starting networking.
  pinMode(PIN_BLOWER_PWM, OUTPUT);
  pinMode(PIN_MOSFET_B_PWM_RESERVED, OUTPUT);
  pinMode(PIN_MOSFET_C_PWM_RESERVED, OUTPUT);
  digitalWrite(PIN_BLOWER_PWM, LOW);
  digitalWrite(PIN_MOSFET_B_PWM_RESERVED, LOW);
  digitalWrite(PIN_MOSFET_C_PWM_RESERVED, LOW);

  Serial.begin(115200);
  const uint32_t serialDeadline = millis() + 3000;
  while (!Serial && static_cast<int32_t>(millis() - serialDeadline) < 0) {
    delay(10);
  }

  pinMode(PIN_BUTTON_START, INPUT_PULLUP);
  pinMode(PIN_BUTTON_MODE, INPUT_PULLUP);
  pinMode(PIN_BUTTON_TEST, INPUT_PULLUP);
  pinMode(PIN_FOG_TRIGGER_RESERVED, OUTPUT);
  pinMode(PIN_PIR_RESERVED, INPUT);
  digitalWrite(PIN_FOG_TRIGGER_RESERVED, LOW);
  writeBlowerRaw(0);

  Wire.begin();
  oledOk = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  bmeOk = bme.begin(0x76);
  if (!bmeOk) bmeOk = bme.begin(0x77);

  ds18b20.begin();
  ds18b20.setResolution(11);

  connectWiFi();
  setupRoutes();
  setupOta();
  readSensors();
  drawDisplay();

  Serial.printf("OLED: %s, BME280: %s, DS18B20 count: %d\n",
                oledOk ? "OK" : "missing",
                bmeOk ? "OK" : "missing",
                ds18b20.getDeviceCount());
  logNetworkStatus();
}

void loop() {
  server.handleClient();
  ArduinoOTA.handle();
  scanButtons();
  updateBoost();

  if (mode == Mode::TEST && testUntil != 0 &&
      static_cast<int32_t>(millis() - testUntil) >= 0) {
    testUntil = 0;
    mode = Mode::MANUAL;
    setBlower(0, false);
  }

  if (millis() - lastSensorRead >= SENSOR_INTERVAL_MS) {
    lastSensorRead = millis();
    readSensors();
    runAutomaticLogic();
  }

  if (millis() - lastDisplay >= DISPLAY_INTERVAL_MS) {
    lastDisplay = millis();
    drawDisplay();
  }

  if (millis() - lastSerialStatus >= SERIAL_STATUS_INTERVAL_MS) {
    lastSerialStatus = millis();
    logNetworkStatus();
  }

  delay(2);
}
