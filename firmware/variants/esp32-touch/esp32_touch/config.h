#ifndef CONFIG_H
#define CONFIG_H

// External icon declarations
extern const uint16_t discord_logo[];
extern const uint16_t slider_icon[];
extern const uint16_t gpt_icon[];
extern const uint16_t deafen_icon[];
extern const uint16_t slid_logo[];
extern const uint16_t live_stream[];
extern const uint16_t short_app[];

// /* Correct 16-bit RGB565 color definitions (STANDARD) */

// Basic colors
#define BLACK   0x0000
#define WHITE   0xFFFF
#define GREY    0x7BEF

#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F

#define CYAN    0x07FF
#define YELLOW  0xFFE0
#define MAGENTA 0xF81F

// Extended colors (CORRECTED)
#define ORANGE  0xFD20
#define PURPLE  0x780F
#define BROWN   0x9A60
#define PINK    0xFE19
#define LIME    0x07FF
#define NAVY    0x000F
#define TEAL    0x0410
#define OLIVE   0x8400
#define MAROON  0x8000
#define SILVER  0xC618
#define GOLD    0xFEA0

// Additional useful colors
#define DARK_GREY   0x4208
#define LIGHT_GREY  0xBDF7
#define SKY_BLUE    0x867D
#define FOREST_GREEN 0x2444



#define CRIMSON     0xF104  // Dark red
#define RED_ORANGE  0xF0E3  // Bright red-orange



// Add or remove buttons here. Each button needs: Label, Color, Action ID, Icon, Icon Width, Icon Height
// The Action ID is sent to the PC when the button is pressed
// To add icons: 
// 1. Add image data to image.h using ImageConverter 565 Online
// 2. Declare external reference at top of config.h
// 3. Update button config: {"LABEL", COLOR, "action_id", icon_array_name, width, height}
// 4. Use NULL for icon if no icon needed


enum ButtonType {
  BUTTON_NORMAL,
  BUTTON_FOLDER
};


struct ButtonConfig {
  const char* label;
  uint16_t color;
  const char* actionId;
  const uint16_t* icon;  // Pointer to icon data (NULL if no icon)
  int iconWidth;               // Icon width
  int iconHeight;              // Icon height
  ButtonType type;      //  folder or not 
  int folderIndex;      // Which folder it opens (-1 if none)
};

/* ================= FOLDER BUTTONS ================= */
const ButtonConfig FOLDER_0_BUTTONS[] = {
  {"OBS", RED, "obs_start", NULL, 0, 0, BUTTON_NORMAL, -1},
  {"REC", RED, "obs_record", NULL, 0, 0, BUTTON_NORMAL, -1},
  {"o_STOP", RED, "obs_stop", NULL, 0, 0, BUTTON_NORMAL, -1},
    {"APPS", BLACK, "", short_app, 64, 64, BUTTON_FOLDER, 1},
};

const ButtonConfig FOLDER_1_BUTTONS[] = {
  {"VS", BLUE, "vscode", NULL, 0, 0, BUTTON_NORMAL, -1},
  {"CHROME", BLUE, "chrome", NULL, 0, 0, BUTTON_NORMAL, -1},
  {"BRAVE", BLUE, "brave", NULL, 0, 0, BUTTON_NORMAL, -1},
  {"valorant", BLUE, "valo", NULL, 0, 0, BUTTON_NORMAL, -1},
  {"Bitvise ", BLUE, "Bitvise ", NULL, 0, 0, BUTTON_NORMAL, -1},
  {"ardiuno ", BLUE, "uno ", NULL, 0, 0, BUTTON_NORMAL, -1},
  {"esp", BLUE, "esp ", NULL, 0, 0, BUTTON_NORMAL, -1},
 

  {"kero hive", BLUE, "chimera hive", NULL, 0, 0, BUTTON_NORMAL, -1},
  {"fivem", BLUE, "fivem", NULL, 0, 0, BUTTON_NORMAL, -1},
  {"telegram", BLUE, "tele", NULL, 0, 0, BUTTON_NORMAL, -1},


};

/* ================= FOLDER META ================= */
struct FolderConfig {
  const char* name;
  const ButtonConfig* buttons;
  int count;
};

const FolderConfig FOLDERS[] = {
  {"OBS TOOLS", FOLDER_0_BUTTONS, sizeof(FOLDER_0_BUTTONS)/sizeof(ButtonConfig)},
  {"APPLICATIONS", FOLDER_1_BUTTONS, sizeof(FOLDER_1_BUTTONS)/sizeof(ButtonConfig)}
};

#define TOTAL_FOLDERS (sizeof(FOLDERS)/sizeof(FolderConfig))

const ButtonConfig BUTTON_CONFIG[] = {
  {"PLAY",   RED,  "PLAY",NULL, 0, 0, BUTTON_NORMAL, -1},     // YouTube play/pause
  {"PAUSE",  RED,    "system_mute",NULL, 0, 0, BUTTON_NORMAL, -1},      // System mute toggle
  {"STOP",   RED,   "media_stop",NULL, 0, 0, BUTTON_NORMAL, -1},       // Media stop
  {"NEXT",   RED,   "NEXT",NULL, 0, 0, BUTTON_NORMAL, -1},     // YouTube skip forward
  {"BACK",   RED,  "BACK",NULL, 0, 0, BUTTON_NORMAL, -1},     // YouTube skip backward
  {"MUTE",   ORANGE, "MUTE",NULL, 0, 0, BUTTON_NORMAL, -1},       // F13 hotkey
  {"slide",  BLACK,   "open_sliders",slider_icon, 64, 64, BUTTON_NORMAL, -1},     // Open slider menu
  {"DEF",    BLACK, "DEF",deafen_icon, 64, 64, BUTTON_NORMAL, -1},       // Ctrl+Shift+Alt+D
  {"F8",    BLACK, "F8", NULL, 0, 0, BUTTON_NORMAL, -1},       // Ctrl+Shift+Alt+D
  {"YTUBE",    BLACK, "ytube", NULL, 0, 0, BUTTON_NORMAL, -1},       // Ctrl+Shift+Alt+D
  {"Brave",    BLACK, "brave", gpt_icon, 64, 64, BUTTON_NORMAL, -1},
  {"GPT",    BLACK, "gpt", gpt_icon, 64, 64, BUTTON_NORMAL, -1},
  {"DISCORD",    BLACK, "discord", discord_logo, 64, 64, BUTTON_NORMAL, -1},
  {"OBS", BLACK, "", live_stream, 64, 64, BUTTON_FOLDER, 0}, 
  {"APPS", BLACK, "", short_app, 64, 64, BUTTON_FOLDER, 1},
  
};


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


#define NAV_BUTTON_WIDTH 55        // Width of next/prev buttons
#define NAV_BUTTON_HEIGHT 40       // Height of next/prev buttons
#define NAV_BUTTON_Y 210           // Y position of nav buttons
#define HOME_BTN_W 60
#define HOME_BTN_H 35


#endif // CONFIG_H
