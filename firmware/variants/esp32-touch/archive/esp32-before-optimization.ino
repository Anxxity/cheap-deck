/*
 * Cheap Deck - ESP32 Stream Deck Controller
 * 
 * This sketch provides a touchscreen interface with customizable buttons
 * and volume sliders. It communicates with a PC via serial connection.
 * 
 * Hardware:
 * - ESP32 DevKit
 * - 2.8" TFT LCD Touch Display 
 * 
 * Libraries Required:
 * - TFT_eSPI
 * - XPT2046_Touchscreen
 */

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "config.h"
#include "image.h"

// Color definitions are now in config.h

// ========== Hardware Configuration ==========
// TFT Display
TFT_eSPI tft = TFT_eSPI();

// Touch Screen Configuration
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Touch calibration values (adjust as needed)3800
const int TS_LEFT = 3700, TS_RT = 200, TS_TOP = 3800, TS_BOT = 240;

// ========== UI Configuration (from config.h) ==========
// Configuration is now in config.h file

// ========== Application State ==========
enum AppState {
  MAIN_MENU,
  SLIDER_MENU, // for slider 
   FOLDER_MENU // for folder 
};

AppState currentState = MAIN_MENU;

// ========== Data Structures ==========
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
  ButtonType type;     // folder or not
  int folderIndex;     // idenxz  of folder 
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
Button backButton = {10, 45, 60, 35, "HOME", RED, "", false, NULL, 0, 0, false};  // Top left
// Prev/Next navigation buttons (shown when there are multiple pages)
Button prevButton = {10, 220, NAV_BUTTON_WIDTH, NAV_BUTTON_HEIGHT, "PREV", RED, "", false, NULL, 0, 0, false};
Button nextButton = {200, 220, NAV_BUTTON_WIDTH, NAV_BUTTON_HEIGHT, "NEXT", RED, "", false, NULL, 0, 0, false};

// folder  idk want i am doing 

Button folderButtons[20];   // max buttons inside folder
int folderButtonCount = 0;
int currentFolderIndex = -1;
int currentFolderPage = 0;

const int FOLDER_PAGES = (folderButtonCount + BUTTONS_PER_PAGE - 1) / BUTTONS_PER_PAGE;



// Current page trackers
int currentButtonPage = 0;
int currentSliderPage = 0;

// ========== Analog Slider Control ==========
// Analog pins for slider control


// ========== Analog Slider Control Functions ==========
/**
 * Read analog pins and update corresponding slider values
 * A8 controls first slider on current page, A9 second, A10 third
 */
void handleAnalogSliders() {
  // Analog slider control (todo)
}


void initAnalogSliders() {
  // Analog slider initialization (todo)
}

// ========== Initialization Functions ==========
// Initialize navigation button labels
void initNavButtons() {
  strcpy(backButton.label, "HOME");
  strcpy(backButton.actionId, "");
  // Initialize prev/next buttons (positions updated in layout functions)
  prevButton.w = NAV_BUTTON_WIDTH;
  prevButton.h = NAV_BUTTON_HEIGHT;
  strcpy(prevButton.label, "<");
  strcpy(prevButton.actionId, "");
  prevButton.color = RED;

  nextButton.w = NAV_BUTTON_WIDTH;
  nextButton.h = NAV_BUTTON_HEIGHT;
  strcpy(nextButton.label, ">");
  strcpy(nextButton.actionId, "");
  nextButton.color = RED;
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
    
    // Copy icon data
    buttons[i].icon = BUTTON_CONFIG[i].icon;
    buttons[i].iconWidth = BUTTON_CONFIG[i].iconWidth;
    buttons[i].iconHeight = BUTTON_CONFIG[i].iconHeight;
    buttons[i].hasIcon = (BUTTON_CONFIG[i].icon != NULL);
    buttons[i].type = BUTTON_CONFIG[i].type;
    buttons[i].folderIndex = BUTTON_CONFIG[i].folderIndex;
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
  int startX = (SCREEN_WIDTH - (BUTTON_COLS * btnW + (BUTTON_COLS - 1) * BUTTON_SPACING)) / 2;
  
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
  nextButton.x = SCREEN_WIDTH - nextButton.w - 10;
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
  int rightEdge = SCREEN_WIDTH - 10;
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

// ========== Drawing Functions ==========
/**
 * Draw rounded rectangle for modern button appearance
 */
void drawRoundRect(int x, int y, int w, int h, int r, uint16_t color) {
  // TFT_eSPI has built-in fillRoundRect
  tft.fillRoundRect(x, y, w, h, r, color);
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
  tft.drawRoundRect(btn.x, btn.y, btn.w, btn.h, cornerRadius, TFT_WHITE);
  
  // Draw icon if available
  if (btn.hasIcon && btn.icon != NULL) {
    // Calculate icon position (centered in button)
    int iconX = btn.x + (btn.w - btn.iconWidth) / 2;
    int iconY = btn.y + (btn.h - btn.iconHeight) / 2;
    
    // Reserve space for icon at top, text will be below
    int textAreaY = iconY + btn.iconHeight + 5;  // 5 pixels spacing below icon
    
    // Draw icon (ESP32 doesn't need PROGMEM, data is in RAM)
    tft.pushImage(iconX, iconY, btn.iconWidth, btn.iconHeight, (uint16_t*)btn.icon);
    
    // Draw label below icon with shadow for better readability
    tft.setTextSize(1);  // Smaller text when icon is present
    uint16_t w = tft.textWidth(btn.label);
    uint16_t h = tft.fontHeight();
    int cx = btn.x + (btn.w - w) / 2;
    int cy = textAreaY;
    
    // Text shadow
    tft.setTextColor(TFT_BLACK, btn.color);
    tft.setCursor(cx + 1, cy + 1);
    tft.print(btn.label);
    
    // Main text
    tft.setTextColor(TFT_WHITE, btn.color);
    tft.setCursor(cx, cy);
    tft.print(btn.label);
  } else {
    // Draw label with shadow for better readability (no icon case)
    tft.setTextSize(2);
    uint16_t w = tft.textWidth(btn.label);
    uint16_t h = tft.fontHeight();
    int cx = btn.x + (btn.w - w) / 2;
    int cy = btn.y + (btn.h - h) / 2;
    
    // Text shadow
    tft.setTextColor(TFT_BLACK, btn.color);
    tft.setCursor(cx + 1, cy + 1);
    tft.print(btn.label);
    
    // Main text
    tft.setTextColor(TFT_WHITE, btn.color);
    tft.setCursor(cx, cy);
    tft.print(btn.label);
  }
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
  tft.drawRoundRect(btn.x + pressDepth, btn.y + pressDepth, btn.w, btn.h, cornerRadius, TFT_WHITE);
  
  // Draw label
  tft.setTextSize(2);
  uint16_t w = tft.textWidth(btn.label);
  uint16_t h = tft.fontHeight();
  int cx = btn.x + pressDepth + (btn.w - w) / 2;
  int cy = btn.y + pressDepth + (btn.h - h) / 2;
  
  tft.setTextColor(TFT_WHITE, pressedColor);
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
  tft.fillRect(s.x - 5, s.y - 5, s.w + 10, s.h + 35, TFT_BLACK);
  
  // Draw outer track border with rounded edges
  tft.drawRoundRect(s.x - 1, s.y - 1, s.w + 2, s.h + 2, trackRadius, TFT_WHITE);
  
  // Draw track background
  tft.fillRoundRect(s.x, s.y, s.w, s.h, trackRadius, 0x2104);  // Dark gray
  
  // Calculate thumb position
  s.thumbY = map(s.value, 0, 100, s.y + s.h - thumbHeight, s.y);
  
  // Draw filled portion with gradient (from bottom to thumb)
  int fillHeight = (s.y + s.h) - s.thumbY;
  for (int i = 0; i < fillHeight; i++) {
    // Gradient from RED to blue
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
  tft.fillRoundRect(s.x - 2, s.thumbY, thumbWidth, thumbHeight, 4, TFT_RED);
  // Highlight
  tft.fillRoundRect(s.x - 1, s.thumbY + 1, thumbWidth - 2, 3, 2, TFT_WHITE);
  // Border
  tft.drawRoundRect(s.x - 2, s.thumbY, thumbWidth, thumbHeight, 4, TFT_WHITE);
  
  // Draw percentage value on thumb
  tft.setTextColor(TFT_BLACK, TFT_RED);
  tft.setTextSize(1);
  String valueStr = String(s.value);
  uint16_t w = tft.textWidth(valueStr);
  uint16_t h = tft.fontHeight();
  tft.setCursor(s.x + (s.w - w) / 2, s.thumbY + (thumbHeight - h) / 2 + 1);
  tft.print(valueStr);
  
  // Draw slider label below with background
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  w = tft.textWidth(s.name);
  h = tft.fontHeight();
  int labelX = s.x + (s.w - w) / 2 - 2;
  int labelY = s.y + s.h + 8;
  
  // Label background for better readability
  tft.fillRoundRect(labelX - 2, labelY - 2, w + 4, h + 4, 3, 0x2104);
  tft.drawRoundRect(labelX - 2, labelY - 2, w + 4, h + 4, 3, TFT_RED);
  
  tft.setCursor(labelX, labelY);
  tft.print(s.name);
}

// ========== Menu Drawing Functions ==========
/**
 * Draw animated background pattern
 */
void drawBackgroundPattern() {
  // Draw subtle grid pattern
  for (int x = 0; x < SCREEN_WIDTH; x += 20) {
    tft.drawFastVLine(x, 0, SCREEN_HEIGHT, 0x2104);
  }
  for (int y = 0; y < SCREEN_HEIGHT; y += 20) {
    tft.drawFastHLine(0, y, SCREEN_WIDTH, 0x2104);
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
  uint16_t w = tft.textWidth(title);
  uint16_t h = tft.fontHeight();
  int cx = (SCREEN_WIDTH - w) / 2;
  int cy = (headerHeight - h) / 2;
  
  // Glow effect (multiple offset shadows)
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setCursor(cx - 1, cy);
  tft.print(title);
  tft.setCursor(cx + 1, cy);
  tft.print(title);
  tft.setCursor(cx, cy - 1);
  tft.print(title);
  tft.setCursor(cx, cy + 1);
  tft.print(title);
  
  // Main text
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(cx, cy);
  tft.print(title);
  
  // Draw separator line
  tft.drawFastHLine(0, headerHeight, SCREEN_WIDTH, TFT_RED);
  tft.drawFastHLine(0, headerHeight + 1, SCREEN_WIDTH, 0x2104);
}

/**
 * Draw the main menu with buttons and pagination
 */
void drawMainMenu() {
  tft.fillScreen(0x7BEF);  // GREY
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
    tft.setTextColor(TFT_WHITE, 0x7BEF);
    tft.setTextSize(1);
    
    // Draw Prev/Next buttons (only when applicable)
    if (currentButtonPage > 0) {
      drawButton(prevButton);
    }
    if (currentButtonPage < BUTTON_PAGES - 1) {
      drawButton(nextButton);
    }

    // Draw page dots at bottom
    int dotY = SCREEN_HEIGHT - 10;
    int dotSpacing = 15;
    int totalDotWidth = BUTTON_PAGES * dotSpacing;
    int dotStartX = (SCREEN_WIDTH - totalDotWidth) / 2;
    
    for (int i = 0; i < BUTTON_PAGES; i++) {
      int dotX = dotStartX + i * dotSpacing;
      if (i == currentButtonPage) {
        // Active page - larger filled circle with glow
        tft.fillCircle(dotX, dotY, 5, TFT_RED);
        tft.drawCircle(dotX, dotY, 6, TFT_WHITE);
        tft.drawCircle(dotX, dotY, 7, 0x2104);
      } else {
        // Inactive page - smaller circle
        tft.fillCircle(dotX, dotY, 3, 0x4208);
        tft.drawCircle(dotX, dotY, 4, TFT_WHITE);
      }
    }
  }
}

/**
 * Draw the slider menu with volume controls and stylish pagination
 */

 void positionHomeDefault() {
  backButton.x = 10;
  backButton.y = 45;
}


void drawSliderMenu() {
  tft.fillScreen(TFT_BLACK);
    positionHomeDefault();
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
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
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
        tft.fillCircle(dotX, dotY, 4, TFT_RED);
        tft.drawCircle(dotX, dotY, 5, TFT_WHITE);
      } else {
        // Inactive page - smaller circle
        tft.fillCircle(dotX, dotY, 2, 0x4208);
        tft.drawCircle(dotX, dotY, 3, TFT_WHITE);
      }
    }
  }
}

void openFolder(int folderIndex) {
  currentFolderIndex = folderIndex;
  currentFolderPage = 0;

  const FolderConfig& folder = FOLDERS[folderIndex];
  folderButtonCount = folder.count;

  for (int i = 0; i < folderButtonCount; i++) {
    folderButtons[i].w = BUTTON_WIDTH;
    folderButtons[i].h = BUTTON_HEIGHT;

    strncpy(folderButtons[i].label, folder.buttons[i].label, 7);
    folderButtons[i].label[7] = '\0';

    strncpy(folderButtons[i].actionId, folder.buttons[i].actionId, 19);
    folderButtons[i].actionId[19] = '\0';

    folderButtons[i].color = folder.buttons[i].color;
    folderButtons[i].pressed = false;

    folderButtons[i].icon = folder.buttons[i].icon;
    folderButtons[i].iconWidth = folder.buttons[i].iconWidth;
    folderButtons[i].iconHeight = folder.buttons[i].iconHeight;
    folderButtons[i].hasIcon = folder.buttons[i].icon != NULL;

    // nesting
    folderButtons[i].type = folder.buttons[i].type;
    folderButtons[i].folderIndex = folder.buttons[i].folderIndex;
  }

  currentState = FOLDER_MENU;
  drawFolderMenu();
}


void positionFolderHomeButton() {
  backButton.x = (SCREEN_WIDTH - backButton.w) / 2;
  backButton.y = SCREEN_HEIGHT - backButton.h - 10; 
}



void drawFolderMenu() {
  tft.fillScreen(TFT_BLACK);
  drawBackgroundPattern();

  String title = FOLDERS[currentFolderIndex].name;
  title += " [" + String(currentFolderPage + 1) + "]";
  drawHeader(title);

  int startIdx = currentFolderPage * BUTTONS_PER_PAGE;
  int endIdx = min(startIdx + BUTTONS_PER_PAGE, folderButtonCount);

  // reuse layout logic
  int btnW = BUTTON_WIDTH;
  int btnH = BUTTON_HEIGHT;
  int startX = (SCREEN_WIDTH - (BUTTON_COLS * btnW + (BUTTON_COLS - 1) * BUTTON_SPACING)) / 2;

  for (int i = startIdx; i < endIdx; i++) {
    int local = i - startIdx;
    int col = local % BUTTON_COLS;
    int row = local / BUTTON_COLS;

    folderButtons[i].x = startX + col * (btnW + BUTTON_SPACING);
    folderButtons[i].y = BUTTON_START_Y + row * (btnH + BUTTON_SPACING);
 
    drawButton(folderButtons[i]);
  }
  positionFolderHomeButton();
  //  backButton.x = (SCREEN_WIDTH - backButton.w) / 2;
  // backButton.y = SCREEN_HEIGHT - backButton.h - 10;
  drawButton(backButton);

  if (folderButtonCount > BUTTONS_PER_PAGE) {
    if (currentFolderPage > 0) drawButton(prevButton);
    if (endIdx < folderButtonCount) drawButton(nextButton);
  }
}

void handleFolderMenuTouch(int x, int y) {
  int startIdx = currentFolderPage * BUTTONS_PER_PAGE;
  int endIdx = min(startIdx + BUTTONS_PER_PAGE, folderButtonCount);

  for (int i = startIdx; i < endIdx; i++) {
    Button &btn = folderButtons[i];
    if (x > btn.x && x < btn.x + btn.w &&
        y > btn.y && y < btn.y + btn.h) {

      if (btn.type == BUTTON_FOLDER) {
        openFolder(btn.folderIndex);
        return;
      }

      flashButton(btn);
      Serial.print("Action: ");
      Serial.println(btn.actionId);
      return;
    }
  }

  // BACK
  if (x > backButton.x && x < backButton.x + backButton.w &&
      y > backButton.y && y < backButton.y + backButton.h) {
    currentState = MAIN_MENU;
    drawMainMenu();
    return;
  }

  // PREV / NEXT
  if (currentFolderPage > 0 &&
      x > prevButton.x && x < prevButton.x + prevButton.w &&
      y > prevButton.y && y < prevButton.y + prevButton.h) {
    currentFolderPage--;
    drawFolderMenu();
    return;
  }

  if ((currentFolderPage + 1) * BUTTONS_PER_PAGE < folderButtonCount &&
      x > nextButton.x && x < nextButton.x + nextButton.w &&
      y > nextButton.y && y < nextButton.y + nextButton.h) {
    currentFolderPage++;
    drawFolderMenu();
    return;
  }
}


// ========== ESP32 Setup ==========
/**
 * Initialize hardware and display
 */
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  touchscreen.setRotation(1);
  
  // Initialize TFT display
  tft.init();
  tft.setRotation(1);  // Landscape orientation
  tft.fillScreen(TFT_BLACK);
  
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
  // Check if touchscreen was touched
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get touchscreen points
    TS_Point p = touchscreen.getPoint();
    
    // Calibrate touchscreen points with map function to the correct width and height
    int x = map(p.x, TS_LEFT, TS_RT, 1, SCREEN_WIDTH);
    int y = map(p.y, TS_TOP, TS_BOT, 1, SCREEN_HEIGHT);
    int z = p.z;

    // Only process if there's actual pressure
    if (z > 0) {
      // Handle main menu touches
      if (currentState == MAIN_MENU) {
        handleMainMenuTouch(x, y);
      } 
      // Handle slider menu touches
      else if (currentState == SLIDER_MENU) {
        handleSliderMenuTouch(x, y);
      }
      else if (currentState == FOLDER_MENU) {
        handleFolderMenuTouch(x, y);
      }

      
      delay(100);  // Debounce delay
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

      // ---- FOLDER FIRST ----
       if (btn.type == BUTTON_FOLDER) {
          openFolder(btn.folderIndex);
          return;
        }


        // ---- SLIDER ----
        if (strcmp(btn.actionId, "open_sliders") == 0) {
          currentState = SLIDER_MENU;
          currentSliderPage = 0;
          drawSliderMenu();
          return;
        }

        // ---- NORMAL ACTION ----
        Serial.print("Action: ");
        Serial.println(btn.actionId);
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
