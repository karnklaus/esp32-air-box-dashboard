#include <HardwareSerial.h>

HardwareSerial pmsSerial(2);

constexpr int PMS_RX_PIN = 16;
constexpr int PMS_TX_PIN = 17;
constexpr uint32_t USB_BAUD = 115200;
constexpr uint32_t PMS_BAUD = 9600;

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um;
  uint16_t particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
};

pms5003data data;

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

  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
  }

  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }

  memcpy((void *)&data, (void *)buffer_u16, 30);

  if (sum != data.checksum) {
    Serial.println("Checksum failure");
    return false;
  }

  return true;
}

void setup() {
  Serial.begin(USB_BAUD);
  pmsSerial.begin(PMS_BAUD, SERIAL_8N1, PMS_RX_PIN, PMS_TX_PIN);

  Serial.println();
  Serial.println("ESP32 + PMS5003");
  Serial.println("Wiring:");
  Serial.println("PMS5003 VCC   -> 5V");
  Serial.println("PMS5003 GND   -> GND");
  Serial.println("PMS5003 TXD   -> GPIO23");
  Serial.println("PMS5003 RXD   -> GPIO22");
  Serial.println("PMS5003 SET   -> HIGH");
  Serial.println("PMS5003 RESET -> HIGH");
  Serial.println("Waiting for PMS5003 data...");
}

void loop() {
  if (readPMSdata(&pmsSerial)) {
    Serial.println();
    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (standard)");
    Serial.print("PM 1.0: ");
    Serial.print(data.pm10_standard);
    Serial.print("\t\tPM 2.5: ");
    Serial.print(data.pm25_standard);
    Serial.print("\t\tPM 10: ");
    Serial.println(data.pm100_standard);

    Serial.println("---------------------------------------");
    Serial.println("Concentration Units (environmental)");
    Serial.print("PM 1.0: ");
    Serial.print(data.pm10_env);
    Serial.print("\t\tPM 2.5: ");
    Serial.print(data.pm25_env);
    Serial.print("\t\tPM 10: ");
    Serial.println(data.pm100_env);

    Serial.println("---------------------------------------");
    Serial.print("Particles > 0.3um / 0.1L air:");
    Serial.println(data.particles_03um);
    Serial.print("Particles > 0.5um / 0.1L air:");
    Serial.println(data.particles_05um);
    Serial.print("Particles > 1.0um / 0.1L air:");
    Serial.println(data.particles_10um);
    Serial.print("Particles > 2.5um / 0.1L air:");
    Serial.println(data.particles_25um);
    Serial.print("Particles > 5.0um / 0.1L air:");
    Serial.println(data.particles_50um);
    Serial.print("Particles > 10.0 um / 0.1L air:");
    Serial.println(data.particles_100um);
    Serial.println("---------------------------------------");
  }
}
