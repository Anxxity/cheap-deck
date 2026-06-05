/*
 * Cheap Deck - Arduino Stream Deck Controller
 * 
 * This sketch provides a touchscreen interface with customizable buttons
 * and volume sliders. It communicates with a PC via serial connection.
 * 
 * Hardware:
 * - Arduino Uno
 * - 2.4" TFT LCD Touch Display (MCUFRIEND_kbv compatible)
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
#include "config.h"

// Color definitions are now in config.h

// ========== Hardware Configuration ==========
// TFT Display
MCUFRIEND_kbv tft;

// Touch Screen Configuration
#define MINPRESSURE 10
#define MAXPRESSURE 1000

const int XP = 6, XM = A2, YP = A1, YM = 7;  // Touch pins
const int TS_LEFT = 920, TS_RT = 140, TS_TOP = 120, TS_BOT = 900;  // Calibration values
const int TS_RESISTANCE = 300;  // Touch screen resistance

TouchScreen ts = TouchScreen(XP, YP, XM, YM, TS_RESISTANCE);

// ========== UI Configuration (from config.h) ==========
// Configuration is now in config.h file

// ========== Application State ==========
enum AppState {
  MAIN_MENU,
  SLIDER_MENU
};

AppState currentState = MAIN_MENU;

// ========== Data Structures ==========
struct Button {
  int x, y, w, h;      // Position and size
  char label[8];       // Button text (reduced from 10)
  uint16_t color;      // Button color
  char actionId[20];   // Action identifier (reduced from 30)
  bool pressed;        // Press state (for future use)
};

struct Slider {
  int x, y, w, h;      // Position and size
  int value;           // Current value (0-100)
  int thumbY;          // Thumb position
  char name[12];       // Slider name (reduced from 20)
  char actionId[20];   // Action identifier (reduced from 30)
};

// Calculate counts from config
const int BUTTON_COUNT = sizeof(BUTTON_CONFIG) / sizeof(BUTTON_CONFIG[0]);
const int TOTAL_SLIDERS = sizeof(SLIDER_CONFIG) / sizeof(SLIDER_CONFIG[0]);
const int SLIDER_PAGES = (TOTAL_SLIDERS + SLIDERS_PER_PAGE - 1) / SLIDERS_PER_PAGE;
const int BUTTON_PAGES = (BUTTON_COUNT + BUTTONS_PER_PAGE - 1) / BUTTONS_PER_PAGE;

// Main menu buttons - loaded from config
Button buttons[BUTTON_COUNT];

// Volume sliders - loaded from config
Slider sliders[TOTAL_SLIDERS];

// Navigation buttons for menus
Button backButton = {10, 45, 60, 35, "HOME", RED, "", false};  // Top left
// Prev/Next navigation buttons (shown when there are multiple pages)
Button prevButton = {10, 220, NAV_BUTTON_WIDTH, NAV_BUTTON_HEIGHT, "PREV", CYAN, "", false};
Button nextButton = {200, 220, NAV_BUTTON_WIDTH, NAV_BUTTON_HEIGHT, "NEXT", CYAN, "", false};

// Current page trackers
int currentButtonPage = 0;
int currentSliderPage = 0;

// ========== Initialization Functions ==========
// Initialize navigation button labels
void initNavButtons() {
  strcpy(backButton.label, "HOME");
  strcpy(backButton.actionId, "");
  // Initialize prev/next buttons (positions updated in layout functions)
  prevButton.w = NAV_BUTTON_WIDTH;
  prevButton.h = NAV_BUTTON_HEIGHT;
  strcpy(prevButton.label, "PREV");
  strcpy(prevButton.actionId, "");
  prevButton.color = CYAN;

  nextButton.w = NAV_BUTTON_WIDTH;
  nextButton.h = NAV_BUTTON_HEIGHT;
  strcpy(nextButton.label, "NEXT");
  strcpy(nextButton.actionId, "");
  nextButton.color = CYAN;
}

// ========== Layout Functions ==========
/**
 * Initialize buttons from config
 */
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
  }
}

/**
 * Initialize sliders from config
 */
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

/**
 * Calculate and set button positions in a grid layout for current page
 */
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

/**
 * Calculate and set slider positions for current page
 */
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
  
  // Position prev/next navigation buttons for slider menu
  // Stack HOME, PREV, NEXT vertically at the top-left for slider page
  int vSpacing = 6; // vertical gap between stacked nav buttons
  prevButton.x = backButton.x;
  prevButton.y = backButton.y + backButton.h + vSpacing; // below HOME
  nextButton.x = backButton.x;
  nextButton.y = prevButton.y + prevButton.h + vSpacing; // below PREV

  // Navigation buttons are already positioned (see initialization)
  // HOME at x=10, y=45 (top left)
  // PREV at x=10, y=190 (bottom left)
  // NEXT at x=80, y=190 (next to PREV)
}

// ========== Graphics Data ==========
// Removed to save memory

// ========== Drawing Functions ==========
/**
 * Draw rounded rectangle for modern button appearance
 */
void drawRoundRect(int x, int y, int w, int h, int r, uint16_t color) {
  // Draw filled rounded rectangle using circles at corners
  tft.fillRect(x + r, y, w - 2 * r, h, color);
  tft.fillRect(x, y + r, w, h - 2 * r, color);
  tft.fillCircle(x + r, y + r, r, color);
  tft.fillCircle(x + w - r - 1, y + r, r, color);
  tft.fillCircle(x + r, y + h - r - 1, r, color);
  tft.fillCircle(x + w - r - 1, y + h - r - 1, r, color);
}

/**
 * Create gradient effect by drawing horizontal lines with varying brightness
 */
void drawGradientButton(int x, int y, int w, int h, uint16_t baseColor) {
  // Extract RGB components
  uint8_t r = (baseColor >> 11) & 0x1F;
  uint8_t g = (baseColor >> 5) & 0x3F;
  uint8_t b = baseColor & 0x1F;
  
  // Draw gradient from lighter at top to darker at bottom
  for (int i = 0; i < h; i++) {
    float factor = 1.0 - (i * 0.3 / h);  // Reduce brightness gradually
    uint8_t nr = r * factor;
    uint8_t ng = g * factor;
    uint8_t nb = b * factor;
    uint16_t lineColor = (nr << 11) | (ng << 5) | nb;
    tft.drawFastHLine(x, y + i, w, lineColor);
  }
}

/**
 * Draw a button with modern styling: 3D shadow, rounded corners, gradient
 */
void drawButton(Button &btn) {
  const int cornerRadius = 8;
  const int shadowOffset = 3;
  
  // Draw 3D shadow effect
  // drawRoundRect(btn.x + shadowOffset, btn.y + shadowOffset, btn.w, btn.h, cornerRadius, 0x2104);  // Dark gray shadow
  
  // Draw button with gradient
  // drawGradientButton(btn.x, btn.y, btn.w, btn.h, btn.color);
  
  // Draw rounded border for clean edges
  drawRoundRect(btn.x, btn.y, btn.w, btn.h, cornerRadius, btn.color);
  
  // Draw glossy highlight at top (lighter shade)
  uint8_t r = ((btn.color >> 11) & 0x1F) * 1.3;
  uint8_t g = ((btn.color >> 5) & 0x3F) * 1.3;
  uint8_t b = (btn.color & 0x1F) * 1.3;
  if (r > 31) r = 31;
  if (g > 63) g = 63;
  if (b > 31) b = 31;
  uint16_t highlightColor = (r << 11) | (g << 5) | b;
  
  // Draw subtle highlight arc at top
  for (int i = 0; i < 8; i++) {
    tft.drawFastHLine(btn.x + cornerRadius, btn.y + i, btn.w - 2 * cornerRadius, highlightColor);
  }
  
  // Draw border with lighter shade for depth
  tft.drawRoundRect(btn.x, btn.y, btn.w, btn.h, cornerRadius, WHITE);
  
  // Draw label with shadow for better readability
  tft.setTextSize(2);
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(btn.label, btn.x, btn.y, &x1, &y1, &w, &h);
  int cx = btn.x + (btn.w - w) / 2;
  int cy = btn.y + (btn.h - h) / 2;
  
  // Text shadow
  tft.setTextColor(BLACK);
  tft.setCursor(cx + 1, cy + 1);
  tft.print(btn.label);
  
  // Main text
  tft.setTextColor(WHITE);
  tft.setCursor(cx, cy);
  tft.print(btn.label);
}

/**
 * Flash button with animated press effect
 */
void flashButton(Button &btn) {
  const int cornerRadius = 8;
  const int pressDepth = 2;
  
  // Pressed state: move button down and darken
  uint8_t r = ((btn.color >> 11) & 0x1F) * 0.6;
  uint8_t g = ((btn.color >> 5) & 0x3F) * 0.6;
  uint8_t b = (btn.color & 0x1F) * 0.6;
  uint16_t pressedColor = (r << 11) | (g << 5) | b;
  
  // Draw pressed button (offset down)
  drawRoundRect(btn.x + pressDepth, btn.y + pressDepth, btn.w, btn.h, cornerRadius, pressedColor);
  tft.drawRoundRect(btn.x + pressDepth, btn.y + pressDepth, btn.w, btn.h, cornerRadius, WHITE);
  
  // Draw label
  tft.setTextSize(2);
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(btn.label, btn.x, btn.y, &x1, &y1, &w, &h);
  int cx = btn.x + pressDepth + (btn.w - w) / 2;
  int cy = btn.y + pressDepth + (btn.h - h) / 2;
  
  tft.setTextColor(WHITE);
  tft.setCursor(cx, cy);
  tft.print(btn.label);
  
  delay(FLASH_DURATION);
  
  // Restore original appearance with animation
  drawButton(btn);
}

/**
 * Draw a vertical slider with modern styling and gradient fill
 */
void drawSlider(Slider &s) {
  const int thumbHeight = 15;
  const int thumbWidth = s.w + 4;
  const int trackRadius = 3;
  
  // Clear the entire slider area (including label area below)
  tft.fillRect(s.x - 5, s.y - 5, s.w + 10, s.h + 35, BLACK);
  
  // Draw outer track border with rounded edges
  tft.drawRoundRect(s.x - 1, s.y - 1, s.w + 2, s.h + 2, trackRadius, WHITE);
  
  // Draw track background
  tft.fillRoundRect(s.x, s.y, s.w, s.h, trackRadius, 0x2104);  // Dark gray
  
  // Calculate thumb position
  s.thumbY = map(s.value, 0, 100, s.y + s.h - thumbHeight, s.y);
  
  // Draw filled portion with gradient (from bottom to thumb)
  int fillHeight = (s.y + s.h) - s.thumbY;
  for (int i = 0; i < fillHeight; i++) {
    // Gradient from cyan to blue
    float factor = (float)i / fillHeight;
    uint8_t r = 0;
    uint8_t g = (1.0 - factor) * 31;
    uint8_t b = 31;
    uint16_t lineColor = (r << 11) | (g << 6) | b;
    tft.drawFastHLine(s.x + 2, s.thumbY + i, s.w - 4, lineColor);
  }
  
  // Draw stylish thumb with 3D effect
  // Shadow
  tft.fillRoundRect(s.x - 2 + 1, s.thumbY + 1, thumbWidth, thumbHeight, 4, 0x2104);
  // Main thumb
  tft.fillRoundRect(s.x - 2, s.thumbY, thumbWidth, thumbHeight, 4, CYAN);
  // Highlight
  tft.fillRoundRect(s.x - 1, s.thumbY + 1, thumbWidth - 2, 3, 2, WHITE);
  // Border
  tft.drawRoundRect(s.x - 2, s.thumbY, thumbWidth, thumbHeight, 4, WHITE);
  
  // Draw percentage value on thumb
  tft.setTextColor(BLACK);
  tft.setTextSize(1);
  String valueStr = String(s.value);
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(valueStr, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(s.x + (s.w - w) / 2, s.thumbY + (thumbHeight - h) / 2 + 1);
  tft.print(valueStr);
  
  // Draw slider label below with background
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.getTextBounds(s.name, 0, 0, &x1, &y1, &w, &h);
  int labelX = s.x + (s.w - w) / 2 - 2;
  int labelY = s.y + s.h + 8;
  
  // Label background for better readability
  tft.fillRoundRect(labelX - 2, labelY - 2, w + 4, h + 4, 3, 0x2104);
  tft.drawRoundRect(labelX - 2, labelY - 2, w + 4, h + 4, 3, CYAN);
  
  tft.setCursor(labelX, labelY);
  tft.print(s.name);
}

// ========== Menu Drawing Functions ==========
/**
 * Draw animated background pattern
 */
void drawBackgroundPattern() {
  // Draw subtle grid pattern
  for (int x = 0; x < tft.width(); x += 20) {
    tft.drawFastVLine(x, 0, tft.height(), 0x2104);
  }
  for (int y = 0; y < tft.height(); y += 20) {
    tft.drawFastHLine(0, y, tft.width(), 0x2104);
  }
}

/**
 * Draw stylish header with title
 */
void drawHeader(String title) {
  const int headerHeight = 30;
  
  // Draw gradient header background
  for (int i = 0; i < headerHeight; i++) {
    float factor = 1.0 - (i * 0.5 / headerHeight);
    uint8_t b = 15 * factor;
    uint16_t lineColor = (0 << 11) | (0 << 5) | b;
    tft.drawFastHLine(0, i, tft.width(), lineColor);
  }
  
  // Draw title with glow effect
  tft.setTextSize(2);
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  int cx = (tft.width() - w) / 2;
  int cy = (headerHeight - h) / 2;
  
  // Glow effect (multiple offset shadows)
  tft.setTextColor(CYAN);
  tft.setCursor(cx - 1, cy);
  tft.print(title);
  tft.setCursor(cx + 1, cy);
  tft.print(title);
  tft.setCursor(cx, cy - 1);
  tft.print(title);
  tft.setCursor(cx, cy + 1);
  tft.print(title);
  
  // Main text
  tft.setTextColor(WHITE);
  tft.setCursor(cx, cy);
  tft.print(title);
  
  // Draw separator line
  tft.drawFastHLine(0, headerHeight, tft.width(), CYAN);
  tft.drawFastHLine(0, headerHeight + 1, tft.width(), 0x2104);
}

/**
 * Draw the main menu with buttons and pagination
 */
void drawMainMenu() {
  tft.fillScreen(GREY);
  drawBackgroundPattern();
  
  // Draw header with page indicator
  String headerTitle = "CHEAP DECK";
  if (BUTTON_PAGES > 1) {
    headerTitle += " [" + String(currentButtonPage + 1) + "/" + String(BUTTON_PAGES) + "]";
  }
  drawHeader(headerTitle);
  
  layoutButtons();
  
  // Draw buttons for current page
  int startIdx = currentButtonPage * BUTTONS_PER_PAGE;
  int endIdx = min(startIdx + BUTTONS_PER_PAGE, BUTTON_COUNT);
  
  for (int i = startIdx; i < endIdx; i++) {
    drawButton(buttons[i]);
  }
  
  // Draw page dots and navigation hints if multiple pages
  if (BUTTON_PAGES > 1) {
    // Draw page navigation hints
    tft.setTextColor(WHITE);
    tft.setTextSize(1);
    
    // Draw Prev/Next buttons (only when applicable)
    if (currentButtonPage > 0) {
      drawButton(prevButton);
    }
    if (currentButtonPage < BUTTON_PAGES - 1) {
      drawButton(nextButton);
    }

    // Draw page dots at bottom
    int dotY = tft.height() - 10;
    int dotSpacing = 15;
    int totalDotWidth = BUTTON_PAGES * dotSpacing;
    int dotStartX = (tft.width() - totalDotWidth) / 2;
    
    for (int i = 0; i < BUTTON_PAGES; i++) {
      int dotX = dotStartX + i * dotSpacing;
      if (i == currentButtonPage) {
        // Active page - larger filled circle with glow
        tft.fillCircle(dotX, dotY, 5, CYAN);
        tft.drawCircle(dotX, dotY, 6, WHITE);
        tft.drawCircle(dotX, dotY, 7, 0x2104);
      } else {
        // Inactive page - smaller circle
        tft.fillCircle(dotX, dotY, 3, 0x4208);
        tft.drawCircle(dotX, dotY, 4, WHITE);
      }
    }
  }
}

/**
 * Draw the slider menu with volume controls and stylish pagination
 */
void drawSliderMenu() {
  tft.fillScreen(BLACK);
  
  // Draw header with page indicator
  String headerTitle = "VOLUME MIXER";
  if (SLIDER_PAGES > 1) {
    headerTitle += " [" + String(currentSliderPage + 1) + "/" + String(SLIDER_PAGES) + "]";
  }
  drawHeader(headerTitle);
  
  layoutSliders();
  drawButton(backButton);
  
  // Draw sliders for current page
  int startIdx = currentSliderPage * SLIDERS_PER_PAGE;
  int endIdx = min(startIdx + SLIDERS_PER_PAGE, TOTAL_SLIDERS);
  
  for (int i = startIdx; i < endIdx; i++) {
    drawSlider(sliders[i]);
  }
  
  // Draw page dots and navigation hints if multiple pages
  if (SLIDER_PAGES > 1) {
    // Draw page navigation hints
    tft.setTextColor(WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, 200);
    
    // Draw Prev/Next buttons (only when applicable)
    if (currentSliderPage > 0) {
      drawButton(prevButton);
    }
    if (currentSliderPage < SLIDER_PAGES - 1) {
      drawButton(nextButton);
    }
    
  // Draw page dots below navigation hints
  int dotY = 220;
    int dotSpacing = 12;
    int dotStartX = 20;
    
    for (int i = 0; i < SLIDER_PAGES; i++) {
      int dotX = dotStartX + i * dotSpacing;
      if (i == currentSliderPage) {
        // Active page - larger filled circle with glow
        tft.fillCircle(dotX, dotY, 4, CYAN);
        tft.drawCircle(dotX, dotY, 5, WHITE);
      } else {
        // Inactive page - smaller circle
        tft.fillCircle(dotX, dotY, 2, 0x4208);
        tft.drawCircle(dotX, dotY, 3, WHITE);
      }
    }
  }
}

// ========== Arduino Setup ==========
/**
 * Initialize hardware and display
 */
void setup() {
  // Initialize TFT display
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(1);  // Landscape orientation
  tft.fillScreen(BLACK);
  
  // Initialize serial communication
  Serial.begin(9600);
  
  // Initialize buttons and sliders from config
  initNavButtons();
  initButtons();
  initSliders();
  
  // Display main menu
  drawMainMenu();
}

// ========== Main Loop ==========
/**
 * Main loop - handle touch input and update display
 */
void loop() {
  // Read touch input
  TSPoint p = ts.getPoint();
  
  // Restore pin modes (required after reading touch)
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);

  // Check if touch is within valid pressure range
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    // Map touch coordinates to screen coordinates
    int x = map(p.x, TS_LEFT, TS_RT, 0, tft.width());
    int y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());

    // Handle main menu touches
    if (currentState == MAIN_MENU) {
      handleMainMenuTouch(x, y);
    } 
    // Handle slider menu touches
    else if (currentState == SLIDER_MENU) {
      handleSliderMenuTouch(x, y);
    }
  }
}

/**
 * Handle touch events in main menu
 */
void handleMainMenuTouch(int x, int y) {
  // Check buttons for current page
  int startIdx = currentButtonPage * BUTTONS_PER_PAGE;
  int endIdx = min(startIdx + BUTTONS_PER_PAGE, BUTTON_COUNT);
  
  
  for (int i = startIdx; i < endIdx; i++) {
    Button &btn = buttons[i];
    
    // Check if button was touched
    if (x > btn.x && x < btn.x + btn.w && y > btn.y && y < btn.y + btn.h) {
      flashButton(btn);
      
      // Send action to PC
      Serial.print("Action: ");
      Serial.println(btn.actionId);
      
      // Switch to slider menu if slider button pressed
      if (strcmp(btn.actionId, "open_sliders") == 0) {
        currentState = SLIDER_MENU;
        currentSliderPage = 0;  // Reset to first page
        drawSliderMenu();
      }
      return;
    }
  }

  // Check Prev/Next navigation buttons
  if (BUTTON_PAGES > 1) {
    // Prev
    if (currentButtonPage > 0 && x > prevButton.x && x < prevButton.x + prevButton.w &&
        y > prevButton.y && y < prevButton.y + prevButton.h) {
  currentButtonPage--;
  drawMainMenu();
  delay(200);
      return;
    }
    // Next
    if (currentButtonPage < BUTTON_PAGES - 1 && x > nextButton.x && x < nextButton.x + nextButton.w &&
        y > nextButton.y && y < nextButton.y + nextButton.h) {
      currentButtonPage++;
      drawMainMenu();
      delay(200);
      return;
    }
  }
}

/**
 * Handle touch events in slider menu
 */
void handleSliderMenuTouch(int x, int y) {
  const int thumbHeight = 15;  // Must match thumbHeight in drawSlider
  
  // Check slider touches for current page
  int startIdx = currentSliderPage * SLIDERS_PER_PAGE;
  int endIdx = min(startIdx + SLIDERS_PER_PAGE, TOTAL_SLIDERS);
  
  for (int i = startIdx; i < endIdx; i++) {
    Slider &s = sliders[i];
    
    if (x > s.x && x < s.x + s.w && y > s.y && y < s.y + s.h) {
      // Update slider value based on touch position
      s.thumbY = constrain(y - thumbHeight / 2, s.y, s.y + s.h - thumbHeight);
      s.value = map(s.thumbY, s.y + s.h - thumbHeight, s.y, 0, 100);
      
      // Redraw slider
      drawSlider(s);
      
      // Send value to PC with action ID (format: actionId:value)
      Serial.print(s.actionId);
      Serial.print(":");
      Serial.println(s.value);
      
      delay(DEBOUNCE_DELAY);
      return;
    }
  }

  // Prev/Next navigation for slider pages
  if (SLIDER_PAGES > 1) {
    // Prev
    if (currentSliderPage > 0 && x > prevButton.x && x < prevButton.x + prevButton.w &&
        y > prevButton.y && y < prevButton.y + prevButton.h) {
      currentSliderPage--;
      drawSliderMenu();
      delay(200);
      return;
    }
    // Next
    if (currentSliderPage < SLIDER_PAGES - 1 && x > nextButton.x && x < nextButton.x + nextButton.w &&
        y > nextButton.y && y < nextButton.y + nextButton.h) {
      currentSliderPage++;
      drawSliderMenu();
      delay(200);
      return;
    }
  }

  // Check back button touch
  if (x > backButton.x && x < backButton.x + backButton.w && 
      y > backButton.y && y < backButton.y + backButton.h) {
    currentState = MAIN_MENU;
    drawMainMenu();
    return;
  }
}
