#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

SoftwareSerial mySerial(D5, D6);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1); 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 10);
  display.println("Connecting Sensor...");
  display.display();

  // Auto-Scanner to find the R307S speed
  int baudRates[] = {9600, 19200, 38400, 57600, 115200};
  bool found = false;
  for (int i = 0; i < 5; i++) {
    finger.begin(baudRates[i]);
    if (finger.verifyPassword()) {
      found = true;
      break; 
    }
  }

  if (!found) {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Sensor Not Found!");
    display.display();
    while (1) { delay(1); }
  }

  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("Sensor Ready!");
  display.setCursor(0, 25);
  display.println("Open Serial Monitor");
  display.display();
}

uint8_t readnumber(void) {
  uint8_t num = 0;
  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop() {
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type the ID # (from 1 to 127) you want to save this finger as...");
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println("Enter ID in");
  display.setCursor(0, 25);
  display.println("Serial Monitor...");
  display.display();

  id = readnumber();
  if (id == 0) { return; }
  
  Serial.print("Enrolling ID #");
  Serial.println(id);
  
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("Enrolling ID: ");
  display.println(id);
  display.display();
  
  getFingerprintEnroll();
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  display.setCursor(0, 30);
  display.println("Place Finger Now");
  display.display();
  Serial.print("Waiting for valid finger to enroll as ID #"); Serial.println(id);
  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      display.clearDisplay();
      display.setCursor(0, 10);
      display.println("Image taken!");
      display.display();
      break;
    }
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) return p;
  
  display.setCursor(0, 30);
  display.println("Remove Finger");
  display.display();
  Serial.println("Remove finger");
  delay(2000);
  
  p = 0;
  while (p != FINGERPRINT_NOFINGER) { p = finger.getImage(); }
  
  Serial.print("Place same finger again");
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("Place Same");
  display.setCursor(0, 25);
  display.println("Finger Again!");
  display.display();
  
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_OK) { Serial.println("Image taken"); }
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) return p;
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else {
    Serial.println("Fingerprints did not match");
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Did Not Match!");
    display.display();
    delay(2000);
    return p;
  }
  
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 20);
    display.println("SAVED!");
    display.display();
    delay(3000);
  } else {
    Serial.println("Error saving");
  }
  return p;
}