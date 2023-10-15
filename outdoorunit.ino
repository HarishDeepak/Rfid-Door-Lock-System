#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RST_PIN 9
#define SS_PIN 10

byte key = 77;
byte readCard[4];
String tagID = "";

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.begin(9600);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print(" Access Control ");
  lcd.setCursor(0, 1);
  lcd.print("Scan Your Card>>");
  pinMode(2, INPUT);
}

void access_granted() {
  Serial.println("Granted");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Door opened");
  key += 5;
  return;
}

void encrypt() {
  for (int i = 0; i < 4; i++) {
    Serial.print(readCard[i], HEX);
    readCard[i] = readCard[i] ^ key;
  }
  Serial.println();
  for (int i = 0; i < 4; i++)
    Serial.print(readCard[i], HEX);
  Serial.println();
}

void door_reset() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Access Control ");
  lcd.setCursor(0, 1);
  lcd.print("Scan Your Card>>");
  return;
}

int b = 1;

void loop() {
  while (getID()) {
    Serial.println(tagID);
    sendTag();
  }
  int a = digitalRead(2);
  if (a == 0 && a != b)
    access_granted();
  else if (a == 1 && a != b)
    door_reset();
  b = a;
}

void sendTag() {
  encrypt();
  Wire.beginTransmission(8);
  for (int i = 0; i < 4; i++)
    Wire.write((char)readCard[i]);
  Wire.write('\0');
  Wire.endTransmission();
}

boolean getID() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return false;
  }
  tagID = "";
  for (uint8_t i = 0; i < 4; i++) {
    readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA();
  return true;
}
