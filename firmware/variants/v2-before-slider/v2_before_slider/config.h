/*
 * Configuration File for Cheap Deck
 * 
 * This file contains all button and slider configurations.
 * Edit this file to customize your stream deck without modifying the main code.
 * 
 * HOW TO ADD BUTTONS:
 * - Add a new line in the BUTTON_CONFIG array
 * - Format: {"LABEL", COLOR, "ACTION_ID"}
 * - Available colors: GREEN, RED, BLUE, CYAN, YELLOW, ORANGE, PURPLE, WHITE
 * 
 * HOW TO ADD SLIDERS:
 * - Add a new line in the SLIDER_CONFIG array
 * - Format: {"SLIDER_NAME", "ACTION_ID"}
 * - Sliders will automatically paginate (4 per page)
 * - Navigation buttons appear automatically when needed
 */

#ifndef CONFIG_H
#define CONFIG_H

// ========== Color Definitions ==========
#define WHITE   0x0000
#define GREY    0x8410 
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define CYAN    0x07FF
#define BLACK   0xFFFF
#define YELLOW  0xFFE0
#define ORANGE  0xFD20
#define PURPLE  0x780F

// ========== BUTTON CONFIGURATION ==========
// Add or remove buttons here. Each button needs: Label, Color, Action ID
// The Action ID is sent to the PC when the button is pressed
struct ButtonConfig {
  const char* label;
  uint16_t color;
  const char* actionId;
};

const ButtonConfig BUTTON_CONFIG[] = {
  {"PLAY",   CYAN,  "PLAY"},     // YouTube play/pause
  {"PAUSE",  CYAN,    "system_mute"},      // System mute toggle
  {"STOP",   CYAN,   "media_stop"},       // Media stop
  {"NEXT",   CYAN,   "NEXT"},     // YouTube skip forward
  {"BACK",   CYAN,  "BACK"},     // YouTube skip backward
  {"MUTE",   ORANGE, "MUTE"},       // F13 hotkey
  {"slide",  BLUE,   "open_sliders"},     // Open slider menu
  {"DEF",    ORANGE, "DEF"},       // Ctrl+Shift+Alt+D
  {"F8",    BLACK, "F8"},       // Ctrl+Shift+Alt+D
  {"YTUBE",    BLACK, "ytube"},       // Ctrl+Shift+Alt+D
  {"Brave",    BLACK, "brave"},
  {"GPT",    BLACK, "gpt"},
};

// ========== SLIDER CONFIGURATION ==========
// Add or remove sliders here. Each slider needs: Name, Action ID
// The Action ID is sent to the PC with the slider value (0-100)
// Sliders will automatically paginate - only 4 shown per page
struct SliderConfig {
  const char* name;
  const char* actionId;
};

const SliderConfig SLIDER_CONFIG[] = {
  {"Brave",    "slider_1"},      // Brave Browser volume
  {"Discord",  "slider_2"},    // Discord volume
  {"System",   "slider_3"},     // System master volume
  {"Headphones",  "slider_4"}    // Spotify volume

};

// ========== UI CONFIGURATION ==========
// Customize the appearance and layout
#define BUTTONS_PER_PAGE 8         // Number of buttons shown per page
#define BUTTON_COLS 4              // Number of button columns
#define BUTTON_SPACING 14          // Space between buttons (pixels)
#define BUTTON_START_Y 36          // Top margin for buttons (pixels)
#define BUTTON_WIDTH 70            // Button width (pixels)
#define BUTTON_HEIGHT 70           // Button height (pixels)

#define SLIDERS_PER_PAGE 3         // Number of sliders shown per page
#define SLIDER_WIDTH 20            // Slider width (pixels)
#define SLIDER_HEIGHT 150          // Slider height (pixels)
#define SLIDER_START_Y 45          // Top margin for sliders (pixels)

#define FLASH_DURATION 120         // Button flash duration (ms)
#define DEBOUNCE_DELAY 100         // Slider update delay (ms)

// Navigation button configuration
#define NAV_BUTTON_WIDTH 55        // Width of next/prev buttons
#define NAV_BUTTON_HEIGHT 40       // Height of next/prev buttons
#define NAV_BUTTON_Y 210           // Y position of nav buttons

#endif // CONFIG_H
