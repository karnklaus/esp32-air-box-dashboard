#include <Wire.h>

constexpr uint32_t USB_BAUD = 115200;
constexpr uint8_t I2C_SDA_PIN = 22;
constexpr uint8_t I2C_SCL_PIN = 23;
constexpr uint8_t AM2320_ADDR = 0x5C;
constexpr uint8_t OLED_ADDR = 0x3C;
constexpr uint8_t OLED_WIDTH = 128;
constexpr uint8_t OLED_HEIGHT = 64;
constexpr uint8_t OLED_PAGES = OLED_HEIGHT / 8;
constexpr unsigned long SENSOR_READ_MS = 2000;
constexpr unsigned long I2C_SCAN_MS = 30000;
constexpr bool SERIAL_DEBUG = true;
constexpr uint8_t AM2320_MAX_RETRIES = 3;
constexpr uint32_t I2C_CLOCK_HZ = 50000;
constexpr uint8_t AM2320_WAKE_DELAY_MS = 10;
constexpr uint8_t AM2320_MEASURE_DELAY_MS = 10;
constexpr bool ENABLE_PERIODIC_I2C_SCAN = false;

uint8_t screenBuffer[OLED_WIDTH * OLED_PAGES];
unsigned long lastSensorReadMs = 0;
unsigned long lastScanMs = 0;
float lastTemperatureC = 0.0f;
float lastHumidityRh = 0.0f;
bool sensorOk = false;
bool displayOk = false;

void printHexByte(uint8_t value) {
  if (value < 0x10) {
    Serial.print('0');
  }
  Serial.print(value, HEX);
}

void printHexBuffer(const uint8_t *data, size_t length) {
  for (size_t i = 0; i < length; ++i) {
    if (i > 0) {
      Serial.print(' ');
    }
    printHexByte(data[i]);
  }
}

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
  {0b01110, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001, 0b10001},
  {0b11110, 0b10001, 0b11110, 0b10001, 0b10001, 0b10001, 0b11110},
  {0b01111, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b01111},
  {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110},
  {0b11111, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000, 0b11111},
  {0b11111, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000, 0b10000},
  {0b01111, 0b10000, 0b10000, 0b10011, 0b10001, 0b10001, 0b01110},
  {0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001, 0b10001},
  {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b11111},
  {0b00001, 0b00001, 0b00001, 0b00001, 0b10001, 0b10001, 0b01110},
  {0b10001, 0b10010, 0b11100, 0b10010, 0b10001, 0b10001, 0b10001},
  {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111},
  {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001},
  {0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b10001},
  {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},
  {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000},
  {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101},
  {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001},
  {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110},
  {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100},
  {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},
  {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100},
  {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010},
  {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001},
  {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100},
  {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111}
};

uint16_t crc16Modbus(const uint8_t *data, size_t length) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < length; ++i) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; ++bit) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
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

bool initDisplay() {
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
  if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) {
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

void drawText(int x, int y, const char *text, uint8_t scale) {
  int cursorX = x;
  while (*text) {
    const char current = *text;
    drawChar5x7(cursorX, y, current, scale);
    cursorX += (current == '.') ? (2 * scale) : (6 * scale);
    ++text;
  }
}

bool flushDisplay() {
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

bool isDeviceReady(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

void debugProbeAddress(uint8_t address, const char *label) {
  Wire.beginTransmission(address);
  const uint8_t error = Wire.endTransmission();
  Serial.print("[I2C PROBE] ");
  Serial.print(label);
  Serial.print(" addr=0x");
  printHexByte(address);
  Serial.print(" endTransmission=");
  Serial.print(error);
  Serial.print(" status=");
  Serial.println(error == 0 ? "ACK" : "NO ACK");
}

void scanI2CBus() {
  Serial.println("I2C scan start");
  uint8_t foundCount = 0;
  for (uint8_t address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    const uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("Found I2C device at 0x");
      if (address < 16) {
        Serial.print('0');
      }
      Serial.println(address, HEX);
      ++foundCount;
    } else if (SERIAL_DEBUG && error != 2) {
      Serial.print("Probe 0x");
      printHexByte(address);
      Serial.print(" -> error ");
      Serial.println(error);
    }
    delay(1);
  }
  if (foundCount == 0) {
    Serial.println("No I2C device found");
  }
  Serial.println("I2C scan end");
}

bool readAM2320(float &temperatureC, float &humidityRh) {
  const uint8_t requestFrame[] = {0x03, 0x00, 0x04};
  const uint8_t expectedBytes = 8;

  if (SERIAL_DEBUG) {
    Serial.println("[AM2320] read cycle start");
  }

  for (uint8_t attempt = 1; attempt <= AM2320_MAX_RETRIES; ++attempt) {
    uint8_t responseFrame[expectedBytes] = {};

    if (SERIAL_DEBUG) {
      Serial.print("[AM2320] attempt ");
      Serial.print(attempt);
      Serial.print("/");
      Serial.println(AM2320_MAX_RETRIES);
    }

    Wire.beginTransmission(AM2320_ADDR);
    const uint8_t wakeResult = Wire.endTransmission();
    if (SERIAL_DEBUG) {
      Serial.print("[AM2320] wake addr=0x");
      printHexByte(AM2320_ADDR);
      Serial.print(" result=");
      Serial.println(wakeResult);
    }
    delay(AM2320_WAKE_DELAY_MS);

    Wire.beginTransmission(AM2320_ADDR);
    Wire.write(requestFrame, sizeof(requestFrame));
    const uint8_t txResult = Wire.endTransmission();
    if (SERIAL_DEBUG) {
      Serial.print("[AM2320] tx raw=");
      printHexBuffer(requestFrame, sizeof(requestFrame));
      Serial.print(" endTransmission=");
      Serial.println(txResult);
    }
    if (txResult != 0) {
      delay(5);
      continue;
    }

    delay(AM2320_MEASURE_DELAY_MS);

    const uint8_t received = Wire.requestFrom(static_cast<int>(AM2320_ADDR), static_cast<int>(expectedBytes));
    if (SERIAL_DEBUG) {
      Serial.print("[AM2320] requestFrom addr=0x");
      printHexByte(AM2320_ADDR);
      Serial.print(" expected=");
      Serial.print(expectedBytes);
      Serial.print(" received=");
      Serial.println(received);
    }
    if (received != expectedBytes) {
      while (Wire.available()) {
        Wire.read();
      }
      delay(10);
      continue;
    }

    uint8_t buffer[expectedBytes];
    for (uint8_t i = 0; i < expectedBytes; ++i) {
      buffer[i] = Wire.read();
      responseFrame[i] = buffer[i];
    }

    if (SERIAL_DEBUG) {
      Serial.print("[AM2320] rx raw=");
      printHexBuffer(responseFrame, expectedBytes);
      Serial.println();
    }

    if (buffer[0] != 0x03 || buffer[1] != 0x04) {
      if (SERIAL_DEBUG) {
        Serial.print("[AM2320] invalid function/length header: fn=0x");
        printHexByte(buffer[0]);
        Serial.print(" len=0x");
        printHexByte(buffer[1]);
        Serial.println();
      }
      delay(10);
      continue;
    }

    const uint16_t crcExpected = static_cast<uint16_t>(buffer[7] << 8) | buffer[6];
    const uint16_t crcActual = crc16Modbus(buffer, 6);
    if (SERIAL_DEBUG) {
      Serial.print("[AM2320] crc raw=0x");
      Serial.print(crcExpected, HEX);
      Serial.print(" calc=0x");
      Serial.print(crcActual, HEX);
      Serial.print(" match=");
      Serial.println(crcActual == crcExpected ? "YES" : "NO");
    }
    if (crcActual != crcExpected) {
      delay(10);
      continue;
    }

    const uint16_t humidityRaw = static_cast<uint16_t>(buffer[2] << 8) | buffer[3];
    uint16_t temperatureRaw = static_cast<uint16_t>(buffer[4] << 8) | buffer[5];
    const bool negative = (temperatureRaw & 0x8000) != 0;
    if (negative) {
      temperatureRaw &= 0x7FFF;
    }

    humidityRh = humidityRaw / 10.0f;
    temperatureC = temperatureRaw / 10.0f;
    if (negative) {
      temperatureC = -temperatureC;
    }

    if (SERIAL_DEBUG) {
      Serial.print("[AM2320] humidityRaw=0x");
      Serial.print(humidityRaw, HEX);
      Serial.print(" -> ");
      Serial.print(humidityRh, 1);
      Serial.println(" %RH");
      Serial.print("[AM2320] temperatureRaw=0x");
      Serial.print(temperatureRaw | (negative ? 0x8000 : 0x0000), HEX);
      Serial.print(" sign=");
      Serial.print(negative ? "-" : "+");
      Serial.print(" -> ");
      Serial.print(temperatureC, 1);
      Serial.println(" C");
      Serial.println("[AM2320] read cycle end");
    }
    return true;
  }

  if (SERIAL_DEBUG) {
    Serial.println("[AM2320] read cycle end -> FAILED after retries");
  }
  return false;
}

void drawSensorScreen() {
  clearBuffer();
  drawRect(0, 0, 128, 64, true);
  drawText(6, 4, "AM2320 I2C TEST", 1);
  drawText(6, 18, "SDA:22  SCL:23", 1);

  drawText(6, 30, "OLED", 1);
  drawText(40, 30, displayOk ? "OK" : "FAIL", 1);
  drawText(72, 30, "AM2320", 1);
  drawText(112, 30, sensorOk ? "OK" : "X", 1);

  if (sensorOk) {
    char tempText[20];
    char humidText[20];
    snprintf(tempText, sizeof(tempText), "%.1f C", lastTemperatureC);
    snprintf(humidText, sizeof(humidText), "%.1f RH", lastHumidityRh);
    drawText(6, 42, "TEMP", 1);
    drawText(42, 42, tempText, 1);
    drawText(6, 54, "HUM", 1);
    drawText(42, 54, humidText, 1);
  } else {
    drawText(6, 44, "TEMP --.- C", 1);
    drawText(6, 54, "CHECK ADDR 0X5C", 1);
  }

  flushDisplay();
}

void setup() {
  Serial.begin(USB_BAUD);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(I2C_CLOCK_HZ);
  delay(200);

  Serial.println();
  Serial.println("ESP32 I2C test for AM2320 + NFP1315-157B");
  Serial.println("Pins: SDA=22, SCL=23");
  Serial.print("I2C clock: ");
  Serial.print(I2C_CLOCK_HZ);
  Serial.println(" Hz");

  scanI2CBus();
  debugProbeAddress(OLED_ADDR, "OLED");
  debugProbeAddress(AM2320_ADDR, "AM2320");

  displayOk = isDeviceReady(OLED_ADDR) && initDisplay();
  sensorOk = isDeviceReady(AM2320_ADDR);

  Serial.print("OLED 0x3C: ");
  Serial.println(displayOk ? "OK" : "NOT FOUND");
  Serial.print("AM2320 0x5C: ");
  Serial.println(sensorOk ? "OK" : "NOT FOUND");

  if (sensorOk) {
    sensorOk = readAM2320(lastTemperatureC, lastHumidityRh);
    Serial.print("First read AM2320: ");
    Serial.println(sensorOk ? "SUCCESS" : "FAILED");
  }

  if (displayOk) {
    drawSensorScreen();
  }
}

void loop() {
  if (ENABLE_PERIODIC_I2C_SCAN && millis() - lastScanMs >= I2C_SCAN_MS) {
    lastScanMs = millis();
    scanI2CBus();
  }

  if (millis() - lastSensorReadMs < SENSOR_READ_MS) {
    delay(50);
    return;
  }

  lastSensorReadMs = millis();
  sensorOk = readAM2320(lastTemperatureC, lastHumidityRh);

  if (sensorOk) {
    Serial.print("AM2320 -> Temp: ");
    Serial.print(lastTemperatureC, 1);
    Serial.print(" C, Humidity: ");
    Serial.print(lastHumidityRh, 1);
    Serial.println(" %RH");
  } else {
    Serial.println("AM2320 read failed");
  }

  if (displayOk) {
    drawSensorScreen();
  }
}
