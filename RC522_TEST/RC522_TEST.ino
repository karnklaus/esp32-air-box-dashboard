#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);

byte lastUid[10] = {0};
byte lastUidSize = 0;
bool hasLastUid = false;

void printDivider(const char *title) {
  Serial.println();
  Serial.println("============================================================");
  Serial.println(title);
  Serial.println("============================================================");
}

void printBits8(byte value) {
  for (int8_t bit = 7; bit >= 0; --bit) {
    Serial.print((value >> bit) & 0x01);
  }
}

void printByteHex(byte value) {
  if (value < 0x10) {
    Serial.print('0');
  }
  Serial.print(value, HEX);
}

void printUidHex(const MFRC522::Uid &uid) {
  for (byte i = 0; i < uid.size; i++) {
    if (i > 0) {
      Serial.print(' ');
    }
    printByteHex(uid.uidByte[i]);
  }
}

void printUidBinary(const MFRC522::Uid &uid) {
  for (byte i = 0; i < uid.size; i++) {
    printBits8(uid.uidByte[i]);
    if (i + 1 < uid.size) {
      Serial.print(' ');
    }
  }
}

uint64_t uidToUint64(const MFRC522::Uid &uid) {
  uint64_t value = 0;
  for (byte i = 0; i < uid.size; i++) {
    value = (value << 8) | uid.uidByte[i];
  }
  return value;
}

void printUidDecimal(const MFRC522::Uid &uid) {
  uint64_t value = uidToUint64(uid);
  Serial.printf("%llu", static_cast<unsigned long long>(value));
}

bool isSameAsLastUid(const MFRC522::Uid &uid) {
  if (!hasLastUid || uid.size != lastUidSize) {
    return false;
  }

  for (byte i = 0; i < uid.size; i++) {
    if (uid.uidByte[i] != lastUid[i]) {
      return false;
    }
  }
  return true;
}

void rememberUid(const MFRC522::Uid &uid) {
  lastUidSize = uid.size;
  for (byte i = 0; i < uid.size; i++) {
    lastUid[i] = uid.uidByte[i];
  }
  hasLastUid = true;
}

void printSakInfo(byte sak) {
  Serial.print("SAK               : 0x");
  printByteHex(sak);
  Serial.print("  BIN=");
  printBits8(sak);
  Serial.println();
}

bool printAtqa() {
  byte atqa[2] = {0, 0};
  byte atqaSize = sizeof(atqa);
  MFRC522::StatusCode status = rfid.PICC_RequestA(atqa, &atqaSize);

  if (status != MFRC522::STATUS_OK) {
    return false;
  }

  Serial.print("ATQA status       : ");
  Serial.println(rfid.GetStatusCodeName(status));

  Serial.print("ATQA bytes        : ");
  Serial.println(atqaSize);

  Serial.print("ATQA hex          : ");
  for (byte i = 0; i < atqaSize; i++) {
    if (i > 0) {
      Serial.print(' ');
    }
    printByteHex(atqa[i]);
  }
  Serial.println();

  Serial.print("ATQA bin          : ");
  for (byte i = 0; i < atqaSize; i++) {
    printBits8(atqa[i]);
    if (i + 1 < atqaSize) {
      Serial.print(' ');
    }
  }
  Serial.println();

  return true;
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  Serial.println();
  Serial.println("ESP32 + RC522 SPI debug test");
  Serial.println("Wiring:");
  Serial.println("SDA/SS -> GPIO5");
  Serial.println("RST    -> GPIO22");
  Serial.println("SCK    -> GPIO18");
  Serial.println("MOSI   -> GPIO23");
  Serial.println("MISO   -> GPIO19");
  Serial.println("Power  -> 3.3V");
  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
}

void loop() {
  if (!printAtqa()) {
    delay(50);
    return;
  }

  printDivider("Card detected");
  Serial.print("Timestamp (ms)    : ");
  Serial.println(millis());

  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println("UID read failed");
    delay(300);
    return;
  }

  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  bool sameCard = isSameAsLastUid(rfid.uid);

  Serial.print("Card state        : ");
  Serial.println(sameCard ? "Same as previous card" : "New card");

  Serial.print("RFID/NFC Tag Type : ");
  Serial.println(rfid.PICC_GetTypeName(piccType));

  Serial.print("UID size          : ");
  Serial.print(rfid.uid.size);
  Serial.println(" byte(s)");

  Serial.print("UID hex           : ");
  printUidHex(rfid.uid);
  Serial.println();

  Serial.print("UID dec           : ");
  printUidDecimal(rfid.uid);
  Serial.println();

  Serial.print("UID bin           : ");
  printUidBinary(rfid.uid);
  Serial.println();

  printSakInfo(rfid.uid.sak);

  rememberUid(rfid.uid);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(1000);
}
