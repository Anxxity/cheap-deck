#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include "config.h"
#include "image.h"

TFT_eSPI tft = TFT_eSPI();

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

const int TS_LEFT = 3700, TS_RT = 200, TS_TOP = 3800, TS_BOT = 240;

enum AppState { MAIN_MENU, SLIDER_MENU, FOLDER_MENU };
AppState currentState = MAIN_MENU;

struct Button {
  int x, y, w, h;
  char label[8];
  uint16_t color;
  char actionId[20];
  bool pressed;
  const uint16_t* icon;
  int iconWidth, iconHeight;
  bool hasIcon;
  ButtonType type;
  int folderIndex;
};

struct Slider {
  int x, y, w, h;
  int value, thumbY;
  char name[12];
  char actionId[20];
};

const int BUTTON_COUNT = sizeof(BUTTON_CONFIG) / sizeof(BUTTON_CONFIG[0]);
const int TOTAL_SLIDERS = sizeof(SLIDER_CONFIG) / sizeof(SLIDER_CONFIG[0]);
const int SLIDER_PAGES = (TOTAL_SLIDERS + SLIDERS_PER_PAGE - 1) / SLIDERS_PER_PAGE;
const int BUTTON_PAGES = (BUTTON_COUNT + BUTTONS_PER_PAGE - 1) / BUTTONS_PER_PAGE;

Button buttons[BUTTON_COUNT];
Slider sliders[TOTAL_SLIDERS];
Button folderButtons[20];

Button backButton = {10, 45, 60, 35, "HOME", RED, "", false, NULL, 0, 0, false};
Button prevButton = {10, 220, NAV_BUTTON_WIDTH, NAV_BUTTON_HEIGHT, "<", RED, "", false, NULL, 0, 0, false};
Button nextButton = {200, 220, NAV_BUTTON_WIDTH, NAV_BUTTON_HEIGHT, ">", RED, "", false, NULL, 0, 0, false};

int folderButtonCount = 0;
int currentFolderIndex = -1;
int currentFolderPage = 0;
int currentButtonPage = 0;
int currentSliderPage = 0;


inline void drawRoundRect(int x, int y, int w, int h, int r, uint16_t color) {
  tft.fillRoundRect(x, y, w, h, r, color);
}

void drawButton(Button &btn) {
  const int cornerRadius = 8;
  
  drawRoundRect(btn.x, btn.y, btn.w, btn.h, cornerRadius, btn.color);
  
  tft.drawRoundRect(btn.x, btn.y, btn.w, btn.h, cornerRadius, TFT_WHITE);
  
  if (btn.hasIcon && btn.icon != NULL) {
    int iconX = btn.x + (btn.w - btn.iconWidth) / 2;
    int iconY = btn.y + (btn.h - btn.iconHeight) / 2;
    int textAreaY = iconY + btn.iconHeight + 5;
    
    tft.pushImage(iconX, iconY, btn.iconWidth, btn.iconHeight, (uint16_t*)btn.icon);
    
    tft.setTextSize(1);
    uint16_t w = tft.textWidth(btn.label);
    int cx = btn.x + (btn.w - w) / 2;
    
    tft.setTextColor(TFT_WHITE, btn.color);
    tft.setCursor(cx, textAreaY);
    tft.print(btn.label);
  } else {
    tft.setTextSize(2);
    uint16_t w = tft.textWidth(btn.label);
    uint16_t h = tft.fontHeight();
    int cx = btn.x + (btn.w - w) / 2;
    int cy = btn.y + (btn.h - h) / 2;
    
    tft.setTextColor(TFT_WHITE, btn.color);
    tft.setCursor(cx, cy);
    tft.print(btn.label);
  }
}

void flashButton(Button &btn) {
  const int cornerRadius = 8;
  uint16_t pressedColor = tft.color565(
    ((btn.color >> 11) & 0x1F) * 0.6,
    ((btn.color >> 5) & 0x3F) * 0.6 / 4,
    (btn.color & 0x1F) * 0.6
  );
  
  drawRoundRect(btn.x, btn.y, btn.w, btn.h, cornerRadius, pressedColor);
  delay(FLASH_DURATION / 2);
  drawButton(btn);
}

void drawSlider(Slider &s) {
  const int thumbHeight = 15;
  const int thumbWidth = s.w + 4;
  const int trackRadius = 3;
  
  tft.fillRect(s.x - 5, s.y - 5, s.w + 10, s.h + 35, TFT_BLACK);
  
  tft.drawRoundRect(s.x - 1, s.y - 1, s.w + 2, s.h + 2, trackRadius, TFT_WHITE);
  tft.fillRoundRect(s.x, s.y, s.w, s.h, trackRadius, 0x2104);
  
  s.thumbY = map(s.value, 0, 100, s.y + s.h - thumbHeight, s.y);
  int fillHeight = (s.y + s.h) - s.thumbY;
  
  tft.fillRect(s.x + 2, s.thumbY, s.w - 4, fillHeight, BLUE);
  

  tft.fillRoundRect(s.x - 2, s.thumbY, thumbWidth, thumbHeight, 4, TFT_RED);
  tft.drawRoundRect(s.x - 2, s.thumbY, thumbWidth, thumbHeight, 4, TFT_WHITE);
  
  tft.setTextColor(TFT_BLACK, TFT_RED);
  tft.setTextSize(1);
  String valueStr = String(s.value);
  uint16_t w = tft.textWidth(valueStr);
  uint16_t h = tft.fontHeight();
  tft.setCursor(s.x + (s.w - w) / 2, s.thumbY + (thumbHeight - h) / 2 + 1);
  tft.print(valueStr);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  w = tft.textWidth(s.name);
  h = tft.fontHeight();
  int labelX = s.x + (s.w - w) / 2;
  int labelY = s.y + s.h + 8;
  tft.fillRoundRect(labelX - 2, labelY - 2, w + 4, h + 4, 3, 0x2104);
  tft.setCursor(labelX, labelY);
  tft.print(s.name);
}

void drawHeader(const String &title) {
  const int headerHeight = 30;
  tft.fillRect(0, 0, SCREEN_WIDTH, headerHeight, TFT_NAVY);
  
  tft.setTextSize(2);
  uint16_t w = tft.textWidth(title);
  uint16_t h = tft.fontHeight();
  int cx = (SCREEN_WIDTH - w) / 2;
  int cy = (headerHeight - h) / 2;
  
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.setCursor(cx, cy);
  tft.print(title);
  
  tft.drawFastHLine(0, headerHeight, SCREEN_WIDTH, TFT_RED);
}

void layoutButtons() {
  int startIdx = currentButtonPage * BUTTONS_PER_PAGE;
  int endIdx = min(startIdx + BUTTONS_PER_PAGE, BUTTON_COUNT);
  int startX = (SCREEN_WIDTH - (BUTTON_COLS * BUTTON_WIDTH + (BUTTON_COLS - 1) * BUTTON_SPACING)) / 2;
  
  for (int i = startIdx; i < endIdx; i++) {
    int localIdx = i - startIdx;
    int col = localIdx % BUTTON_COLS;
    int row = localIdx / BUTTON_COLS;
    buttons[i].x = startX + col * (BUTTON_WIDTH + BUTTON_SPACING);
    buttons[i].y = BUTTON_START_Y + row * (BUTTON_HEIGHT + BUTTON_SPACING);
  }
  
  prevButton.x = 10;
  prevButton.y = NAV_BUTTON_Y;
  nextButton.x = SCREEN_WIDTH - nextButton.w - 10;
  nextButton.y = NAV_BUTTON_Y;
}

void layoutSliders() {
  int startIdx = currentSliderPage * SLIDERS_PER_PAGE;
  int endIdx = min(startIdx + SLIDERS_PER_PAGE, TOTAL_SLIDERS);
  int rightEdge = SCREEN_WIDTH - 10;
  
  for (int i = 0; i < endIdx - startIdx; i++) {
    int sliderIdx = startIdx + i;
    sliders[sliderIdx].x = rightEdge - (i + 1) * (sliders[sliderIdx].w + 25);
  }
  
  prevButton.x = backButton.x;
  prevButton.y = backButton.y + backButton.h + 6;
  nextButton.x = backButton.x;
  nextButton.y = prevButton.y + prevButton.h + 6;
}

void layoutFolderButtons() {
  int startIdx = currentFolderPage * BUTTONS_PER_PAGE;
  int endIdx = min(startIdx + BUTTONS_PER_PAGE, folderButtonCount);
  int startX = (SCREEN_WIDTH - (BUTTON_COLS * BUTTON_WIDTH + (BUTTON_COLS - 1) * BUTTON_SPACING)) / 2;
  
  for (int i = startIdx; i < endIdx; i++) {
    int local = i - startIdx;
    int col = local % BUTTON_COLS;
    int row = local / BUTTON_COLS;
    folderButtons[i].x = startX + col * (BUTTON_WIDTH + BUTTON_SPACING);
    folderButtons[i].y = BUTTON_START_Y + row * (BUTTON_HEIGHT + BUTTON_SPACING);
  }
}

void drawMainMenu() {
  tft.fillScreen(0x7BEF);
  
  String headerTitle = "CHEAP DECK";
  if (BUTTON_PAGES > 1) {
    headerTitle += " [" + String(currentButtonPage + 1) + "/" + String(BUTTON_PAGES) + "]";
  }
  drawHeader(headerTitle);
  
  layoutButtons();
  
  int startIdx = currentButtonPage * BUTTONS_PER_PAGE;
  int endIdx = min(startIdx + BUTTONS_PER_PAGE, BUTTON_COUNT);
  for (int i = startIdx; i < endIdx; i++) {
    drawButton(buttons[i]);
  }
  
  if (BUTTON_PAGES > 1) {
    if (currentButtonPage > 0) drawButton(prevButton);
    if (currentButtonPage < BUTTON_PAGES - 1) drawButton(nextButton);
  }
}

void drawSliderMenu() {
  tft.fillScreen(TFT_BLACK);
  
  backButton.x = 10;
  backButton.y = 45;
  
  String headerTitle = "VOLUME MIXER";
  if (SLIDER_PAGES > 1) {
    headerTitle += " [" + String(currentSliderPage + 1) + "/" + String(SLIDER_PAGES) + "]";
  }
  drawHeader(headerTitle);
  
  layoutSliders();
  drawButton(backButton);
  
  int startIdx = currentSliderPage * SLIDERS_PER_PAGE;
  int endIdx = min(startIdx + SLIDERS_PER_PAGE, TOTAL_SLIDERS);
  for (int i = startIdx; i < endIdx; i++) {
    drawSlider(sliders[i]);
  }
  
  if (SLIDER_PAGES > 1) {
    if (currentSliderPage > 0) drawButton(prevButton);
    if (currentSliderPage < SLIDER_PAGES - 1) drawButton(nextButton);
  }
}

void drawFolderMenu() {
  tft.fillScreen(TFT_BLACK);
  
  String title = FOLDERS[currentFolderIndex].name;
  if ((folderButtonCount + BUTTONS_PER_PAGE - 1) / BUTTONS_PER_PAGE > 1) {
    title += " [" + String(currentFolderPage + 1) + "]";
  }
  drawHeader(title);
  
  layoutFolderButtons();
  
  int startIdx = currentFolderPage * BUTTONS_PER_PAGE;
  int endIdx = min(startIdx + BUTTONS_PER_PAGE, folderButtonCount);
  for (int i = startIdx; i < endIdx; i++) {
    drawButton(folderButtons[i]);
  }
  
  backButton.x = (SCREEN_WIDTH - backButton.w) / 2;
  backButton.y = SCREEN_HEIGHT - backButton.h - 10;
  drawButton(backButton);
  
  if (folderButtonCount > BUTTONS_PER_PAGE) {
    if (currentFolderPage > 0) drawButton(prevButton);
    if (endIdx < folderButtonCount) drawButton(nextButton);
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
    folderButtons[i].icon = folder.buttons[i].icon;
    folderButtons[i].iconWidth = folder.buttons[i].iconWidth;
    folderButtons[i].iconHeight = folder.buttons[i].iconHeight;
    folderButtons[i].hasIcon = folder.buttons[i].icon != NULL;
    folderButtons[i].type = folder.buttons[i].type;
    folderButtons[i].folderIndex = folder.buttons[i].folderIndex;
  }
  
  currentState = FOLDER_MENU;
  drawFolderMenu();
}

inline bool isTouched(const Button &btn, int x, int y) {
  return x > btn.x && x < btn.x + btn.w && y > btn.y && y < btn.y + btn.h;
}

void handleMainMenuTouch(int x, int y) {
  int startIdx = currentButtonPage * BUTTONS_PER_PAGE;
  int endIdx = min(startIdx + BUTTONS_PER_PAGE, BUTTON_COUNT);
  
  for (int i = startIdx; i < endIdx; i++) {
    if (isTouched(buttons[i], x, y)) {
      flashButton(buttons[i]);
      
      if (buttons[i].type == BUTTON_FOLDER) {
        openFolder(buttons[i].folderIndex);
        return;
      }
      
      if (strcmp(buttons[i].actionId, "open_sliders") == 0) {
        currentState = SLIDER_MENU;
        currentSliderPage = 0;
        drawSliderMenu();
        return;
      }
      
      Serial.print("Action: ");
      Serial.println(buttons[i].actionId);
      return;
    }
  }
  
  if (BUTTON_PAGES > 1) {
    if (currentButtonPage > 0 && isTouched(prevButton, x, y)) {
      currentButtonPage--;
      drawMainMenu();
      return;
    }
    if (currentButtonPage < BUTTON_PAGES - 1 && isTouched(nextButton, x, y)) {
      currentButtonPage++;
      drawMainMenu();
      return;
    }
  }
}

void handleSliderMenuTouch(int x, int y) {
  const int thumbHeight = 15;
  int startIdx = currentSliderPage * SLIDERS_PER_PAGE;
  int endIdx = min(startIdx + SLIDERS_PER_PAGE, TOTAL_SLIDERS);
  
  for (int i = startIdx; i < endIdx; i++) {
    Slider &s = sliders[i];
    if (x > s.x && x < s.x + s.w && y > s.y && y < s.y + s.h) {
      s.thumbY = constrain(y - thumbHeight / 2, s.y, s.y + s.h - thumbHeight);
      s.value = map(s.thumbY, s.y + s.h - thumbHeight, s.y, 0, 100);
      
      drawSlider(s);
      
      Serial.print(s.actionId);
      Serial.print(":");
      Serial.println(s.value);
      delay(DEBOUNCE_DELAY);
      return;
    }
  }
  
  if (SLIDER_PAGES > 1) {
    if (currentSliderPage > 0 && isTouched(prevButton, x, y)) {
      currentSliderPage--;
      drawSliderMenu();
      return;
    }
    if (currentSliderPage < SLIDER_PAGES - 1 && isTouched(nextButton, x, y)) {
      currentSliderPage++;
      drawSliderMenu();
      return;
    }
  }
  
  if (isTouched(backButton, x, y)) {
    currentState = MAIN_MENU;
    drawMainMenu();
  }
}

void handleFolderMenuTouch(int x, int y) {
  int startIdx = currentFolderPage * BUTTONS_PER_PAGE;
  int endIdx = min(startIdx + BUTTONS_PER_PAGE, folderButtonCount);
  
  for (int i = startIdx; i < endIdx; i++) {
    if (isTouched(folderButtons[i], x, y)) {
      if (folderButtons[i].type == BUTTON_FOLDER) {
        openFolder(folderButtons[i].folderIndex);
        return;
      }
      flashButton(folderButtons[i]);
      Serial.print("Action: ");
      Serial.println(folderButtons[i].actionId);
      return;
    }
  }
  
  if (isTouched(backButton, x, y)) {
    currentState = MAIN_MENU;
    drawMainMenu();
    return;
  }
  
  if (currentFolderPage > 0 && isTouched(prevButton, x, y)) {
    currentFolderPage--;
    drawFolderMenu();
    return;
  }
  
  if ((currentFolderPage + 1) * BUTTONS_PER_PAGE < folderButtonCount && isTouched(nextButton, x, y)) {
    currentFolderPage++;
    drawFolderMenu();
  }
}

void initButtons() {
  for (int i = 0; i < BUTTON_COUNT; i++) {
    buttons[i].w = BUTTON_WIDTH;
    buttons[i].h = BUTTON_HEIGHT;
    strncpy(buttons[i].label, BUTTON_CONFIG[i].label, 7);
    buttons[i].label[7] = '\0';
    buttons[i].color = BUTTON_CONFIG[i].color;
    strncpy(buttons[i].actionId, BUTTON_CONFIG[i].actionId, 19);
    buttons[i].actionId[19] = '\0';
    buttons[i].icon = BUTTON_CONFIG[i].icon;
    buttons[i].iconWidth = BUTTON_CONFIG[i].iconWidth;
    buttons[i].iconHeight = BUTTON_CONFIG[i].iconHeight;
    buttons[i].hasIcon = BUTTON_CONFIG[i].icon != NULL;
    buttons[i].type = BUTTON_CONFIG[i].type;
    buttons[i].folderIndex = BUTTON_CONFIG[i].folderIndex;
  }
}

void initSliders() {
  for (int i = 0; i < TOTAL_SLIDERS; i++) {
    sliders[i].w = SLIDER_WIDTH;
    sliders[i].h = SLIDER_HEIGHT;
    sliders[i].y = SLIDER_START_Y;
    sliders[i].value = 100;
    strncpy(sliders[i].name, SLIDER_CONFIG[i].name, 11);
    sliders[i].name[11] = '\0';
    strncpy(sliders[i].actionId, SLIDER_CONFIG[i].actionId, 19);
    sliders[i].actionId[19] = '\0';
  }
}

void setup() {
  Serial.begin(115200);
  
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);
  
  tft.init();
  tft.setRotation(1);
  
  tft.fillScreen(TFT_BLACK);
  
  initButtons();
  initSliders();
  
  drawMainMenu();
}

void loop() {
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    
    int x = map(p.x, TS_LEFT, TS_RT, 1, SCREEN_WIDTH);
    int y = map(p.y, TS_TOP, TS_BOT, 1, SCREEN_HEIGHT);
    
    if (p.z > 0) {
      switch(currentState) {
        case MAIN_MENU: handleMainMenuTouch(x, y); break;
        case SLIDER_MENU: handleSliderMenuTouch(x, y); break;
        case FOLDER_MENU: handleFolderMenuTouch(x, y); break;
      }
      delay(100);
    }
  }
}