#include <Wire.h>

// ESP32 + OLED mock interface for PM2.5 / PM5 / PM10
// Wiring:
// OLED VCC -> 3V3
// OLED GND -> GND
// OLED SDA -> GPIO21
// OLED SCL -> GPIO22

constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint8_t OLED_ADDR = 0x3C;
constexpr uint8_t OLED_WIDTH = 128;
constexpr uint8_t OLED_PAGES = 8;
constexpr uint32_t USB_BAUD = 115200;
constexpr unsigned long SCREEN_STEP_MS = 2500;

uint8_t screenBuffer[OLED_WIDTH * OLED_PAGES];
unsigned long lastScreenStepMs = 0;
uint8_t demoIndex = 0;

struct DemoValues {
  uint16_t pm25;
  uint16_t pm5;
  uint16_t pm10;
  const char *label;
};

const DemoValues demoData[] = {
  {12, 18, 24, "CLEAN"},
  {35, 49, 62, "MODERATE"},
  {78, 96, 120, "HIGH"},
  {145, 180, 220, "DANGER"}
};

const uint8_t digits3x5[][5] = {
  {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
  {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
  {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
  {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
  {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
  {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
  {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
  {0b111, 0b001, 0b010, 0b010, 0b010}, // 7
  {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
  {0b111, 0b101, 0b111, 0b001, 0b111}  // 9
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

bool sendCommand(uint8_t command) {
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x00);
  Wire.write(command);
  return Wire.endTransmission() == 0;
}

bool sendData(const uint8_t *data, size_t length) {
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x40);
  for (size_t i = 0; i < length; ++i) {
    Wire.write(data[i]);
  }
  return Wire.endTransmission() == 0;
}

bool initDisplay() {
  const uint8_t commands[] = {
    0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
    0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,
    0x81, 0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6,
    0x2E, 0xAF
  };

  for (size_t i = 0; i < sizeof(commands); ++i) {
    if (!sendCommand(commands[i])) {
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

  size_t index = x + (y / 8) * OLED_WIDTH;
  uint8_t mask = 1 << (y % 8);
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

  if (c >= '0' && c <= '9') {
    drawDigit3x5(x + scale, y + scale, c, scale);
    return;
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

void drawText5x7(int x, int y, const char *text, uint8_t scale) {
  int cursorX = x;
  while (*text) {
    char current = *text;
    drawChar5x7(cursorX, y, current, scale);
    cursorX += (current == '.') ? (2 * scale) : (6 * scale);
    ++text;
  }
}

int textWidth5x7(const char *text, uint8_t scale) {
  int width = 0;
  while (*text) {
    width += (*text == '.') ? (2 * scale) : (6 * scale);
    ++text;
  }
  return width;
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

bool flushBuffer() {
  for (uint8_t page = 0; page < OLED_PAGES; ++page) {
    if (!sendCommand(0xB0 | page) || !sendCommand(0x00) || !sendCommand(0x10)) {
      return false;
    }

    for (uint8_t block = 0; block < 8; ++block) {
      if (!sendData(&screenBuffer[page * OLED_WIDTH + block * 16], 16)) {
        return false;
      }
    }
  }
  return true;
}

void renderHeader(const char *title) {
  drawRect(0, 0, 128, 64, true);
  drawText5x7(6, 4, title, 1);
  for (int x = 5; x < 123; x += 4) {
    setPixel(x, 14, true);
  }
}

void renderStatusPill(const char *label) {
  int pillWidth = textWidth5x7(label, 1) + 8;
  int pillX = 128 - pillWidth - 6;
  drawRect(pillX, 2, pillWidth, 11, true);
  drawText5x7(pillX + 4, 4, label, 1);
}

void renderValueCard(int x, int y, int w, int h, const char *title, uint16_t value) {
  drawRect(x, y, w, h, true);
  drawText5x7(x + 4, y + 4, title, 1);

  char buf[6];
  snprintf(buf, sizeof(buf), "%u", value);
  int numberWidth = static_cast<int>(strlen(buf)) * 8 - 2;
  int numberX = x + (w - numberWidth) / 2;
  drawNumber(numberX, y + 18, value, 2);
}

void renderPMInterface(const DemoValues &values) {
  clearBuffer();
  renderHeader("AIR PANEL");
  renderStatusPill(values.label);

  renderValueCard(4, 20, 38, 36, "PM2.5", values.pm25);
  renderValueCard(45, 20, 38, 36, "PM5", values.pm5);
  renderValueCard(86, 20, 38, 36, "PM10", values.pm10);
}

void setup() {
  Serial.begin(USB_BAUD);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);

  Serial.println();
  Serial.println("ESP32 + OLED PM interface demo");
  Serial.println("Showing mock PM2.5 / PM5 / PM10 screens");

  Wire.beginTransmission(OLED_ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.println("OLED not found at 0x3C");
    return;
  }

  if (!initDisplay()) {
    Serial.println("OLED init failed");
    return;
  }

  renderPMInterface(demoData[demoIndex]);
  if (!flushBuffer()) {
    Serial.println("OLED flush failed");
    return;
  }

  Serial.println("OLED init success");
  Serial.print("Demo state: ");
  Serial.println(demoData[demoIndex].label);
}

void loop() {
  if (millis() - lastScreenStepMs < SCREEN_STEP_MS) {
    delay(50);
    return;
  }

  lastScreenStepMs = millis();
  demoIndex = (demoIndex + 1) % (sizeof(demoData) / sizeof(demoData[0]));

  renderPMInterface(demoData[demoIndex]);
  if (!flushBuffer()) {
    Serial.println("OLED flush failed");
    delay(250);
    return;
  }

  Serial.print("Demo state: ");
  Serial.print(demoData[demoIndex].label);
  Serial.print(" | PM2.5=");
  Serial.print(demoData[demoIndex].pm25);
  Serial.print(" PM5=");
  Serial.print(demoData[demoIndex].pm5);
  Serial.print(" PM10=");
  Serial.println(demoData[demoIndex].pm10);
}
