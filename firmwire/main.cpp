// Starbie - motion-controlled desktop pet
// Board: Seeed XIAO ESP32-C3
// Parts: MPU6050 (I2C), 128x64 SSD1306 OLED (I2C), 2 push buttons, 2 addressable RGB LEDs
//
// !! Check the pin numbers below against YOUR schematic and change them if needed. !!

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

// ---------- Pins (edit to match your schematic) ----------
#define PIN_SDA   D4      // I2C data  (OLED + MPU6050)
#define PIN_SCL   D5      // I2C clock
#define PIN_BTN1  D1      // Feed button
#define PIN_BTN2  D2      // Play button
#define PIN_LEDS  D3      // Data pin for the 2 RGB LEDs
#define NUM_LEDS  2

// ---------- Settings ----------
#define OLED_ADDR      0x3C
#define MPU_ADDR       0x68
#define SHAKE_THRESH   0.6f      // change in acceleration (g) that counts as a shake
#define SLEEP_AFTER_MS 20000UL   // inactivity before the yawn
#define YAWN_MS        2500UL
#define STAT_TICK_MS   10000UL   // hunger/boredom grow every 10 s

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_NeoPixel leds(NUM_LEDS, PIN_LEDS, NEO_GRB + NEO_KHZ800);

enum Mood { HAPPY, SAD, ANGRY, EXCITED, SLEEPY, SLEEPING };
Mood mood = HAPPY;

bool asleep = false;
bool yawning = false;
unsigned long lastActivity = 0;
unsigned long yawnStart = 0;
unsigned long lastStatTick = 0;
unsigned long excitedUntil = 0;
int hunger = 0;    // 0-100
int boredom = 0;   // 0-100
float lastMag = 1.0f;

// ---------- MPU6050 ----------
void mpuInit() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // PWR_MGMT_1
  Wire.write(0x00);  // wake up
  Wire.endTransmission();
}

float readAccelMagnitude() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  // ACCEL_XOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6);
  if (Wire.available() < 6) return lastMag;
  int16_t ax = (Wire.read() << 8) | Wire.read();
  int16_t ay = (Wire.read() << 8) | Wire.read();
  int16_t az = (Wire.read() << 8) | Wire.read();
  float x = ax / 16384.0f, y = ay / 16384.0f, z = az / 16384.0f;
  return sqrtf(x * x + y * y + z * z);
}

// ---------- LEDs ----------
void setLeds(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUM_LEDS; i++) leds.setPixelColor(i, leds.Color(r, g, b));
  leds.show();
}

void updateLeds() {
  switch (mood) {
    case HAPPY:    setLeds(0, 60, 0);   break;  // green
    case EXCITED:  setLeds(0, 100, 0);  break;  // bright green
    case SAD:      setLeds(60, 40, 0);  break;  // yellow
    case ANGRY:    setLeds(100, 0, 0);  break;  // red
    case SLEEPY:   setLeds(10, 8, 0);   break;  // dim yellow
    case SLEEPING: setLeds(0, 0, 0);    break;  // off
  }
}

// ---------- Faces ----------
void drawEyes(bool open, int pupilDx = 0) {
  if (open) {
    display.fillCircle(38, 24, 10, SSD1306_WHITE);
    display.fillCircle(90, 24, 10, SSD1306_WHITE);
    display.fillCircle(38 + pupilDx, 24, 4, SSD1306_BLACK);
    display.fillCircle(90 + pupilDx, 24, 4, SSD1306_BLACK);
  } else {
    display.drawLine(28, 24, 48, 24, SSD1306_WHITE);
    display.drawLine(80, 24, 100, 24, SSD1306_WHITE);
  }
}

// Draw part of a circle as a thick curve (used for smiles and frowns)
void drawArc(int cx, int cy, int r, int startDeg, int endDeg) {
  for (int a = startDeg; a <= endDeg; a += 4) {
    float rad = a * 0.0174533f;
    display.fillCircle(cx + (int)(r * cosf(rad)), cy + (int)(r * sinf(rad)), 1, SSD1306_WHITE);
  }
}

void drawFace() {
  display.clearDisplay();
  switch (mood) {
    case HAPPY:
      drawEyes(true);
      drawArc(64, 40, 16, 20, 160);           // smile
      break;
    case EXCITED:
      drawEyes(true);
      display.fillCircle(64, 50, 8, SSD1306_WHITE);  // open mouth
      break;
    case SAD:
      drawEyes(true, 0);
      drawArc(64, 62, 16, 200, 340);          // frown
      break;
    case ANGRY:
      drawEyes(true);
      display.drawLine(26, 8, 50, 14, SSD1306_WHITE);   // eyebrows
      display.drawLine(102, 8, 78, 14, SSD1306_WHITE);
      display.drawLine(48, 52, 80, 52, SSD1306_WHITE);  // flat mouth
      break;
    case SLEEPY:
      drawEyes(false);
      display.drawCircle(64, 48, 7, SSD1306_WHITE);     // yawn
      break;
    case SLEEPING:
      drawEyes(false);
      display.drawLine(54, 48, 74, 48, SSD1306_WHITE);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(104, 8);
      display.print("Zzz");
      break;
  }
  display.display();
}

// ---------- Logic ----------
void wakeUp() {
  asleep = false;
  yawning = false;
  lastActivity = millis();
  mood = EXCITED;
  excitedUntil = millis() + 1500;
}

void chooseMood() {
  if (asleep) { mood = SLEEPING; return; }
  if (yawning) { mood = SLEEPY; return; }
  if (millis() < excitedUntil) { mood = EXCITED; return; }
  if (boredom > 70) mood = ANGRY;
  else if (hunger > 70) mood = SAD;
  else mood = HAPPY;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(PIN_SDA, PIN_SCL);
  pinMode(PIN_BTN1, INPUT_PULLUP);
  pinMode(PIN_BTN2, INPUT_PULLUP);

  leds.begin();
  leds.setBrightness(80);
  leds.show();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found - check wiring and address");
  }
  mpuInit();

  lastActivity = millis();
  lastStatTick = millis();
  wakeUp();
}

void loop() {
  unsigned long now = millis();

  // Shake detection
  float mag = readAccelMagnitude();
  bool shaken = fabsf(mag - lastMag) > SHAKE_THRESH;
  lastMag = mag;

  // Buttons (active low)
  bool feed = digitalRead(PIN_BTN1) == LOW;
  bool play = digitalRead(PIN_BTN2) == LOW;

  if (shaken || feed || play) {
    if (asleep || yawning) {
      wakeUp();
    } else {
      lastActivity = now;
    }
    if (feed) { hunger = 0; excitedUntil = now + 1500; }
    if (play) { boredom = 0; excitedUntil = now + 1500; }
    if (shaken && boredom > 0) boredom -= 2;
  }

  // Hunger and boredom grow over time
  if (now - lastStatTick >= STAT_TICK_MS) {
    lastStatTick = now;
    if (hunger < 100) hunger += 5;
    if (!asleep && boredom < 100) boredom += 4;
  }

  // Inactivity: yawn, then sleep
  if (!asleep && !yawning && now - lastActivity > SLEEP_AFTER_MS) {
    yawning = true;
    yawnStart = now;
  }
  if (yawning && now - yawnStart > YAWN_MS) {
    yawning = false;
    asleep = true;
  }

  chooseMood();
  updateLeds();
  drawFace();
  delay(50);
}
