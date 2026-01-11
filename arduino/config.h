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

// icon declarations

//16-bit RGB565 color definitions

#define BLACK   0xFFFF  
#define WHITE   0x0000  
#define GREY    0x7BEF
#define RED     0x07FF  
#define GREEN   0xF81F 
#define BLUE    0xFFE0  
#define CYAN    0xF800  
#define YELLOW  0x001F  
#define ORANGE  0x02DF 
#define PURPLE  0x841F 
#define MAGENTA 0x07E0  
#define BROWN   0x5AEB
#define PINK    0x07B5
#define LIME    0xF81F
#define NAVY    0xE800
#define TEAL    0xFC00
#define OLIVE   0x841F
#define MAROON  0x07E0
#define SILVER  0xC618
#define GOLD    0x0595

// Add or remove buttons here. Each button needs: Label, Color, Action ID, Icon, Icon Width, Icon Height
// The Action ID is sent to the PC when the button is pressed
// To add icons: 
// 1. Add image data to image.h using ImageConverter 565 Online
// 2. Declare external reference at top of config.h
// 3. Update button config: {"LABEL", COLOR, "action_id", icon_array_name, width, height}
// 4. Use NULL for icon if no icon needed


struct ButtonConfig {
  const char* label;            // label
  uint16_t color;             // color of button 
  const char* actionId;      // action id 
  const uint16_t* icon;  // Pointer to icon data (NULL if no icon)
  int iconWidth;               // Icon width
  int iconHeight;              // Icon height
};

// buttons in the menu 
//
const ButtonConfig BUTTON_CONFIG[] = {
  {"PLAY",   RED,  "PLAY",NULL, 0, 0},     // YouTube play/pause
  {"PAUSE",  RED,    "system_mute",NULL, 0, 0},      // System mute toggle
  {"STOP",   RED,   "media_stop",NULL, 0, 0},       // Media stop
  {"NEXT",   RED,   "NEXT",NULL, 0, 0},     // YouTube skip forward
  {"BACK",   RED,  "BACK",NULL, 0, 0},     // YouTube skip backward
  {"MUTE",   ORANGE, "MUTE",NULL, 0, 0},       // F13 hotkey
  // this open the slider menu it is hard coded
  {"slide",  BLACK,   "open_sliders",slider_icon, 64, 64},     // Open slider menu
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  {"DEF",    BLACK, "DEF",deafen_icon, 64, 64},       // Ctrl+Shift+Alt+D
  {"F8",    BLACK, "F8", NULL, 0, 0},       // Ctrl+Shift+Alt+D
  {"YTUBE",    BLACK, "ytube", NULL, 0, 0},       // Ctrl+Shift+Alt+D
  {"Brave",    BLACK, "brave", gpt_icon, 64, 64},
  {"GPT",    BLACK, "gpt", gpt_icon, 64, 64},
  {"DISCORD",    BLACK, "discord", discord_logo, 64, 64},
};


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
