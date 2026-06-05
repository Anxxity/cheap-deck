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

// External icon declarations
extern const uint16_t discord_logo[];
extern const uint16_t slider_icon[];
extern const uint16_t gpt_icon[];
extern const uint16_t deafen_icon[];

// ========== Color Definitions ==========
// ✅ Correct 16-bit RGB565 color definitions (STANDARD)

#define BLACK   0x0000
#define WHITE   0xFFFF
#define GREY    0x7BEF

#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F

#define CYAN    0x07FF
#define YELLOW  0xFFE0
#define MAGENTA 0xF81F

#define ORANGE  0xFD20
#define PURPLE  0x780F
#define BROWN   0xA145
#define PINK    0xF81F
#define LIME    0x07E0
#define NAVY    0x000F
#define TEAL    0x0410
#define OLIVE   0x8400
#define MAROON  0x8000
#define SILVER  0xC618

#define GOLD    0x0595



// ========== BUTTON CONFIGURATION ==========
// Add or remove buttons here. Each button needs: Label, Color, Action ID, Icon, Icon Width, Icon Height
// The Action ID is sent to the PC when the button is pressed
// To add icons: 
// 1. Add image data to image.h using ImageConverter 565 Online
// 2. Declare external reference at top of config.h
// 3. Update button config: {"LABEL", COLOR, "action_id", icon_array_name, width, height}
// 4. Use NULL for icon if no icon needed
struct ButtonConfig {
  const char* label;
  uint16_t color;
  const char* actionId;
  const uint16_t* icon;  // Pointer to icon data (NULL if no icon)
  int iconWidth;               // Icon width
  int iconHeight;              // Icon height
};

const ButtonConfig BUTTON_CONFIG[] = {
  {"PLAY",   RED,  "PLAY",NULL, 0, 0},     // YouTube play/pause
  {"PAUSE",  RED,    "system_mute",NULL, 0, 0},      // System mute toggle
  {"STOP",   RED,   "media_stop",NULL, 0, 0},       // Media stop
  {"NEXT",   RED,   "NEXT",NULL, 0, 0},     // YouTube skip forward
  {"BACK",   RED,  "BACK",NULL, 0, 0},     // YouTube skip backward
  {"MUTE",   ORANGE, "MUTE",NULL, 0, 0},       // F13 hotkey
  {"slide",  BLACK,   "open_sliders",slider_icon, 64, 64},     // Open slider menu
  {"DEF",    BLACK, "DEF",deafen_icon, 64, 64},       // Ctrl+Shift+Alt+D
  {"F8",    BLACK, "F8", NULL, 0, 0},       // Ctrl+Shift+Alt+D
  {"YTUBE",    BLACK, "ytube", NULL, 0, 0},       // Ctrl+Shift+Alt+D
  {"Brave",    BLACK, "brave", gpt_icon, 64, 64},
  {"GPT",    BLACK, "gpt", gpt_icon, 64, 64},
   {"DISCORD",    BLACK, "discord", discord_logo, 64, 64},
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

// ========== UI CONFIGURATION (3.5" TFT) ==========
#define BUTTONS_PER_PAGE 8
#define BUTTON_COLS 4
#define BUTTON_SPACING 18
#define BUTTON_START_Y 46

#define BUTTON_WIDTH 95
#define BUTTON_HEIGHT 85

#define SLIDERS_PER_PAGE 3
#define SLIDER_WIDTH 28
#define SLIDER_HEIGHT 200
#define SLIDER_START_Y 55

#define FLASH_DURATION 120
#define DEBOUNCE_DELAY 100

// Navigation buttons
#define NAV_BUTTON_WIDTH 70
#define NAV_BUTTON_HEIGHT 45
#define NAV_BUTTON_Y 255


#endif // CONFIG_H
