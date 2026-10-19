/*
  =====================================================
  GAME SHOW BUZZER SYSTEM
  ATtiny85 + 2x Arcade Buttons + 2x PL9823 LEDs
  =====================================================

  Wiring (ATtiny85 DIP8):
              +-\/-+
    RESET PB5 1|    |8  VCC (5V)
          PB3 2|    |7  PB2 (LED1 data — button 1)
          PB4 3|    |6  PB1 (BTN2)
          GND 4|    |5  PB0 (BTN1)
              +----+

    Button 1:  PB0 (pin 5) — wire button between PB0 and GND
    Button 2:  PB1 (pin 6) — wire button between PB1 and GND
    PL9823 1:  PB2 (pin 7) — data line for button 1's LED
    PL9823 2:  PB3 (pin 2) — data line for button 2's LED
    VCC:       pin 8 — 5V
    GND:       pin 4 — GND

  Arduino IDE upload settings:
    Board:      ATtiny25/45/85
    Processor:  ATtiny85
    Clock:      Internal 8 MHz  <-- burn bootloader first to set fuse
    Programmer: USBtinyISP
    Upload:     Sketch > Upload Using Programmer

  Library required:
    Adafruit NeoPixel (install via Library Manager)

  NOTE: PL9823 LEDs use RGB byte order — NEO_RGB is used, not NEO_GRB.
  =====================================================
*/

#include <Adafruit_NeoPixel.h>

#define BTN1_PIN    0   // PB0
#define BTN2_PIN    1   // PB1
#define NEO1_PIN    2   // PB2
#define NEO2_PIN    3   // PB3

Adafruit_NeoPixel strip1(1, NEO1_PIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel strip2(1, NEO2_PIN, NEO_RGB + NEO_KHZ800);

const unsigned long COUNTDOWN_MS   = 10000UL;  // 10-second countdown
const unsigned long FLASH_INTERVAL = 400UL;    // flash speed when time expires

enum State { IDLE, COUNTDOWN, FLASHING };

State         gameState = IDLE;
int           winner    = 0;
unsigned long buzzTime  = 0;
bool          flashOn   = false;
unsigned long lastFlash = 0;

uint32_t colorBlue()  { return Adafruit_NeoPixel::Color(0,   0,   200); }
uint32_t colorRed()   { return Adafruit_NeoPixel::Color(220, 0,   0);   }
uint32_t colorOff()   { return Adafruit_NeoPixel::Color(0,   0,   0);   }

// Smooth green -> red fade (t: 0.0 = green, 1.0 = red)
uint32_t colorGreenToRed(float t) {
  t = constrain(t, 0.0f, 1.0f);
  uint8_t r = (uint8_t)(220.0f * t);
  uint8_t g = (uint8_t)(220.0f * (1.0f - t));
  return Adafruit_NeoPixel::Color(r, g, 0);
}

void setPixel(int button, uint32_t color) {
  if (button == 1) { strip1.setPixelColor(0, color); strip1.show(); }
  else             { strip2.setPixelColor(0, color); strip2.show(); }
}

void setBoth(uint32_t color) {
  strip1.setPixelColor(0, color); strip1.show();
  strip2.setPixelColor(0, color); strip2.show();
}

void setup() {
  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  strip1.begin();
  strip2.begin();
  strip1.setBrightness(200);
  strip2.setBrightness(200);
  setBoth(colorBlue());
}

void loop() {
  bool btn1 = (digitalRead(BTN1_PIN) == LOW);
  bool btn2 = (digitalRead(BTN2_PIN) == LOW);

  switch (gameState) {

    // Both LEDs blue, waiting for first press
    case IDLE:
      if (btn1 && !btn2) {
        winner    = 1;
        buzzTime  = millis();
        gameState = COUNTDOWN;
        setPixel(winner,     colorGreenToRed(0.0f));  // winner starts green
        setPixel(3 - winner, colorOff());              // loser goes dark
      } else if (btn2 && !btn1) {
        winner    = 2;
        buzzTime  = millis();
        gameState = COUNTDOWN;
        setPixel(winner,     colorGreenToRed(0.0f));
        setPixel(3 - winner, colorOff());
      }
      break;

    // Winner fades green -> red over COUNTDOWN_MS, loser stays off
    case COUNTDOWN: {
      unsigned long elapsed = millis() - buzzTime;
      float t = (float)elapsed / (float)COUNTDOWN_MS;
      setPixel(winner,     colorGreenToRed(t));
      setPixel(3 - winner, colorOff());
      if (elapsed >= COUNTDOWN_MS) {
        gameState = FLASHING;
        lastFlash = millis();
        flashOn   = true;
        setPixel(winner,     colorRed());
        setPixel(3 - winner, colorOff());
      }
      break;
    }

    // Time expired — winner flashes red, press both buttons simultaneously to reset
    case FLASHING:
      if (millis() - lastFlash >= FLASH_INTERVAL) {
        flashOn   = !flashOn;
        lastFlash = millis();
        setPixel(winner,     flashOn ? colorRed() : colorOff());
        setPixel(3 - winner, colorOff());
      }
      if (btn1 && btn2) {
        delay(50);  // debounce
        if (digitalRead(BTN1_PIN) == LOW && digitalRead(BTN2_PIN) == LOW) {
          winner    = 0;
          gameState = IDLE;
          setBoth(colorBlue());
          while (digitalRead(BTN1_PIN) == LOW || digitalRead(BTN2_PIN) == LOW) {
            delay(10);
          }
        }
      }
      break;
  }
}
