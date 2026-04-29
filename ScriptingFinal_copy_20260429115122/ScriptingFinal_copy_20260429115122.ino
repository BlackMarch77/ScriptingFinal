#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define DHTPIN 4
#define DHTTYPE DHT11

// LED Pins
const int whiteLED = 15; const int blueLED = 5; const int greenLED = 18;
const int yellowLED = 19; const int redLED = 23;

// --- ICONS ---
const unsigned char sun_icon [] PROGMEM = { 0x01, 0x80, 0x01, 0x80, 0x21, 0x84, 0x11, 0x88, 0x09, 0x90, 0x07, 0xe0, 0x07, 0xe0, 0x7f, 0xfe, 0x07, 0xe0, 0x07, 0xe0, 0x09, 0x90, 0x11, 0x88, 0x21, 0x84, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00 };
const unsigned char snow_icon [] PROGMEM = { 0x01, 0x80, 0x11, 0x88, 0x09, 0x90, 0x05, 0xa0, 0x03, 0xc0, 0x7f, 0xfe, 0x03, 0xc0, 0x05, 0xa0, 0x09, 0x90, 0x11, 0x88, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
const unsigned char droplet_icon [] PROGMEM = { 0x18, 0x3c, 0x3c, 0x7e, 0x7e, 0xff, 0xff, 0x7e };

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastSensorUpdate = 0;
float f = 0, h = 0, hi = 0;
float maxTemp = -999.0;
float minTemp = 999.0;

void setup() {
  dht.begin();
  pinMode(whiteLED, OUTPUT); pinMode(blueLED, OUTPUT);
  pinMode(greenLED, OUTPUT); pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) for(;;);
  display.clearDisplay();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastSensorUpdate >= 2000) {
    lastSensorUpdate = currentMillis;
    h = dht.readHumidity();
    f = dht.readTemperature(true);
    hi = dht.computeHeatIndex(f, h);

    if (!isnan(f)) {
      if (f > maxTemp) maxTemp = f;
      if (f < minTemp) minTemp = f;
    }
    updateDisplay();
  }

  if (isnan(h) || isnan(f)) return;

  float pulse = (exp(sin(currentMillis / 3000.0 * 2 * PI)) - 0.36787944) * 108.0;
  int glow = (int)pulse;

  analogWrite(whiteLED, 0); analogWrite(blueLED, 0); analogWrite(greenLED, 0);
  analogWrite(yellowLED, 0); analogWrite(redLED, 0);

  if (hi >= 103.0) {
    analogWrite(whiteLED, glow); analogWrite(blueLED, glow); analogWrite(greenLED, glow); analogWrite(yellowLED, glow); analogWrite(redLED, glow);
  } else if (f >= 90.0) {
    analogWrite(whiteLED, glow); analogWrite(blueLED, glow); analogWrite(greenLED, glow); analogWrite(yellowLED, glow);
  } else if (f >= 70.0) {
    analogWrite(whiteLED, glow); analogWrite(blueLED, glow); analogWrite(greenLED, glow);
  } else if (f >= 50.0) {
    analogWrite(whiteLED, glow); analogWrite(blueLED, glow);
  } else if (f > 32.0) {
    analogWrite(whiteLED, glow);
  } else {
    digitalWrite(whiteLED, (currentMillis % 500 < 250) ? HIGH : LOW);
  }
  delay(16);
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("WEATHER STATION");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  // --- ICONS ---
  if (f > 65.0) {
    display.drawBitmap(100, 18, sun_icon, 16, 16, SSD1306_WHITE);
  } else {
    display.drawBitmap(100, 18, snow_icon, 16, 16, SSD1306_WHITE);
  }

  int dropletCount = (h >= 61.0) ? 3 : (h >= 31.0) ? 2 : 1;
  for (int i = 0; i < dropletCount; i++) {
    display.drawBitmap(100 + (i * 9), 38, droplet_icon, 8, 8, SSD1306_WHITE);
  }

  // --- TEXT DATA ---
  display.setCursor(0, 18);
  display.print("Temp: "); display.print(f, 1); display.print(" F");
  display.setCursor(0, 38);
  display.print("Hum:  "); display.print(h, 1); display.print(" %");
  display.setCursor(0, 53);
  display.print("Feel: "); display.print(hi, 1); display.print(" F");

  // --- MAX/MIN (Bottom Right) ---
  if (maxTemp != -999.0) {
    display.setCursor(85, 48); 
    display.print("H:"); display.print((int)maxTemp);
    display.setCursor(85, 56);
    display.print("L:"); display.print((int)minTemp);
  }

  if (hi >= 103.0) {
    display.fillRect(80, 46, 48, 18, SSD1306_BLACK); // Clear Max/Min area
    display.setCursor(82, 53);
    display.print("DANGER!");
  }

  display.display();
}
