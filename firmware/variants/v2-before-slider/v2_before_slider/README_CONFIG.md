# Arduino Configuration Guide

## Overview
The Cheap Deck Arduino code now uses a configuration file (`config.h`) that makes it easy to add, remove, or modify buttons and sliders without touching the main code.

## Features
✅ **Easy Configuration** - All buttons and sliders defined in one place  
✅ **Automatic Pagination** - Sliders automatically paginate (4 per page)  
✅ **Navigation Buttons** - Next/Prev buttons appear when you have more than 4 sliders  
✅ **Action IDs** - Each button and slider has a unique identifier sent to the PC  
✅ **Customizable Layout** - Adjust sizes, spacing, and colors easily  

## How to Add Buttons

Open `config.h` and edit the `BUTTON_CONFIG` array:

```cpp
const ButtonConfig BUTTON_CONFIG[] = {
  {"PLAY",   GREEN,  "youtube_play"},     // Label, Color, Action ID
  {"PAUSE",  RED,    "system_mute"},
  {"STOP",   BLUE,   "media_stop"},
  // Add your button here:
  {"NEW",    PURPLE, "my_custom_action"}
};
```

### Button Parameters:
- **Label**: Text displayed on the button (max ~6 chars for readability)
- **Color**: Button color (GREEN, RED, BLUE, CYAN, YELLOW, ORANGE, PURPLE, WHITE)
- **Action ID**: Unique identifier sent to PC when pressed

## How to Add Sliders

Open `config.h` and edit the `SLIDER_CONFIG` array:

```cpp
const SliderConfig SLIDER_CONFIG[] = {
  {"Brave",    "volume_brave"},      // Name, Action ID
  {"Discord",  "volume_discord"},
  {"System",   "volume_system"},
  // Add your slider here:
  {"NewApp",   "volume_newapp"}
};
```

### Slider Parameters:
- **Name**: Label displayed below the slider (max ~6 chars)
- **Action ID**: Unique identifier sent to PC with the value (0-100)

### Automatic Pagination
- Only **4 sliders** are shown per page
- If you have more than 4 sliders, **NEXT** and **PREV** buttons automatically appear
- Page indicator shows current page (e.g., "Page 1/3")

## Customizing Layout

In `config.h`, you can adjust these settings:

```cpp
// Button Layout
#define BUTTON_COLS 3              // Number of button columns
#define BUTTON_SPACING 20          // Space between buttons (pixels)
#define BUTTON_WIDTH 80            // Button width (pixels)
#define BUTTON_HEIGHT 60           // Button height (pixels)

// Slider Layout
#define SLIDERS_PER_PAGE 4         // Sliders per page (recommended: 4)
#define SLIDER_WIDTH 20            // Slider width (pixels)
#define SLIDER_HEIGHT 200          // Slider height (pixels)

// Timing
#define FLASH_DURATION 120         // Button flash duration (ms)
#define DEBOUNCE_DELAY 100         // Slider update delay (ms)
```

## Serial Communication Format

### Button Press
```
Action: <action_id>
```
Example: `Action: youtube_play`

### Slider Change
```
<action_id>:<value>
```
Example: `volume_brave:75`

## Example Configurations

### Gaming Setup
```cpp
const ButtonConfig BUTTON_CONFIG[] = {
  {"GAME",   GREEN,  "launch_game"},
  {"DISC",   BLUE,   "launch_discord"},
  {"MUSIC",  PURPLE, "launch_spotify"},
  {"MUTE",   RED,    "toggle_mic"},
  {"RECORD", ORANGE, "start_recording"},
  {"STREAM", CYAN,   "start_stream"}
};

const SliderConfig SLIDER_CONFIG[] = {
  {"Game",   "volume_game"},
  {"Voice",  "volume_discord"},
  {"Music",  "volume_spotify"},
  {"Master", "volume_system"}
};
```

### Streaming Setup
```cpp
const ButtonConfig BUTTON_CONFIG[] = {
  {"START",  GREEN,  "stream_start"},
  {"STOP",   RED,    "stream_stop"},
  {"MIC",    ORANGE, "toggle_mic"},
  {"CAM",    BLUE,   "toggle_camera"},
  {"SCENE1", CYAN,   "obs_scene_1"},
  {"SCENE2", CYAN,   "obs_scene_2"},
  {"SCENE3", CYAN,   "obs_scene_3"},
  {"SLIDER", PURPLE, "open_sliders"}
};

const SliderConfig SLIDER_CONFIG[] = {
  {"Mic",    "volume_mic"},
  {"Desktop","volume_desktop"},
  {"Game",   "volume_game"},
  {"Music",  "volume_music"},
  {"Alert",  "volume_alerts"},
  {"Browser","volume_browser"}
};
```

## Tips

1. **Keep Labels Short**: Button labels should be 4-6 characters for best display
2. **Unique Action IDs**: Make sure each button/slider has a unique action ID
3. **Test Incrementally**: Add one button/slider at a time and test
4. **Slider Limit**: While you can add many sliders, 8-12 is practical (2-3 pages)
5. **Color Coding**: Use consistent colors for related functions

## Troubleshooting

**Problem**: Buttons don't fit on screen  
**Solution**: Reduce `BUTTON_WIDTH`, `BUTTON_HEIGHT`, or `BUTTON_SPACING`

**Problem**: Slider labels overlap  
**Solution**: Use shorter names (max 6 characters) or increase slider spacing

**Problem**: Navigation buttons don't appear  
**Solution**: Make sure you have more than 4 sliders defined

**Problem**: Compilation error after adding items  
**Solution**: Check for missing commas between array items and ensure proper syntax

## Next Steps

After configuring your Arduino:
1. Upload the sketch to your Arduino
2. Update your PC software to handle the new action IDs
3. Test each button and slider to ensure proper communication
