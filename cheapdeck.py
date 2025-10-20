import serial
import subprocess
import pyautogui
import re
import time
from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume
from comtypes import CLSCTX_ALL

# Setup Serial (adjust COM port as needed)
mega = serial.Serial('COM3', 9600)

# Setup system audio control
devices = AudioUtilities.GetSpeakers()  # Correct method
interface = devices.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
volume = interface.QueryInterface(IAudioEndpointVolume)

# Function to set system volume
def set_system_volume(percent):
    percent = max(0, min(100, percent))  # Clamp value between 0 and 100
    volume_level = percent / 100.0
    print(f"[System Volume] Setting volume to {percent}% (scalar {volume_level})")
    volume.SetMasterVolumeLevelScalar(volume_level, None)

# Function to set app volume (all or specific app)
def set_app_volume(percent, app_name=None):
    percent = max(0, min(100, percent))
    volume_level = percent / 100.0
    sessions = AudioUtilities.GetAllSessions()
    for session in sessions:
        if session.Process and (app_name is None or app_name.lower() in session.Process.name().lower()):
            print(f"[App Volume] Setting {session.Process.name()} to {percent}%")
            session.SimpleAudioVolume.SetMasterVolume(volume_level, None)
def get_volume_by_name(name):
    sessions = AudioUtilities.GetAllSessions()
    for session in sessions:
        if session.Process and name.lower() in session.Process.name().lower():
            return int(session._volume.GetMasterVolume() * 100)
    return None

def send_volume(tag, value):
    if value is not None:
        msg = f"{tag}:{value}\n"
        mega.write(msg.encode())
        print(f"Sent:", msg.strip())

# Main loop to read serial commands
while True:
     sys_vol = get_volume_by_name("audiodg")  # System sounds (sort of a placeholder)
     brave_vol = get_volume_by_name("Brave Browser")
    #  discord_vol = get_volume_by_name("Discord")

     send_volume("SYS", sys_vol)
     send_volume("BRV", brave_vol)
    #  send_volume("DSC", discord_vol)

     time.sleep(1)
     try:
        cmd = mega.readline().decode().strip()
        print(f"[Serial] Received: {cmd}")

        if cmd == "Action: PLAY":
            pyautogui.press('K')
        elif cmd == "Action: PAUSE":
            subprocess.Popen(["nircmd.exe", "mutesysvolume", "2"])  # Toggle mute
        elif cmd == "Action: NEXT":
          pyautogui.press('L')
        elif cmd == "Action: BACK":
           pyautogui.press('J')
        
        elif cmd == "Action: mute1":
            subprocess.Popen(["nircmd.exe", "mutesysvolume", "2"])  # Toggle mute

        elif cmd == "Action: MUTE":
            pyautogui.press('f13')

        elif cmd == "Action: DEF":
            pyautogui.hotkey('ctrl', 'shift', 'alt', 'd')

        elif cmd == "Action: SWITCHAUDIO":
            pyautogui.hotkey('ctrl', 'alt', 'f11')

        # Volume sliders
        elif "slider 3" in cmd.lower() and "value:" in cmd.lower():
            match = re.search(r'value[:\s]+(\d+)', cmd, re.IGNORECASE)
            if match:
                value = int(match.group(1))
                set_system_volume(value)
                print(f"[Volume] System volume set to {value}%")

        elif "slider 2" in cmd.lower() and "value:" in cmd.lower():
            match = re.search(r'value[:\s]+(\d+)', cmd, re.IGNORECASE)
            if match:
                value = int(match.group(1))
                set_app_volume(value, app_name="Discord.exe")
                print(f"[Volume] Discord volume set to {value}%")

        elif "slider 1" in cmd.lower() and "value:" in cmd.lower():
            match = re.search(r'value[:\s]+(\d+)', cmd, re.IGNORECASE)
            if match:
                value = int(match.group(1))
                set_app_volume(value, app_name="brave.exe")
                print(f"[Volume] Brave volume set to {value}%")

     except Exception as e:
        print(f"[Error] {e}")
