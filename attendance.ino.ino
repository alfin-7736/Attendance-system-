#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// --- WI-FI CREDENTIALS ---
const char* ssid = "Lava";        
const char* password = "123456789"; 

// --- YOUR GOOGLE SCRIPT URL ---
String scriptURL = "https://script.google.com/macros/s/AKfycbxqgg42BzQz8bRBGgMOEfWLMtxN_UX24BNNnR2NgFNZAjzNxpmvE0MJuIJ75FxZREpK/exec";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

SoftwareSerial mySerial(D5, D6);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(115200);
  
  Wire.begin(D2, D1); 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // 1. CONNECT TO SENSOR FIRST (Before Wi-Fi Spike)
  display.setCursor(0, 10);
  display.println("Starting Sensor...");
  display.display();

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
    display.println("Sensor Error!");
    display.setCursor(0, 25);
    display.println("Check Wires.");
    display.display();
    while (1) { delay(1); }
  }

  // 2. NOW CONNECT TO WI-FI
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("Sensor Ready!");
  display.setCursor(0, 25);
  display.println("Connecting Wi-Fi...");
  display.display();
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print(".");
  }

  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("Wi-Fi Connected!");
  display.setCursor(0, 30);
  display.println("System Ready!");
  display.display();
  delay(2000);
}

void loop() {
  // Main standby screen
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.println("SCAN FINGER");
  display.display();

  int fingerID = getFingerprintID();
  
  // If a fingerprint is recognized!
  if (fingerID > 0) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print("ID: ");
    display.println(fingerID);
    
    display.setTextSize(1);
    display.setCursor(10, 40);
    display.println("Sending to Database.");
    display.display();

    // Send to Google Sheets
    sendToGoogleSheets(fingerID);
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(20, 20);
    display.println("LOGGED!");
    display.display();
    
    delay(3000); // Wait 3 seconds before allowing the next scan
  }
}

// Read the scanner
int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) { 
    return finger.fingerID; 
  }
  return -1;
}

// Send data over Wi-Fi
void sendToGoogleSheets(int id) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); // Required for Google Apps Script HTTPS
    HTTPClient http;

    String finalURL = scriptURL + "?action=log&id=" + String(id);
    
    http.begin(client, finalURL);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      Serial.println("Data successfully logged to Google Sheets!");
    } else {
      Serial.println("Failed to connect to Google Sheets.");
    }
    http.end();
  }
}