// ============================================================
//  esp32_test_mode_AP.ino  v2.4 (Tích hợp mạch Uno)
// ============================================================

#include <WiFi.h>
#include <WebServer.h>
#include <math.h>
#include <pgmspace.h>
#include <time.h>

#include "model_weights.h"
#include "web_ui.h"

// ============================================================
// CẤU HÌNH WIFI ACCESS POINT
// ============================================================
const char* AP_SSID     = "ESP32-WeatherStation";
const char* AP_PASSWORD = "12345678";

#define UTC_OFFSET_SEC  25200L
#define NTP_SERVER      "pool.ntp.org"
#define RAIN_THRESHOLD  1.0f
#define ML_INTERVAL_MS  5000UL

// ============================================================
// CẤU HÌNH UART2 GIAO TIẾP VỚI UNO (Qua HW-221)
// ============================================================
#define ARDUINO_RX_PIN  16   // Nối vào LV1 (Từ TX của Uno)
#define ARDUINO_TX_PIN  17   // Nối vào LV2 (Từ RX của Uno)
#define ARDUINO_BAUD    9600

// ============================================================
// STRUCTS
// ============================================================
struct SensorData {
  int   hour        = 12;
  int   month       = 6;
  float temperature = 28.0f;
  float humidity    = 65.0f;
  float windSpeed   = 1.5f;
  String windDir    = "N/A"; // Thêm biến lưu hướng gió
  int   rain        = 0;     // Thêm biến lưu trạng thái mưa
  bool  valid       = false;
  unsigned long lastUpdate = 0;
};

struct ForecastResult {
  float temp7d[7]   = {0};
  float precip7d[7] = {0};
  float temp24h[24] = {0};
  bool  valid       = false;
};

// ============================================================
// GLOBALS
// ============================================================
SensorData     g_sensor;
ForecastResult g_fc;
volatile bool  g_mlBusy  = false;

WebServer      server(80);
String         uartBuf  = "";
unsigned long  lastML   = 0;

// ============================================================
// MATH HELPERS & ML INFERENCE
// ============================================================
static inline float fclamp(float v, float lo, float hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

void polyExpand(const float x[8], float poly[45]) {
  poly[0] = 1.0f;
  for (int i = 0; i < 8; i++) poly[i + 1] = x[i];
  int idx = 9;
  for (int i = 0; i < 8; i++)
    for (int j = i; j < 8; j++)
      poly[idx++] = x[i] * x[j];
}

void ridgePredict(const float* x, int n_in,
                  const float* W_pgm, const float* b_pgm,
                  float* y, int n_out) {
  for (int o = 0; o < n_out; o++) {
    float acc = pgm_read_float_near(&b_pgm[o]);
    for (int i = 0; i < n_in; i++)
      acc += x[i] * pgm_read_float_near(&W_pgm[i * n_out + o]);
    y[o] = acc;
  }
}

void buildFeatures(const SensorData& s, float feat[8]) {
  int month = (s.valid && s.month >= 1 && s.month <= 12) ? s.month : 6;
  int hour  = (s.valid && s.hour  >= 0 && s.hour  <= 23) ? s.hour  : 12;

  feat[0] = sinf(2.0f * (float)M_PI * month / 12.0f);
  feat[1] = cosf(2.0f * (float)M_PI * month / 12.0f);
  feat[2] = sinf(2.0f * (float)M_PI * hour  / 24.0f);
  feat[3] = cosf(2.0f * (float)M_PI * hour  / 24.0f);
  feat[4] = 1013.25f; // Áp suất hardcode
  feat[5] = s.humidity;
  feat[6] = s.temperature;
  feat[7] = s.windSpeed;
}

void runML(const SensorData& s, ForecastResult& fc) {
  g_mlBusy = true;
  unsigned long t0 = micros();
  float feat[8];
  buildFeatures(s, feat);

  ridgePredict(feat, N_FEAT_LIN, W_7d_temp,  b_7d_temp,  fc.temp7d,  N_OUT_7D);
  ridgePredict(feat, N_FEAT_LIN, W_24h_temp, b_24h_temp, fc.temp24h, N_OUT_24H);

  float dummy7[7];
  float dummy24[24];
  ridgePredict(feat, N_FEAT_LIN, W_7d_clouds,  b_7d_clouds,  dummy7,  N_OUT_7D);
  ridgePredict(feat, N_FEAT_LIN, W_24h_clouds, b_24h_clouds, dummy24, N_OUT_24H);

  float poly[N_FEAT_POLY];
  polyExpand(feat, poly);
  ridgePredict(poly, N_FEAT_POLY, W_7d_precip,  b_7d_precip,  fc.precip7d, N_OUT_7D);
  ridgePredict(poly, N_FEAT_POLY, W_24h_precip, b_24h_precip, dummy24,     N_OUT_24H);

  for (int i = 0; i < 7; i++) {
    fc.precip7d[i] = fclamp(fc.precip7d[i], 0.0f, 300.0f);
  }

  fc.valid = true;
  g_mlBusy = false; 
}

// ============================================================
// PARSE UART PACKET TỪ UNO: *T:28,H:75,S:3.5,D:Dong,R:0#
// ============================================================
bool parsePacket(const String& pkt) {
  int s = pkt.indexOf("*T:");
  int e = pkt.lastIndexOf('#');
  if (s < 0 || e <= s) return false;

  String data = pkt.substring(s, e + 1); 

  int t_start = data.indexOf("T:") + 2;
  int h_start = data.indexOf(",H:") + 3;
  int s_start = data.indexOf(",S:") + 3;
  int d_start = data.indexOf(",D:") + 3;
  int r_start = data.indexOf(",R:") + 3;

  if (t_start < 2 || h_start < 3 || s_start < 3 || d_start < 3 || r_start < 3) return false;

  g_sensor.temperature = data.substring(t_start, h_start - 3).toFloat();
  g_sensor.humidity    = data.substring(h_start, s_start - 3).toFloat();
  g_sensor.windSpeed   = data.substring(s_start, d_start - 3).toFloat();
  g_sensor.windDir     = data.substring(d_start, r_start - 3);
  g_sensor.rain        = data.substring(r_start, data.length() - 1).toInt();

  g_sensor.valid      = true;
  g_sensor.lastUpdate = millis();
  
  Serial.printf("=> Parse OK: Temp:%.1f, Hum:%.1f, Wind:%.1f, Dir:%s, Rain:%d\n", 
                 g_sensor.temperature, g_sensor.humidity, g_sensor.windSpeed, 
                 g_sensor.windDir.c_str(), g_sensor.rain);
                 
  return true;
}

// ============================================================
// UART2 HANDLER LẮNG NGHE UNO
// ============================================================
void handleSerial() {
  while (Serial2.available() > 0) {
    char c = Serial2.read();
    if (c == '\n' || c == '\r') {
      if (uartBuf.length() > 0) {
        if (uartBuf.startsWith("*T:")) {
          parsePacket(uartBuf);
        }
        uartBuf = "";
      }
    } else if (uartBuf.length() < 128) {
      uartBuf += c;
    }
  }
}

// ============================================================
// HTTP HANDLERS
// ============================================================
void handleRoot() {
  server.send_P(200, "text/html", WEB_UI_HTML);
}

void handleData() {
  if (g_mlBusy) {
    server.send(503, "application/json", "{\"error\":\"ML busy, retry\"}");
    return;
  }

  if (!g_sensor.valid || !g_fc.valid) {
    server.send(503, "application/json", "{\"error\":\"No data\"}");
    return;
  }

  float baseTemp = fclamp(g_sensor.temperature, -20.0f, 60.0f);
  float minD0 = g_fc.temp24h[0], maxD0 = g_fc.temp24h[0];

  for (int i = 1; i < 24; i++) {
    if (g_fc.temp24h[i] < minD0) minD0 = g_fc.temp24h[i];
    if (g_fc.temp24h[i] > maxD0) maxD0 = g_fc.temp24h[i];
  }

  float swing = maxD0 - minD0;
  if (swing < 0.5f) {
    swing  = 8.0f;
    minD0  = baseTemp - 4.0f;
    maxD0  = baseTemp + 4.0f;
  }

  float targetSwing = fclamp(swing, 6.0f, 12.0f);
  float scale       = targetSwing / swing;
  float rawMean     = (maxD0 + minD0) / 2.0f;
  float safeMean    = fclamp(rawMean, baseTemp - 3.0f, baseTemp + 3.0f);

  float hourlyMatrix[7][24];
  float minTemp7[7];
  float maxTemp7[7];

  for (int d = 0; d < 7; d++) {
    minTemp7[d] =  999.0f;
    maxTemp7[d] = -999.0f;
    float dayOffset = fclamp(g_fc.temp7d[d] - g_fc.temp7d[0], -4.0f, 4.0f);
    float dayScale  = scale * (1.0f + (d % 3) * 0.05f);

    for (int h = 0; h < 24; h++) {
      float t = safeMean + (g_fc.temp24h[h] - rawMean) * dayScale + dayOffset;
      t = fclamp(t, baseTemp - 12.0f, baseTemp + 12.0f);
      hourlyMatrix[d][h] = t;
      if (t < minTemp7[d]) minTemp7[d] = t;
      if (t > maxTemp7[d]) maxTemp7[d] = t;
    }
  }

  const int JSON_SIZE = 2048;
  char* json = (char*)malloc(JSON_SIZE);
  if (!json) {
    server.send(503, "application/json", "{\"error\":\"OOM\"}");
    return;
  }

  int pos = 0;
  // Đóng gói JSON kèm theo Hướng gió và Trạng thái mưa
  pos += snprintf(json + pos, JSON_SIZE - pos,
    "{\"realtime\":{\"temperature\":%.1f,\"humidity\":%.1f,\"windSpeed\":%.1f,\"windDir\":\"%s\",\"rain\":%d},",
    g_sensor.temperature, g_sensor.humidity, g_sensor.windSpeed, g_sensor.windDir.c_str(), g_sensor.rain);

  pos += snprintf(json + pos, JSON_SIZE - pos, "\"forecast7d\":[");
  for (int d = 0; d < 7; d++) {
    bool isRain = (g_fc.precip7d[d] > RAIN_THRESHOLD);
    pos += snprintf(json + pos, JSON_SIZE - pos,
      "%s{\"date\":\"Day+%d\",\"minTemp\":%.1f,\"maxTemp\":%.1f,\"rain\":%s}",
      d > 0 ? "," : "", d + 1,
      minTemp7[d], maxTemp7[d],
      isRain ? "true" : "false");
  }
  
  pos += snprintf(json + pos, JSON_SIZE - pos, "],\"hourlyForecast\":[");
  for (int d = 0; d < 7; d++) {
    pos += snprintf(json + pos, JSON_SIZE - pos, "%s[", d > 0 ? "," : "");
    for (int h = 0; h < 24; h++) {
      pos += snprintf(json + pos, JSON_SIZE - pos,
        "%s%.1f", h > 0 ? "," : "", hourlyMatrix[d][h]);
    }
    pos += snprintf(json + pos, JSON_SIZE - pos, "]");
  }
  pos += snprintf(json + pos, JSON_SIZE - pos, "]}");

  server.send(200, "application/json", json);
  free(json);
}

// ============================================================
// SETUP
// ============================================================
void setupWiFi() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleData);
  server.onNotFound([]() { server.send(404, "text/plain", "Not Found"); });
  server.begin();
}

void setup() {
  // Cổng Serial (USB) để debug trên máy tính
  Serial.begin(115200);
  
  // Cổng Serial2 (Chân 16, 17) để nhận dữ liệu từ Arduino Uno
  Serial2.begin(ARDUINO_BAUD, SERIAL_8N1, ARDUINO_RX_PIN, ARDUINO_TX_PIN);
  delay(200);

  Serial.println("\nESP32 Weather Station is Booting...");
  
  setupWiFi();
  setupWebServer();
  
  Serial.println("Ready! Dang cho du lieu UART tu Uno qua chan 16...");
}

void loop() {
  server.handleClient();
  handleSerial();

  unsigned long now = millis();
  if (now - lastML > ML_INTERVAL_MS) {
    lastML = now;
    if (g_sensor.valid) {
      runML(g_sensor, g_fc);
    }
  }
}