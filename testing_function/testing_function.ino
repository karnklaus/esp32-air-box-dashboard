#include <Arduino.h>
#include <HardwareSerial.h>
#include <HTTPClient.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <qrcode.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>
#include <driver/i2s.h>

// Combined demo:
// - PMS5003 via UART
// - INMP441 via I2S
// - OLED NFP1315-157B via I2C
// - ESP32 config portal via Wi-Fi AP + WebServer
//
// Pin plan
// OLED SDA -> GPIO22
// OLED SCL -> GPIO23
//
// PMS5003 VCC -> 5V
// PMS5003 GND -> GND
// PMS5003 TXD -> GPIO16
// PMS5003 RXD -> GPIO17 (optional)
//
// INMP441 VDD -> 3V3
// INMP441 GND -> GND
// INMP441 SCK -> GPIO27
// INMP441 WS  -> GPIO14
// INMP441 SD  -> GPIO26
// INMP441 L/R -> GPIO12 (reserved, not used in code)
//
// Config button
// RED BUTTON -> GPIO33 to GND (uses internal pull-up)

constexpr uint32_t USB_BAUD = 115200;

constexpr uint8_t OLED_ADDR = 0x3C;
constexpr uint8_t OLED_WIDTH = 128;
constexpr uint8_t OLED_PAGES = 8;
constexpr int OLED_SDA_PIN = 22;
constexpr int OLED_SCL_PIN = 23;

constexpr int PMS_RX_PIN = 16;
constexpr int PMS_TX_PIN = 17;
constexpr uint32_t PMS_BAUD = 9600;

constexpr i2s_port_t I2S_PORT = I2S_NUM_0;
constexpr int MIC_BCLK_PIN = 27;
constexpr int MIC_WS_PIN = 14;
constexpr int MIC_SD_PIN = 26;
constexpr uint32_t MIC_SAMPLE_RATE = 16000;
constexpr size_t MIC_BLOCK_SAMPLES = 256;

constexpr int CONFIG_BUTTON_PIN = 33;
constexpr unsigned long CONFIG_HOLD_MS = 1500;
constexpr unsigned long OLED_REFRESH_MS = 300;
constexpr unsigned long SERIAL_STATUS_MS = 1000;
constexpr unsigned long REPORT_INTERVAL_MS = 3000;
constexpr unsigned long SENSOR_RECOVERY_MS = 2000;
constexpr unsigned long WIFI_RETRY_AFTER_FAIL_MS = 600000;
constexpr int SOUND_THRESHOLD = 100000;
constexpr int SOUND_GAUGE_MAX = SOUND_THRESHOLD;
constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 12000;

HardwareSerial pmsSerial(2);
Preferences preferences;
WebServer configServer(80);
DNSServer dnsServer;

struct DeviceSettings {
  String boxId;
  String wifiSsid;
  String wifiPassword;
  String serverIp;
  uint16_t serverPort;
};

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um;
  uint16_t particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};

pms5003data data = {};
DeviceSettings settings = {};

uint8_t screenBuffer[OLED_WIDTH * OLED_PAGES];
int32_t micSamples[MIC_BLOCK_SAMPLES];

int32_t micEnvelope = 0;
int32_t micPeak = 0;
int32_t reportPeakMax = 0;
bool soundLoud = false;
bool reportSawLoud = false;
bool configMode = false;
bool wifiConnected = false;
bool configButtonLatched = false;
bool configPortalReady = false;
bool configQrView = false;
uint16_t reportPm1Max = 0;
uint16_t reportPm25Max = 0;
uint16_t reportPm10Max = 0;
unsigned long configButtonPressMs = 0;
unsigned long lastOledRefreshMs = 0;
unsigned long lastReportMs = 0;
unsigned long lastSerialStatusMs = 0;
unsigned long rebootAfterSaveMs = 0;
unsigned long sensorRecoveryUntilMs = 0;
unsigned long nextWifiRetryMs = 0;
String configApSsid = "";
String configApIp = "";
String configStatusLine = "";
int qrDrawOriginX = 0;
int qrDrawOriginY = 0;

const uint8_t digits3x5[][5] = {
  {0b111, 0b101, 0b101, 0b101, 0b111},
  {0b010, 0b110, 0b010, 0b010, 0b111},
  {0b111, 0b001, 0b111, 0b100, 0b111},
  {0b111, 0b001, 0b111, 0b001, 0b111},
  {0b101, 0b101, 0b111, 0b001, 0b001},
  {0b111, 0b100, 0b111, 0b001, 0b111},
  {0b111, 0b100, 0b111, 0b101, 0b111},
  {0b111, 0b001, 0b010, 0b010, 0b010},
  {0b111, 0b101, 0b111, 0b101, 0b111},
  {0b111, 0b101, 0b111, 0b001, 0b111}
};

const uint8_t font5x7[][7] = {
  {0b01110, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001, 0b10001}, // A
  {0b11110, 0b10001, 0b11110, 0b10001, 0b10001, 0b10001, 0b11110}, // B
  {0b01111, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b01111}, // C
  {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110}, // D
  {0b11111, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000, 0b11111}, // E
  {0b11111, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000, 0b10000}, // F
  {0b01111, 0b10000, 0b10000, 0b10011, 0b10001, 0b10001, 0b01110}, // G
  {0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001, 0b10001}, // H
  {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b11111}, // I
  {0b00001, 0b00001, 0b00001, 0b00001, 0b10001, 0b10001, 0b01110}, // J
  {0b10001, 0b10010, 0b11100, 0b10010, 0b10001, 0b10001, 0b10001}, // K
  {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111}, // L
  {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001}, // M
  {0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b10001}, // N
  {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}, // O
  {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000}, // P
  {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101}, // Q
  {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001}, // R
  {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110}, // S
  {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}, // T
  {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}, // U
  {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100}, // V
  {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010}, // W
  {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001}, // X
  {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100}, // Y
  {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111}  // Z
};

String getBoxId() {
  uint64_t mac = ESP.getEfuseMac();
  char boxId[20];
  snprintf(
    boxId,
    sizeof(boxId),
    "ABX-%02X%02X%02X%02X",
    static_cast<uint8_t>(mac >> 24),
    static_cast<uint8_t>(mac >> 16),
    static_cast<uint8_t>(mac >> 8),
    static_cast<uint8_t>(mac)
  );
  return String(boxId);
}

String currentBoxId() {
  if (!settings.boxId.isEmpty()) {
    return settings.boxId;
  }
  return getBoxId();
}

String getMacBasedBoxId() {
  uint64_t mac = ESP.getEfuseMac();
  char boxId[20];
  snprintf(
    boxId,
    sizeof(boxId),
    "ABX-%02X%02X%02X%02X",
    static_cast<uint8_t>(mac >> 24),
    static_cast<uint8_t>(mac >> 16),
    static_cast<uint8_t>(mac >> 8),
    static_cast<uint8_t>(mac)
  );
  return String(boxId);
}

String configHoldSecondsLabel() {
  return String(CONFIG_HOLD_MS / 1000);
}

String defaultApSsid() {
  String suffix = currentBoxId();
  suffix.replace("-", "");
  return "AIRBOX-" + suffix.substring(max(0, static_cast<int>(suffix.length()) - 6));
}

String htmlEscape(const String &value) {
  String escaped = value;
  escaped.replace("&", "&amp;");
  escaped.replace("\"", "&quot;");
  escaped.replace("<", "&lt;");
  escaped.replace(">", "&gt;");
  return escaped;
}

String wifiOptionsHtml(const String &selectedSsid) {
  String options;
  int networkCount = WiFi.scanNetworks();

  options += F("<option value=''>Select WiFi...</option>");
  for (int i = 0; i < networkCount; ++i) {
    String ssid = WiFi.SSID(i);
    if (ssid.isEmpty()) {
      continue;
    }

    options += F("<option value='");
    options += htmlEscape(ssid);
    options += F("'");
    if (ssid == selectedSsid) {
      options += F(" selected");
    }
    options += F(">");
    options += htmlEscape(ssid);
    options += F(" (");
    options += String(WiFi.RSSI(i));
    options += F(" dBm)");
    options += F("</option>");
  }

  WiFi.scanDelete();
  return options;
}

void loadSettings() {
  preferences.begin("airbox", true);
  settings.boxId = preferences.getString("box_id", getMacBasedBoxId());
  settings.wifiSsid = preferences.getString("wifi_ssid", "");
  settings.wifiPassword = preferences.getString("wifi_pass", "");
  settings.serverIp = preferences.getString("server_ip", "");
  settings.serverPort = preferences.getUShort("server_port", 5000);
  preferences.end();
  settings.boxId.trim();
  if (settings.boxId.isEmpty()) {
    settings.boxId = getMacBasedBoxId();
  }
  settings.serverIp.trim();
  if (settings.serverPort == 0) {
    settings.serverPort = 5000;
  }
}

void saveSettings(const DeviceSettings &newSettings) {
  preferences.begin("airbox", false);
  preferences.putString("box_id", newSettings.boxId);
  preferences.putString("wifi_ssid", newSettings.wifiSsid);
  preferences.putString("wifi_pass", newSettings.wifiPassword);
  preferences.putString("server_ip", newSettings.serverIp);
  preferences.putUShort("server_port", newSettings.serverPort);
  preferences.end();
  settings = newSettings;
}

bool oledCommand(uint8_t command) {
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x00);
  Wire.write(command);
  return Wire.endTransmission() == 0;
}

bool oledData(const uint8_t *data, size_t length) {
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x40);
  for (size_t i = 0; i < length; ++i) {
    Wire.write(data[i]);
  }
  return Wire.endTransmission() == 0;
}

bool initOLED() {
  const uint8_t commands[] = {
    0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
    0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,
    0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6,
    0x2E, 0xAF
  };

  for (size_t i = 0; i < sizeof(commands); ++i) {
    if (!oledCommand(commands[i])) {
      return false;
    }
  }
  return true;
}

void clearBuffer() {
  memset(screenBuffer, 0, sizeof(screenBuffer));
}

void setPixel(int x, int y, bool on) {
  if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= 64) {
    return;
  }
  const size_t index = x + (y / 8) * OLED_WIDTH;
  const uint8_t mask = 1 << (y % 8);
  if (on) {
    screenBuffer[index] |= mask;
  } else {
    screenBuffer[index] &= ~mask;
  }
}

void fillRect(int x, int y, int w, int h, bool on) {
  for (int yy = y; yy < y + h; ++yy) {
    for (int xx = x; xx < x + w; ++xx) {
      setPixel(xx, yy, on);
    }
  }
}

void drawRect(int x, int y, int w, int h, bool on) {
  for (int xx = x; xx < x + w; ++xx) {
    setPixel(xx, y, on);
    setPixel(xx, y + h - 1, on);
  }
  for (int yy = y; yy < y + h; ++yy) {
    setPixel(x, yy, on);
    setPixel(x + w - 1, yy, on);
  }
}

void drawDigit3x5(int x, int y, char c, uint8_t scale) {
  if (c < '0' || c > '9') {
    return;
  }
  const uint8_t *glyph = digits3x5[c - '0'];
  for (uint8_t row = 0; row < 5; ++row) {
    for (uint8_t col = 0; col < 3; ++col) {
      if ((glyph[row] >> (2 - col)) & 0x01) {
        fillRect(x + col * scale, y + row * scale, scale, scale, true);
      }
    }
  }
}

void drawChar5x7(int x, int y, char c, uint8_t scale) {
  if (c == ' ') {
    return;
  }
  if (c == '.') {
    fillRect(x + 2 * scale, y + 6 * scale, scale, scale, true);
    return;
  }
  if (c == '-') {
    fillRect(x + scale, y + 3 * scale, 3 * scale, scale, true);
    return;
  }
  if (c == ':') {
    fillRect(x + 2 * scale, y + 2 * scale, scale, scale, true);
    fillRect(x + 2 * scale, y + 5 * scale, scale, scale, true);
    return;
  }
  if (c >= '0' && c <= '9') {
    drawDigit3x5(x + scale, y + scale, c, scale);
    return;
  }
  if (c >= 'a' && c <= 'z') {
    c = static_cast<char>(c - 'a' + 'A');
  }
  if (c >= 'A' && c <= 'Z') {
    const uint8_t *glyph = font5x7[c - 'A'];
    for (uint8_t row = 0; row < 7; ++row) {
      for (uint8_t col = 0; col < 5; ++col) {
        if ((glyph[row] >> (4 - col)) & 0x01) {
          fillRect(x + col * scale, y + row * scale, scale, scale, true);
        }
      }
    }
  }
}

int textWidth(const char *text, uint8_t scale) {
  int width = 0;
  while (*text) {
    switch (*text) {
      case '.':
        width += 2 * scale;
        break;
      case '-':
      case ':':
        width += 6 * scale;
        break;
      default:
        width += 6 * scale;
        break;
    }
    ++text;
  }
  return width;
}

void drawText(int x, int y, const char *text, uint8_t scale) {
  int cursorX = x;
  while (*text) {
    const char current = *text;
    drawChar5x7(cursorX, y, current, scale);
    cursorX += (current == '.') ? 2 * scale : 6 * scale;
    ++text;
  }
}

void drawTextLine(int x, int y, const String &text, uint8_t scale) {
  drawText(x, y, text.c_str(), scale);
}

void drawNumber(int x, int y, uint16_t value, uint8_t scale) {
  char buf[6];
  snprintf(buf, sizeof(buf), "%u", value);
  int cursorX = x;
  for (uint8_t i = 0; buf[i] != '\0'; ++i) {
    drawDigit3x5(cursorX, y, buf[i], scale);
    cursorX += 4 * scale;
  }
}

bool flushOLED() {
  for (uint8_t page = 0; page < OLED_PAGES; ++page) {
    if (!oledCommand(0xB0 | page) || !oledCommand(0x00) || !oledCommand(0x10)) {
      return false;
    }
    for (uint8_t block = 0; block < 8; ++block) {
      if (!oledData(&screenBuffer[page * OLED_WIDTH + block * 16], 16)) {
        return false;
      }
    }
  }
  return true;
}

bool readPMSdata(HardwareSerial *s) {
  if (!s->available()) {
    return false;
  }

  if (s->peek() != 0x42) {
    s->read();
    return false;
  }

  if (s->available() < 32) {
    return false;
  }

  uint8_t buffer[32];
  uint16_t sum = 0;
  s->readBytes(buffer, 32);

  for (uint8_t i = 0; i < 30; ++i) {
    sum += buffer[i];
  }

  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; ++i) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }

  pms5003data candidate = {};
  memcpy((void *)&candidate, (void *)buffer_u16, 30);

  if (sum != candidate.checksum) {
    Serial.println("Checksum failure");
    return false;
  }

  data = candidate;
  return true;
}

void clearPmsInputBuffer() {
  while (pmsSerial.available()) {
    pmsSerial.read();
  }
}

bool initMicrophone() {
  const i2s_config_t config = {
    .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = MIC_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 128,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  const i2s_pin_config_t pins = {
    .bck_io_num = MIC_BCLK_PIN,
    .ws_io_num = MIC_WS_PIN,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = MIC_SD_PIN
  };

  if (i2s_driver_install(I2S_PORT, &config, 0, nullptr) != ESP_OK) {
    return false;
  }
  if (i2s_set_pin(I2S_PORT, &pins) != ESP_OK) {
    return false;
  }
  return i2s_zero_dma_buffer(I2S_PORT) == ESP_OK;
}

void updateMicrophoneState() {
  size_t bytesRead = 0;
  if (i2s_read(I2S_PORT, micSamples, sizeof(micSamples), &bytesRead, 10 / portTICK_PERIOD_MS) != ESP_OK) {
    return;
  }

  const size_t sampleCount = bytesRead / sizeof(int32_t);
  if (sampleCount == 0) {
    return;
  }

  int32_t blockPeak = 0;
  for (size_t i = 0; i < sampleCount; ++i) {
    int32_t sample = micSamples[i] >> 8;
    int32_t absSample = abs(sample);
    if (absSample > blockPeak) {
      blockPeak = absSample;
    }
  }

  micPeak = blockPeak;
  if (blockPeak > micEnvelope) {
    micEnvelope = static_cast<int32_t>(micEnvelope * 0.60f + blockPeak * 0.40f);
  } else {
    micEnvelope = static_cast<int32_t>(micEnvelope * 0.90f + blockPeak * 0.10f);
  }
  soundLoud = blockPeak > SOUND_THRESHOLD;
}

void renderHeader() {
  drawRect(0, 0, 128, 64, true);
  drawText(6, 4, "AIR BOX", 1);
  const char *status = soundLoud ? "LOUD" : "QUIET";
  int pillWidth = textWidth(status, 1) + 8;
  int pillX = 128 - pillWidth - 6;
  drawRect(pillX, 2, pillWidth, 11, true);
  drawText(pillX + 4, 4, status, 1);
  for (int x = 5; x < 123; x += 4) {
    setPixel(x, 14, true);
  }
}

void renderValueCard(int x, int y, int w, int h, const char *title, uint16_t value) {
  drawRect(x, y, w, h, true);
  drawText(x + 3, y + 4, title, 1);

  char buf[6];
  snprintf(buf, sizeof(buf), "%u", value);
  int numberWidth = static_cast<int>(strlen(buf)) * 8 - 2;
  int numberX = x + (w - numberWidth) / 2;
  drawNumber(numberX, y + 18, value, 2);
}

void renderSoundBar() {
  drawText(6, 57, "SND", 1);
  drawRect(28, 54, 94, 7, true);
  int barWidth = map(constrain(micPeak, 0, SOUND_GAUGE_MAX), 0, SOUND_GAUGE_MAX, 0, 92);
  fillRect(29, 55, barWidth, 5, true);
}

void renderOLED() {
  clearBuffer();
  renderHeader();
  renderValueCard(4, 20, 38, 30, "PM1.0", data.pm10_env);
  renderValueCard(45, 20, 38, 30, "PM2.5", data.pm25_env);
  renderValueCard(86, 20, 38, 30, "PM10", data.pm100_env);
  renderSoundBar();
  flushOLED();
}

void renderConfigInfoOLED(const String &ssid, const String &ipLine, const String &statusLine) {
  clearBuffer();
  drawRect(0, 0, 128, 64, true);
  drawTextLine(6, 4, "CONFIG MODE", 1);
  drawTextLine(6, 18, "SSID", 1);
  drawTextLine(34, 18, ssid, 1);
  drawTextLine(6, 30, "WEB", 1);
  drawTextLine(30, 30, ipLine, 1);
  drawTextLine(6, 44, "STATUS", 1);
  drawTextLine(6, 54, statusLine, 1);
  flushOLED();
}

void drawQrModules(esp_qrcode_handle_t qrcode) {
  const int size = esp_qrcode_get_size(qrcode);
  constexpr int moduleSize = 2;
  constexpr int quietZone = 1;
  const int totalSize = (size + quietZone * 2) * moduleSize;

  fillRect(qrDrawOriginX, qrDrawOriginY, totalSize, totalSize, false);
  drawRect(qrDrawOriginX, qrDrawOriginY, totalSize, totalSize, true);

  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      if (esp_qrcode_get_module(qrcode, x, y)) {
        fillRect(
          qrDrawOriginX + (x + quietZone) * moduleSize,
          qrDrawOriginY + (y + quietZone) * moduleSize,
          moduleSize,
          moduleSize,
          true
        );
      }
    }
  }
}

void renderQrCode(int originX, int originY, const String &payload) {
  qrDrawOriginX = originX;
  qrDrawOriginY = originY;

  esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
  cfg.display_func = drawQrModules;
  cfg.max_qrcode_version = 4;
  cfg.qrcode_ecc_level = ESP_QRCODE_ECC_LOW;
  esp_qrcode_generate(&cfg, payload.c_str());
}

void renderConfigQrOLED(const String &ssid, const String &ipLine) {
  clearBuffer();
  drawRect(0, 0, 128, 64, true);
  drawTextLine(78, 10, "WIFI", 1);
  drawTextLine(78, 22, ssid, 1);
  renderQrCode(6, 6, "http://" + ipLine);
  flushOLED();
}

void renderConfigOLED(const String &ssid, const String &ipLine, const String &statusLine) {
  configApSsid = ssid;
  configApIp = ipLine;
  configStatusLine = statusLine;

  if (configQrView) {
    renderConfigQrOLED(ssid, ipLine);
    return;
  }
  renderConfigInfoOLED(ssid, ipLine, statusLine);
}

void renderWifiConnectOLED(const String &ssid, const String &statusLine) {
  clearBuffer();
  drawRect(0, 0, 128, 64, true);
  drawTextLine(6, 4, "CONFIG MODE", 1);
  drawTextLine(6, 18, "SSID", 1);
  drawTextLine(34, 18, ssid, 1);
  drawTextLine(6, 30, "WEB", 1);
  drawTextLine(30, 30, configApIp.isEmpty() ? "192.168.4.1" : configApIp, 1);
  drawTextLine(6, 44, "STATUS", 1);
  drawTextLine(6, 54, statusLine, 1);
  flushOLED();
}

bool connectToSavedWifi() {
  if (settings.wifiSsid.isEmpty()) {
    Serial.println("WiFi skipped: no saved SSID");
    wifiConnected = false;
    return false;
  }

  WiFi.disconnect(true, true);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(settings.wifiSsid.c_str(), settings.wifiPassword.c_str());

  Serial.print("Connecting to WiFi SSID: ");
  Serial.println(settings.wifiSsid);
  renderWifiConnectOLED(settings.wifiSsid, "CONNECTING...");

  unsigned long started = millis();
  int dotCount = 0;
  while (WiFi.status() != WL_CONNECTED && millis() - started < WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
    Serial.print(".");
    dotCount = (dotCount + 1) % 7;
    String statusLine = "CONNECTING";
    for (int i = 0; i < dotCount; ++i) {
      statusLine += ".";
    }
    renderWifiConnectOLED(settings.wifiSsid, statusLine);
  }
  Serial.println();

  wifiConnected = WiFi.status() == WL_CONNECTED;
  if (wifiConnected) {
    nextWifiRetryMs = 0;
    Serial.print("WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
    renderWifiConnectOLED(settings.wifiSsid, "COMPLETE");
    delay(700);
  } else {
    nextWifiRetryMs = millis() + WIFI_RETRY_AFTER_FAIL_MS;
    Serial.println("WiFi connect failed");
    renderWifiConnectOLED(settings.wifiSsid, "CONNECTION FAIL");
    delay(900);
  }
  return wifiConnected;
}

String reportUrl() {
  if (settings.serverIp.isEmpty()) {
    return "";
  }
  return "http://" + settings.serverIp + ":" + String(settings.serverPort) + "/api/air-boxes/report";
}

void updateReportWindow() {
  if (data.pm10_env > reportPm1Max) {
    reportPm1Max = data.pm10_env;
  }
  if (data.pm25_env > reportPm25Max) {
    reportPm25Max = data.pm25_env;
  }
  if (data.pm100_env > reportPm10Max) {
    reportPm10Max = data.pm100_env;
  }
  if (micPeak > reportPeakMax) {
    reportPeakMax = micPeak;
  }
  if (soundLoud) {
    reportSawLoud = true;
  }
}

void resetReportWindow() {
  reportPm1Max = data.pm10_env;
  reportPm25Max = data.pm25_env;
  reportPm10Max = data.pm100_env;
  reportPeakMax = micPeak;
  reportSawLoud = soundLoud;
}

bool ensureWifiConnected() {
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    nextWifiRetryMs = 0;
    return true;
  }
  wifiConnected = false;

  if (nextWifiRetryMs != 0 && millis() < nextWifiRetryMs) {
    return false;
  }

  return connectToSavedWifi();
}

void postSensorData() {
  const String url = reportUrl();
  if (url.isEmpty()) {
    Serial.println("POST skipped: server IP not set");
    return;
  }

  if (!ensureWifiConnected()) {
    Serial.println("POST skipped: WiFi offline");
    return;
  }

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String payload;
  payload.reserve(160);
  payload += "{\"id\":\"";
  payload += currentBoxId();
  payload += "\",\"pm1_0\":";
  payload += String(reportPm1Max);
  payload += ",\"pm2_5\":";
  payload += String(reportPm25Max);
  payload += ",\"pm10\":";
  payload += String(reportPm10Max);
  payload += ",\"sound_state\":\"";
  payload += (reportSawLoud ? "LOUD" : "QUIET");
  payload += "\"}";

  const int httpCode = http.POST(payload);
  Serial.print("POST ");
  Serial.print(url);
  Serial.print(" -> ");
  Serial.println(httpCode);
  if (httpCode > 0) {
    String response = http.getString();
    Serial.println(response);
  }
  http.end();

  resetReportWindow();
}

void handleConfigRoot() {
  String html;
  html.reserve(3200);
  String wifiOptions = wifiOptionsHtml(settings.wifiSsid);
  html += F("<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>");
  html += F("<title>Air Box Config</title><style>");
  html += F("body{font-family:Arial,sans-serif;background:#eef5fc;margin:0;padding:24px;color:#102744;}");
  html += F(".wrap{max-width:480px;margin:0 auto;background:#fff;border:1px solid #c9ddf4;padding:20px;}");
  html += F("h1{margin:0 0 8px;font-size:24px;}p{color:#5a7696;}label{display:block;margin-top:14px;font-size:12px;font-weight:700;letter-spacing:.08em;text-transform:uppercase;}");
  html += F("input,select{width:100%;padding:12px;border:1px solid #c9ddf4;margin-top:6px;box-sizing:border-box;background:#fff;}");
  html += F("button{margin-top:18px;width:100%;padding:12px;border:0;background:#198cff;color:#fff;font-weight:700;}");
  html += F("</style></head><body><div class='wrap'>");
  html += F("<h1>Air Box Config</h1>");
  html += F("<p>Connect with your phone, edit box id and WiFi, then save to reboot.</p>");
  html += F("<form method='POST' action='/save'>");
  html += F("<label>Box ID</label><input name='box_id' value='");
  html += htmlEscape(currentBoxId());
  html += F("'>");
  html += F("<label>WiFi SSID</label><select name='wifi_ssid'>");
  html += wifiOptions;
  html += F("</select>");
  html += F("<label>WiFi Password</label><input name='wifi_password' type='password' value='");
  html += htmlEscape(settings.wifiPassword);
  html += F("'>");
  html += F("<label>Server IP</label><input name='server_ip' value='");
  html += htmlEscape(settings.serverIp);
  html += F("'>");
  html += F("<label>Server Port</label><input name='server_port' type='number' min='1' max='65535' value='");
  html += String(settings.serverPort);
  html += F("'>");
  html += F("<button type='submit'>Save and Reboot</button></form></div></body></html>");
  configServer.send(200, "text/html", html);
}

void handleConfigSave() {
  DeviceSettings newSettings = settings;
  newSettings.boxId = configServer.arg("box_id");
  newSettings.boxId.trim();
  if (newSettings.boxId.isEmpty()) {
    newSettings.boxId = getMacBasedBoxId();
  }
  newSettings.wifiSsid = configServer.arg("wifi_ssid");
  newSettings.wifiSsid.trim();
  newSettings.wifiPassword = configServer.arg("wifi_password");
  newSettings.serverIp = configServer.arg("server_ip");
  newSettings.serverIp.trim();
  const String serverPortArg = configServer.arg("server_port");
  const long parsedPort = serverPortArg.toInt();
  if (parsedPort > 0 && parsedPort <= 65535) {
    newSettings.serverPort = static_cast<uint16_t>(parsedPort);
  } else {
    newSettings.serverPort = 5000;
  }

  saveSettings(newSettings);

  configQrView = false;
  renderConfigOLED(defaultApSsid(), WiFi.softAPIP().toString(), "COMPLETE");
  configServer.send(200, "text/html",
    "<!doctype html><html><body style='font-family:Arial;padding:24px'>"
    "<h2>Saved</h2><p>Configuration saved. Device will reboot in 2 seconds.</p></body></html>");
  rebootAfterSaveMs = millis() + 2000;
}

void handleCaptivePortal() {
  handleConfigRoot();
}

void endConfigPortal() {
  configQrView = false;
  dnsServer.stop();
  configServer.stop();
  configPortalReady = false;
  configMode = false;
  rebootAfterSaveMs = 0;
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  wifiConnected = connectToSavedWifi();
  clearPmsInputBuffer();
  sensorRecoveryUntilMs = millis() + SENSOR_RECOVERY_MS;
  resetReportWindow();
  lastReportMs = millis();
  Serial.println("CONFIG MODE disabled");
}

void beginConfigPortal() {
  WiFi.disconnect(true, true);
  delay(100);
  WiFi.mode(WIFI_AP_STA);

  String apSsid = defaultApSsid();
  WiFi.softAP(apSsid.c_str());
  IPAddress ip = WiFi.softAPIP();

  dnsServer.stop();
  dnsServer.start(53, "*", ip);

  configServer.stop();
  configServer.on("/", HTTP_GET, handleConfigRoot);
  configServer.on("/save", HTTP_POST, handleConfigSave);
  configServer.on("/generate_204", HTTP_ANY, handleCaptivePortal);
  configServer.on("/hotspot-detect.html", HTTP_ANY, handleCaptivePortal);
  configServer.on("/connecttest.txt", HTTP_ANY, handleCaptivePortal);
  configServer.on("/ncsi.txt", HTTP_ANY, handleCaptivePortal);
  configServer.on("/fwlink", HTTP_ANY, handleCaptivePortal);
  configServer.onNotFound(handleCaptivePortal);
  configServer.begin();

  configMode = true;
  configPortalReady = true;
  configQrView = false;
  rebootAfterSaveMs = 0;

  Serial.println("CONFIG MODE enabled");
  Serial.print("Config AP SSID: ");
  Serial.println(apSsid);
  Serial.print("Open browser at: http://");
  Serial.println(ip);
  Serial.println("Captive portal enabled: phone should open config page automatically");
  Serial.println("Hold red button " + configHoldSecondsLabel() + "s again to exit config mode");

  renderConfigOLED(apSsid, ip.toString(), "WAITING");
}

void updateConfigButton() {
  const bool pressed = digitalRead(CONFIG_BUTTON_PIN) == LOW;

  if (!pressed) {
    if (configButtonPressMs != 0 && !configButtonLatched && configMode) {
      configQrView = !configQrView;
      renderConfigOLED(configApSsid, configApIp, configStatusLine);
    }
    configButtonLatched = false;
    configButtonPressMs = 0;
    return;
  }

  if (configButtonLatched) {
    return;
  }

  if (configButtonPressMs == 0) {
    configButtonPressMs = millis();
    return;
  }

  if (millis() - configButtonPressMs >= CONFIG_HOLD_MS) {
    configButtonLatched = true;
    if (configMode) {
      endConfigPortal();
    } else {
      beginConfigPortal();
    }
  }
}

void printStatus() {
  Serial.print("PM1.0=");
  Serial.print(data.pm10_env);
  Serial.print(" PM2.5=");
  Serial.print(data.pm25_env);
  Serial.print(" PM10=");
  Serial.print(data.pm100_env);
  Serial.print(" MIC_ENV=");
  Serial.print(micEnvelope);
  Serial.print(" MIC_PEAK=");
  Serial.print(micPeak);
  Serial.print(" SOUND=");
  Serial.print(soundLoud ? "LOUD" : "QUIET");
  Serial.print(" REPORT_PM2.5_MAX=");
  Serial.print(reportPm25Max);
  Serial.print(" REPORT_PEAK_MAX=");
  Serial.print(reportPeakMax);
  Serial.print(" REPORT_SOUND=");
  Serial.print(reportSawLoud ? "LOUD" : "QUIET");
  Serial.print(" WIFI=");
  Serial.println(wifiConnected ? "ONLINE" : "OFFLINE");
}

void setup() {
  pinMode(CONFIG_BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(USB_BAUD);
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  Wire.setClock(100000);
  pmsSerial.begin(PMS_BAUD, SERIAL_8N1, PMS_RX_PIN, PMS_TX_PIN);

  loadSettings();

  Serial.println();
  Serial.println("ESP32 air box demo");
  Serial.println("PMS5003 + INMP441 + OLED");
  Serial.print("BOX_ID: ");
  Serial.println(currentBoxId());
  Serial.print("SERVER_IP: ");
  Serial.println(settings.serverIp);
  Serial.print("SERVER_PORT: ");
  Serial.println(settings.serverPort);
  Serial.println("Note: PMS5003 has PM1.0 / PM2.5 / PM10, not PM5.");
  Serial.println("OLED pins: SDA=22 SCL=23");
  Serial.println("PMS5003 pins: TXD=16 RXD=17");
  Serial.println("INMP441 pins: SCK=27 WS=14 SD=26 L/R reserved at GPIO12");
  Serial.println("CONFIG button: GPIO33 -> GND (hold " + configHoldSecondsLabel() + "s to toggle config mode)");

  if (!initOLED()) {
    Serial.println("OLED init failed");
  }

  if (!initMicrophone()) {
    Serial.println("INMP441 init failed");
  }

  if (digitalRead(CONFIG_BUTTON_PIN) == LOW) {
    delay(CONFIG_HOLD_MS);
    if (digitalRead(CONFIG_BUTTON_PIN) == LOW) {
      beginConfigPortal();
      return;
    }
  }

  connectToSavedWifi();
  resetReportWindow();
  lastReportMs = millis();
}

void loop() {
  updateConfigButton();

  if (configMode) {
    if (configPortalReady) {
      dnsServer.processNextRequest();
      configServer.handleClient();
    }
    if (rebootAfterSaveMs != 0 && millis() >= rebootAfterSaveMs) {
      ESP.restart();
    }
    return;
  }

  if (sensorRecoveryUntilMs != 0 && millis() < sensorRecoveryUntilMs) {
    clearPmsInputBuffer();
    updateMicrophoneState();
    if (millis() - lastOledRefreshMs >= OLED_REFRESH_MS) {
      lastOledRefreshMs = millis();
      renderOLED();
    }
    if (millis() - lastSerialStatusMs >= SERIAL_STATUS_MS) {
      lastSerialStatusMs = millis();
      printStatus();
    }
    return;
  }
  sensorRecoveryUntilMs = 0;

  while (readPMSdata(&pmsSerial)) {
  }

  updateMicrophoneState();
  updateReportWindow();

  if (millis() - lastOledRefreshMs >= OLED_REFRESH_MS) {
    lastOledRefreshMs = millis();
    renderOLED();
  }

  if (millis() - lastReportMs >= REPORT_INTERVAL_MS) {
    lastReportMs = millis();
    postSensorData();
  }

  if (millis() - lastSerialStatusMs >= SERIAL_STATUS_MS) {
    lastSerialStatusMs = millis();
    printStatus();
  }
}
