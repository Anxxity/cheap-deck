#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Adafruit_GFX.h>
#include <avr/pgmspace.h>

// Color definitions
#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define CYAN    0x07FF
#define WHITE   0xFFFF
#define YELLOW  0xFFE0

MCUFRIEND_kbv tft;
#define MINPRESSURE 10
#define MAXPRESSURE 1000

const int XP = 6, XM = A2, YP = A1, YM = 7;
const int TS_LEFT = 920, TS_RT = 140, TS_TOP = 120, TS_BOT = 900;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

enum AppState {
  MAIN_MENU,
  SLIDER_MENU
};

AppState currentState = MAIN_MENU;

struct Button {
  int x, y, w, h;
  String label;
  uint16_t color;
  bool pressed;
};

Button buttons[] = {
  {0, 0, 80, 60, "PLAY",   GREEN, false},
  {0, 0, 80, 60, "PAUSE",  RED,   false},
  {0, 0, 80, 60, "STOP",   BLUE,  false},
  {0, 0, 80, 60, "NEXT",   CYAN,  false},
  {0, 0, 80, 60, "BACK",   GREEN, false},
  {0, 0, 80, 60, "MUTE",   CYAN,  false},
  {0, 0, 80, 60, "DEF",    CYAN,  false},
  {0, 0, 80, 60, "SLIDER", CYAN,  false}
};

Button backButton = {10, 10, 100, 50, "HOME", RED, false};

struct Slider {
  int x, y, w, h;
  int value;
  int thumbY;
};

Slider sliders[] = {
  {0, 10, 20, 200, 0, 0},
  {0, 10, 20, 200, 0, 0},
  {0, 10, 20, 200, 0, 0},
  {0, 10, 20, 200, 0, 0}
};

// Layout functions
void layoutButtons() {
  int cols = 3;
  int spacing = 20;
  int btnW = buttons[0].w;
  int btnH = buttons[0].h;
  int startX = (tft.width() - (cols * btnW + (cols - 1) * spacing)) / 2;
  int startY = 10;  // moved up from 30 to 10

  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    int col = i % cols;
    int row = i / cols;
    buttons[i].x = startX + col * (btnW + spacing);
    buttons[i].y = startY + row * (btnH + spacing);
  }
}

void layoutSliders() {
  int rightEdge = tft.width() - 10;
  for (int i = 0; i < sizeof(sliders) / sizeof(sliders[0]); i++) {
    sliders[i].x = rightEdge - (i + 1) * (sliders[i].w + 20);
  }
}

// Image data for placeholder (green square 100x100)
const uint16_t greenSquare[] PROGMEM = {
  0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0,
  0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0,
  // You can expand this array to include more rows for the square
};

// Drawing functions
void drawButton(Button &btn) {
  tft.fillRect(btn.x - 2, btn.y - 2, btn.w + 4, btn.h + 4, BLACK);  // Simulated rounded/shadow
  tft.fillRect(btn.x, btn.y, btn.w, btn.h, btn.color);
  tft.drawRect(btn.x, btn.y, btn.w, btn.h, WHITE);

  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(btn.label, btn.x, btn.y, &x1, &y1, &w, &h);
  int cx = btn.x + (btn.w - w) / 2;
  int cy = btn.y + (btn.h - h) / 2;
  tft.setCursor(cx, cy);
  tft.print(btn.label);
}

void flashButton(Button &btn) {
  tft.fillRect(btn.x, btn.y, btn.w, btn.h, WHITE);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(btn.label, btn.x, btn.y, &x1, &y1, &w, &h);
  tft.setCursor(btn.x + (btn.w - w) / 2, btn.y + (btn.h - h) / 2);
  tft.print(btn.label);
  delay(120);
  drawButton(btn);
}

void drawSlider(Slider &s) {
  tft.drawRect(s.x, s.y, s.w, s.h, WHITE);
  tft.fillRect(s.x + 1, s.y + 1, s.w - 2, s.h - 2, BLACK);
  int thumbHeight = 10;
  s.thumbY = map(s.value, 0, 100, s.y + s.h - thumbHeight, s.y);
  tft.fillRect(s.x + 2, s.thumbY, s.w - 4, thumbHeight, CYAN);
}

// Menus
void drawMainMenu() {
  tft.fillScreen(BLACK);
  layoutButtons();
  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    drawButton(buttons[i]);
  }
  // Display the placeholder image (green square)
  int imageX = 120;  // X coordinate
  int imageY = 120;  // Y coordinate
  for (int i = 0; i < 100; i++) {
    for (int j = 0; j < 100; j++) {
      tft.drawPixel(imageX + i, imageY + j, pgm_read_word(&greenSquare[i]));
    }
  }
}

void drawSliderMenu() {
  tft.fillScreen(BLACK);
  layoutSliders();
  drawButton(backButton);
  for (int i = 0; i < sizeof(sliders) / sizeof(sliders[0]); i++) {
    drawSlider(sliders[i]);
  }
}

// Setup
void setup() {
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  Serial.begin(9600);
  drawMainMenu();  // Display the main menu and image
}

// Touch Loop
void loop() {
  TSPoint p = ts.getPoint();
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    int x = map(p.x, TS_LEFT, TS_RT, 0, tft.width());
    int y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());

    if (currentState == MAIN_MENU) {
      for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
        Button &btn = buttons[i];
        if (x > btn.x && x < btn.x + btn.w && y > btn.y && y < btn.y + btn.h) {
          flashButton(btn);
          Serial.print("Action: ");
          Serial.println(btn.label);
          if (btn.label == "SLIDER") {
            currentState = SLIDER_MENU;
            drawSliderMenu();
            return;
          }
        }
      }

    } else if (currentState == SLIDER_MENU) {
      for (int i = 0; i < sizeof(sliders) / sizeof(sliders[0]); i++) {
        Slider &s = sliders[i];
        if (x > s.x && x < s.x + s.w && y > s.y && y < s.y + s.h) {
          int thumbHeight = 10;
          s.thumbY = constrain(y - thumbHeight / 2, s.y, s.y + s.h - thumbHeight);
          s.value = map(s.thumbY, s.y + s.h - thumbHeight, s.y, 0, 100);
          drawSlider(s);
          Serial.print("Slider ");
          Serial.print(i);
          Serial.print(" Value: ");
          Serial.println(s.value);
          delay(100);
          return;
        }
      }

      if (x > backButton.x && x < backButton.x + backButton.w && y > backButton.y && y < backButton.y + backButton.h) {
        currentState = MAIN_MENU;
        drawMainMenu();
        return;
      }
    }
  }
}
