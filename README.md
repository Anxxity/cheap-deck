# Cheap Deck (code will be updated soon..)

Cheap Deck is a low-cost alternative to the Stream Deck, built using an Arduino Uno and a TFT touchscreen, with a total cost of around ₹1000.
It provides a simple touchscreen interface for controlling your PC, automating tasks, and launching applications — all without expensive hardware.

## Features

> Touchscreen interface with multiple customizable buttons

> Serial communication between Arduino and PC

> Control PC functions through Python scripts

> Launch applications, send keyboard shortcuts, or control OBS

> Compact, lightweight, and affordable

## How It Works

Cheap Deck uses an Arduino Uno connected to a TFT display (MCUFRIEND_kbv).
When a button on the touchscreen is pressed, the Arduino sends a serial command to the computer.
A Python script running on the computer reads these serial messages and executes corresponding actions, such as opening programs or simulating keypresses.

# Components
| Component | Model | Approx. Cost (₹) | link |
|:------------|:-------------:|-------------:|-------------:|
| Arduino Uno | Generic | 324 | https://www.amazon.in/s?k=ardiuno+uno |
| TFT LCD Display	2.4 | touch | 476 |https://www.amazon.in/s?k=2.4+inch+touch+screen+tft+display+shield+for+arduino+uno |
| USB Cable | Type-B | 122 | https://www.amazon.in/s?k=USB+Cable+Type-B |
| Jumper Wires |  | 189 | https://www.amazon.in/s?k=jumber+wire |
| Total |  | 1111  | |



# Software Requirements

> Arduino IDE

> Python 3

## Setup

1. Upload Arduino Code

2. Open the provided Arduino sketch in the Arduino IDE.

3. Ensure the display driver (e.g., MCUFRIEND_kbv) matches your hardware.

4. Upload the code to the Arduino Uno.

5. Run the Python Script on PC

6. Connect the Arduino to your PC via USB.

7. Run the cheapdeck.py script.

8. It will listen for serial messages from the Arduino and perform actions.

## Customize

> Modify the Arduino layout for different button designs.

> Edit the Python script to change what each button does.

This project is licensed under the MIT License.
You are free to use, modify, and distribute it with attribution.
