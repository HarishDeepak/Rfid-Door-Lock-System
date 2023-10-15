#include <Wire.h>
#include <Servo.h>

#define button A1
#define latch A0
#define buzzer 7

String MasterTag = "8D8C4CD3";
String MasterTag1 = "9CAE54A1";
String tagID = "";
byte cardUID[4];
byte key = 77;

Servo doorlock;
char c;
int i = 0;
bool door = false;

void setup() {
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
  doorlock.attach(9);
  doorlock.write(0);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
}

void loop() {
  if (door)
    door_open();
  int a = analogRead(button);
  if (a > 900)
    door_open();
  delay(100);
}

void decrypt() {
  Serial.println("Decrypt");
  for (int i = 0; i < 4; i++) {
    Serial.print(cardUID[i], HEX);
    cardUID[i] = cardUID[i] ^ key;
  }
  Serial.println();
  for (int i = 0; i < 4; i++) {
    tagID.concat(String(cardUID[i], HEX));
  }
  tagID.toUpperCase();
  Serial.println(tagID);
  Serial.println("Decrypt - over");
  checkTag();
}

void receiveEvent(int a) {
  while (Wire.available()) {
    c = Wire.read();
    if (c != '\0') {
      Serial.print(c);
      cardUID[i] = (int)c;
      i++;
    } else if (c == '\0')
      decrypt();
    Serial.println();
  }
}

void door_open() {
  Serial.println(tagID.equals(MasterTag));
  digitalWrite(2, LOW);
  doorlock.write(180);
  delay(2500);
  int a = analogRead(latch);
  long int start_t = millis();
  while (a > 900) {
    a = analogRead(latch);
    if ((millis() - start_t) > 3000)
      digitalWrite(buzzer, HIGH);
  }
  digitalWrite(buzzer, LOW);
  doorlock.write(0);
  digitalWrite(2, HIGH);
  key += 5;
  door = false;
}

void checkTag() {
  if (tagID.equals(MasterTag) || tagID.equals(MasterTag1)) {
    door = true;
  } else {
    delay(4000);
  }
  tagID = "";
  i = 0;
}
