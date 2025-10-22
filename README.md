# 🎮 Cheap Deck

A low-cost alternative to the Elgato Stream Deck, built using an Arduino Uno and a TFT touchscreen for around **₹1000** (~$12 USD). Control your PC with a customizable touchscreen interface — perfect for streamers, content creators, and productivity enthusiasts on a budget.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## ✨ Features

- **📱 Touchscreen Interface** - Multiple customizable buttons with visual feedback
- **🎚️ Volume Control** - Individual app volume sliders (System, Browser, Discord, etc.)
- **⌨️ Keyboard Shortcuts** - Send custom hotkeys and key combinations
- **🎬 Media Controls** - Play/pause, next/previous track controls
- **🔄 Real-time Feedback** - Volume levels displayed on the Arduino screen
- **💰 Budget-Friendly** - Total cost under ₹1200 with readily available components
- **🔧 Fully Customizable** - Easy to modify buttons, actions, and layouts

## 🎯 How It Works

Cheap Deck uses a two-part system:

1. **Arduino + TFT Display**: Provides a touchscreen interface with buttons and sliders. When touched, it sends serial commands to the PC.
2. **Python Script**: Runs on your PC, listens for serial commands, and executes corresponding actions (keyboard shortcuts, volume control, app launching, etc.).

The Arduino and PC communicate via USB serial connection, making it plug-and-play with no additional hardware required.

## 🛠️ Components

| Component | Model | Approx. Cost (₹) | Link |
|:----------|:------|:-----------------|:-----|
| Arduino Uno | Generic | ₹324 | [Amazon India](https://www.amazon.in/s?k=arduino+uno) |
| TFT LCD Display | 2.4" Touch Shield | ₹476 | [Amazon India](https://www.amazon.in/s?k=2.4+inch+touch+screen+tft+display+shield+for+arduino+uno) |
| USB Cable | Type-B | ₹122 | [Amazon India](https://www.amazon.in/s?k=USB+Cable+Type-B) |
| Jumper Wires | Optional | ₹189 | [Amazon India](https://www.amazon.in/s?k=jumper+wire) |
| **Total** | | **₹1111** | |

> **Note**: Jumper wires are optional if your TFT display is a shield that plugs directly into the Arduino.

## 📋 Prerequisites

### Hardware
- Arduino Uno (or compatible board)
- 2.4" TFT LCD Touch Display (MCUFRIEND_kbv compatible)
- USB Type-B cable
- Windows PC (for the Python script)

### Software
- **Arduino IDE** - [Download here](https://www.arduino.cc/en/software)
- **Python 3.7+** - [Download here](https://www.python.org/downloads/)
- **NirCmd** (optional, for advanced system control) - [Download here](https://www.nirsoft.net/utils/nircmd.html)

## 🚀 Installation

### 1. Arduino Setup

1. **Install Required Libraries** in Arduino IDE:
   - `MCUFRIEND_kbv` (for TFT display)
   - `Adafruit_GFX` (for graphics)
   - `TouchScreen` (for touch input)
   
   Go to **Sketch → Include Library → Manage Libraries** and search for each library.

2. **Upload the Arduino Code**:
   ```bash
   # Open arduino/arduino.ino in Arduino IDE
   # Select your board: Tools → Board → Arduino Uno
   # Select your COM port: Tools → Port → COMx
   # Click Upload
   ```

3. **Verify the Display**: After uploading, you should see the main menu with buttons on the TFT screen.

### 2. Python Setup

1. **Clone or Download** this repository:
   ```bash
   git clone https://github.com/Anxxity/cheap-deck.git
   cd cheap-deck/pc
   ```

2. **Install Python Dependencies**:
   ```bash
   pip install -r req.txt
   ```

3. **Configure COM Port**:
   Open `cheapdeck.py` and update the COM port to match your Arduino:
   ```python
   mega = serial.Serial('COM3', 9600)  # Change COM3 to your port
   ```
   
   > **Tip**: Check your COM port in Arduino IDE under **Tools → Port**

4. **Run the Script**:
   ```bash
   python cheapdeck.py
   ```

## 🎮 Usage

### Main Menu
The main screen displays 8 customizable buttons:
- **PLAY** - Sends 'K' key (YouTube play/pause)
- **PAUSE** - Toggles system mute
- **NEXT** - Sends 'L' key (YouTube skip forward)
- **BACK** - Sends 'J' key (YouTube skip backward)
- **MUTE** - Sends F13 key (custom hotkey)
- **DEF** - Sends Ctrl+Shift+Alt+D (custom hotkey)
- **SLIDER** - Opens the volume control menu

### Slider Menu
Access volume controls for different applications:
- **Slider 1** - Brave Browser volume
- **Slider 2** - Discord volume
- **Slider 3** - System volume
- **HOME Button** - Return to main menu

Volume levels are displayed in real-time on the Arduino screen.

## ⚙️ Customization

### Modifying Buttons (Arduino)

Edit `arduino/arduino.ino` to change button labels, colors, and layout:

```cpp
Button buttons[] = {
  {0, 0, 80, 60, "PLAY",   GREEN, false},
  {0, 0, 80, 60, "CUSTOM", BLUE,  false},  // Add your button
  // ... more buttons
};
```

### Modifying Actions (Python)

Edit `pc/cheapdeck.py` to change what each button does:

```python
if cmd == "Action: PLAY":
    pyautogui.press('K')  # Change to any key or hotkey
elif cmd == "Action: CUSTOM":
    subprocess.Popen(["notepad.exe"])  # Launch an application
```

### Adding New Applications to Volume Control

```python
# In the main loop
app_vol = get_volume_by_name("YourApp")  # e.g., "Spotify"
send_volume("APP", app_vol)

# In the command handler
elif "slider 4" in cmd.lower():
    set_app_volume(value, app_name="YourApp.exe")
```

``` 
{
  "_comment": "Cheap Deck Configuration Example - Copy this to config.json and customize",
  
  "serial": {
    "port": "COM7",
    "baud_rate": 9600,
    "timeout": 1.0
  },
  
  "volume": {
    "update_interval": 1.0,
    "applications": {
      "slider_1": {
        "name": "Brave Browser",
        "process": "brave.exe"
      },
      "slider_2": {
        "name": "Discord",
        "process": "Discord.exe"
      },
      "slider_3": {
        "name": "System",
        "process": null
      },
      "slider_4": {
        "name": "Spotify",
        "process": "Spotify.exe",
        "_comment": "Example: Add more apps as needed"
      }
    }
  },
  
  "buttons": {
    "PLAY": {
      "action": "key",
      "value": "k",
      "description": "YouTube play/pause"
    },
    "PAUSE": {
      "action": "mute_toggle",
      "description": "Toggle system mute"
    },
    "NEXT": {
      "action": "key",
      "value": "l",
      "description": "YouTube skip forward"
    },
    "BACK": {
      "action": "key",
      "value": "j",
      "description": "YouTube skip backward"
    },
    "MUTE": {
      "action": "key",
      "value": "f13",
      "description": "Custom F13 hotkey"
    },
    "DEF": {
      "action": "hotkey",
      "value": ["ctrl", "shift", "alt", "d"],
      "description": "Custom multi-key hotkey"
    },
    "STOP": {
      "action": "none",
      "description": "Not configured"
    },
    "SLIDER": {
      "_comment": "This button opens the slider menu - handled internally",
      "action": "internal",
      "description": "Open volume slider menu"
    }
  },
  
  "paths": {
    "nircmd": "nircmd.exe",
    "_comment": "Path to nircmd.exe for system control"
  },
  
  "logging": {
    "level": "INFO",
    "file": "cheapdeck.log",
    "_comment": "Logging levels: DEBUG, INFO, WARNING, ERROR, CRITICAL"
  }
}

```

## 🐛 Troubleshooting

| Issue | Solution |
|:------|:---------|
| **Arduino not detected** | Check USB cable, install CH340 drivers if needed |
| **Display not working** | Verify TFT shield is properly seated, check library compatibility |
| **Python script errors** | Ensure all dependencies are installed: `pip install -r req.txt` |
| **Wrong COM port** | Check Arduino IDE → Tools → Port, update `cheapdeck.py` |
| **Volume control not working** | Run Python script as Administrator |
| **Buttons not responding** | Calibrate touch screen values in `arduino.ino` (TS_LEFT, TS_RT, etc.) |

## 📸 Preview

https://github.com/user-attachments/assets/38e9ef2a-f3b4-417e-8eb3-a289a02de06f

https://github.com/user-attachments/assets/947858c0-65a7-406a-9523-1bcb4dbef635


## 🗺️ Roadmap

- [ ] Add more button layouts and themes
- [ ] Support for macOS and Linux
- [ ] Web-based configuration interface
- [ ] Profile switching for different applications
- [ ] RGB LED feedback
- [ ] Wireless connectivity (ESP32 version)
- [ ] a web configaration for creating button and layout (ESP32 version)

## 🤝 Contributing

Contributions are welcome! Feel free to:
- Report bugs
- Suggest new features
- Submit pull requests
- Improve documentation

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

You are free to use, modify, and distribute this project with attribution.

## 🙏 Acknowledgments

- Inspired by the Elgato Stream Deck
- Built with [MCUFRIEND_kbv](https://github.com/prenticedavid/MCUFRIEND_kbv) library
- Uses [pycaw](https://github.com/AndreMiras/pycaw) for Windows audio control

## 📧 Contact

Created by [@Anxxity](https://github.com/Anxxity)

If you found this project helpful, consider giving it a ⭐!
