/*
 * Cheap Deck - Arduino Stream Deck Controller
 * 
 * This sketch provides a touchscreen interface with customizable buttons
 * and volume sliders. It communicates with a PC via serial connection.
 * 
 * Hardware:
 * - Arduino Uno
 * - 2.8" TFT LCD Touch Display 
 * 
 * Libraries Required:
 * - MCUFRIEND_kbv
 * - Adafruit_GFX
 * - TouchScreen
 */
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Adafruit_GFX.h>
#include <avr/pgmspace.h>

// Color definitions are now in config.h
//Hardware ConfiguratioN ----------------
// TFT Displa
MCUFRIEND_kbv tft;

// Touch Screen Configuration
#define MINPRESSURE 10
#define MAXPRESSURE 1000

const int XP = 6, XM = A2, YP = A1, YM = 7; // Touch pins
const int TS_LEFT = 920, TS_RT = 140, TS_TOP = 120, TS_BOT = 900; // Calibration values
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300); // Touch screen resistance

enum AppState {
  MAIN_MENU,
  SLIDER_MENU
};

AppState currentState = MAIN_MENU;

// Configuration is now in config.h file
// 
struct Button {
  int x, y, w, h;      // Position and size
  char label[8];       // Button text (reduced from 10)
  uint16_t color;      // Button color
  char actionId[20];   // Action identifier (reduced from 30)
  bool pressed;        // Press state (for future use)
  const uint16_t* icon;  // Pointer to icon data
  int iconWidth;       // Icon width
  int iconHeight;      // Icon height
  bool hasIcon;        // Flag to indicate if button has an icon
};

struct Slider {
  int x, y, w, h;      // Position and size
  int value;           // Current value (0-100)
  int thumbY;          // Thumb position
  char name[12];       // Slider name (reduced from 20)
  char actionId[20];   // Action identifier (reduced from 30)
};


const int BUTTON_COUNT = sizeof(BUTTON_CONFIG) / sizeof(BUTTON_CONFIG[0]);
const int TOTAL_SLIDERS = sizeof(SLIDER_CONFIG) / sizeof(SLIDER_CONFIG[0]);
const int SLIDER_PAGES = (TOTAL_SLIDERS + SLIDERS_PER_PAGE - 1) / SLIDERS_PER_PAGE;
const int BUTTON_PAGES = (BUTTON_COUNT + BUTTONS_PER_PAGE - 1) / BUTTONS_PER_PAGE;

Button buttons[BUTTON_COUNT];
Slider sliders[TOTAL_SLIDERS];

Button backButton = {10, 45, 60, 35, "HOME", RED, "", false};  // Top left
Button prevButton = {10, 220, NAV_BUTTON_WIDTH, NAV_BUTTON_HEIGHT, "PREV", RED, "", false};
Button nextButton = {200, 220, NAV_BUTTON_WIDTH, NAV_BUTTON_HEIGHT, "NEXT", RED, "", false};
int currentButtonPage = 0;
int currentSliderPage = 0;

// Analog pins for slider control
const int ANALOG_PINS[] = {A8, A9, A10};
const int NUM_ANALOG_PINS = 3;
const int ANALOG_THRESHOLD = 1;  // Minimum change to trigger update

int prevAnalogValues[NUM_ANALOG_PINS] = {0};
unsigned long lastAnalogRead = 0;
const unsigned long ANALOG_READ_INTERVAL = 100;  // Read every 50ms

void handleAnalogSliders() {
  unsigned long currentTime = millis();
  
  // Only read analog values at specified interval
  if (currentTime - lastAnalogRead < ANALOG_READ_INTERVAL) {
    return;
  }
  lastAnalogRead = currentTime;
  
  // Only update when in slider menu
  if (currentState != SLIDER_MENU) {
    return;
  }
  
  // Get current page slider range
  int startIdx = currentSliderPage * SLIDERS_PER_PAGE;
  int endIdx = min(startIdx + SLIDERS_PER_PAGE, TOTAL_SLIDERS);
  
  // Check each analog pin
  for (int i = 0; i < NUM_ANALOG_PINS && i < (endIdx - startIdx); i++) {
    int sliderIdx = startIdx + i;
    
    // Read analog value (0-1023) and map to slider value (0-100)
    int analogValue = analogRead(ANALOG_PINS[i]);
    int sliderValue = map(analogValue, 0, 1023, 0, 100);
    
    // Check if value changed significantly
    if (abs(sliderValue - prevAnalogValues[i]) >= ANALOG_THRESHOLD) {
      // Print raw analog value for debugging
      Serial.print("Analog pin A");
      Serial.print(ANALOG_PINS[i] - A0);  // Convert pin number to A-format
      Serial.print(" raw value: ");
      Serial.print(analogValue);
      Serial.print(" -> ");
      
      // Update slider
      Slider &s = sliders[sliderIdx];
      s.value = sliderValue;
      s.thumbY = map(sliderValue, 0, 100, s.y + s.h - 15, s.y);
      
      // Redraw slider
      drawSlider(s);
      
      // Send value to PC
      Serial.print(s.actionId);
      Serial.print(":");
      Serial.println(s.value);
      
      // Store previous value
      prevAnalogValues[i] = sliderValue;
    }
  }
}

void initAnalogSliders() {
  for (int i = 0; i < NUM_ANALOG_PINS; i++) {
    prevAnalogValues[i] = -1;  // Force initial read
  }
}
// Initialize navigation button labels
void initNavButtons() {
  strcpy(backButton.label, "HOME");
  strcpy(backButton.actionId, "");
  // Initialize prev/next buttons (positions updated in layout functions)
  prevButton.w = NAV_BUTTON_WIDTH;
  prevButton.h = NAV_BUTTON_HEIGHT;
  strcpy(prevButton.label, "PREV");
  strcpy(prevButton.actionId, "");
  prevButton.color = RED;

  nextButton.w = NAV_BUTTON_WIDTH;
  nextButton.h = NAV_BUTTON_HEIGHT;
  strcpy(nextButton.label, "NEXT");
  strcpy(nextButton.actionId, "");
  nextButton.color = RED;
}

void initButtons() {
  for (int i = 0; i < BUTTON_COUNT; i++) {
    buttons[i].w = BUTTON_WIDTH;
    buttons[i].h = BUTTON_HEIGHT;
    // Copy strings properly from config
    strncpy(buttons[i].label, BUTTON_CONFIG[i].label, 7);
    buttons[i].label[7] = '\0';
    buttons[i].color = BUTTON_CONFIG[i].color;
    strncpy(buttons[i].actionId, BUTTON_CONFIG[i].actionId, 19);
    buttons[i].actionId[19] = '\0';
    buttons[i].pressed = false;
    
    // Copy icon data
    buttons[i].icon = BUTTON_CONFIG[i].icon;
    buttons[i].iconWidth = BUTTON_CONFIG[i].iconWidth;
    buttons[i].iconHeight = BUTTON_CONFIG[i].iconHeight;
    buttons[i].hasIcon = (BUTTON_CONFIG[i].icon != NULL);
  }
}

void initSliders() {
  for (int i = 0; i < TOTAL_SLIDERS; i++) {
    sliders[i].w = SLIDER_WIDTH;
    sliders[i].h = SLIDER_HEIGHT;
    sliders[i].y = SLIDER_START_Y;
    sliders[i].value = 100;  // Default to 50%
    sliders[i].thumbY = 0;
    // Copy strings properly from config
    strncpy(sliders[i].name, SLIDER_CONFIG[i].name, 11);
    sliders[i].name[11] = '\0';
    strncpy(sliders[i].actionId, SLIDER_CONFIG[i].actionId, 19);
    sliders[i].actionId[19] = '\0';
  }
}

// Layout functions
void layoutButtons() {
int btnW = BUTTON_WIDTH;
  int btnH = BUTTON_HEIGHT;
  int startX = (tft.width() - (BUTTON_COLS * btnW + (BUTTON_COLS - 1) * BUTTON_SPACING)) / 2;
  
  // Calculate visible buttons for current page
  int startIdx = currentButtonPage * BUTTONS_PER_PAGE;
  int endIdx = min(startIdx + BUTTONS_PER_PAGE, BUTTON_COUNT);
  
  for (int i = startIdx; i < endIdx; i++) {
    int localIdx = i - startIdx;  // Index within current page
    int col = localIdx % BUTTON_COLS;
    int row = localIdx / BUTTON_COLS;
    buttons[i].x = startX + col * (btnW + BUTTON_SPACING);
    buttons[i].y = BUTTON_START_Y + row * (btnH + BUTTON_SPACING);
  }
  
  // Position prev/next navigation buttons for main menu
  prevButton.x = 10;
  prevButton.y = NAV_BUTTON_Y; // from config
  nextButton.x = tft.width() - nextButton.w - 10;
  nextButton.y = NAV_BUTTON_Y;
}

void layoutSliders() {
  int startIdx = currentSliderPage * SLIDERS_PER_PAGE;
  int endIdx = min(startIdx + SLIDERS_PER_PAGE, TOTAL_SLIDERS);
  int visibleCount = endIdx - startIdx;
  
  // Position sliders on the right side, leaving space for buttons on left
  int rightEdge = tft.width() - 10;
  int leftMargin = 100;  // Space for buttons on left
  
  for (int i = 0; i < visibleCount; i++) {
    int sliderIdx = startIdx + i;
    // Distribute sliders evenly in remaining space
    sliders[sliderIdx].x = rightEdge - (i + 1) * (sliders[sliderIdx].w + 25);
  }
    int vSpacing = 6; // vertical gap between stacked nav buttons
  prevButton.x = backButton.x;
  prevButton.y = backButton.y + backButton.h + vSpacing; // below HOME
  nextButton.x = backButton.x;
  nextButton.y = prevButton.y + prevButton.h + vSpacing; // below PREV
}

// Image data for placeholder (green square 100x100)
const uint16_t greenSquare[] PROGMEM = {
  0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0,
  0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0, 0x07E0,
  // You can expand this array to include more rows for the square
};

// Drawing functions
void drawRoundRect(int x, int y, int w, int h, int r, uint16_t color) {
  // Draw filled rounded rectangle using circles at corners
  tft.fillRect(x + r, y, w - 2 * r, h, color);
  tft.fillRect(x, y + r, w, h - 2 * r, color);
  tft.fillCircle(x + r, y + r, r, color);
  tft.fillCircle(x + w - r - 1, y + r, r, color);
  tft.fillCircle(x + r, y + h - r - 1, r, color);
  tft.fillCircle(x + w - r - 1, y + h - r - 1, r, color);
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
