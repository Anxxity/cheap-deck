import serial
import subprocess
import pyautogui
import re
import time
import logging
import sys
import json
import os
from typing import Optional, Dict, Any
from pathlib import Path
from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume
from comtypes import CLSCTX_ALL

# Default Configuration (used if config.json not found)
DEFAULT_CONFIG = {
    'serial': {
        'port': 'COM7',
        'baud_rate': 9600,
        'timeout': 1.0
    },
    'volume': {
        'update_interval': 1.0,
        'applications': {
            'slider_1': {'name': 'Brave Browser', 'process': 'brave.exe'},
            'slider_2': {'name': 'Discord', 'process': 'Discord.exe'},
            'slider_3': {'name': 'System', 'process': None}
        }
    },
    'buttons': {
        'PLAY': {'action': 'key', 'value': 'k'},
        'PAUSE': {'action': 'mute_toggle'},
        'NEXT': {'action': 'key', 'value': 'l'},
        'BACK': {'action': 'key', 'value': 'j'},
        'MUTE': {'action': 'key', 'value': 'f13'},
        'DEF': {'action': 'hotkey', 'value': ['ctrl', 'shift', 'alt', 'd']},
        'STOP': {'action': 'none'}
    },
    'paths': {
        'nircmd': 'nircmd.exe'
    },
    'logging': {
        'level': 'INFO',
        'file': 'cheapdeck.log'
    }
}

def load_config(config_path: str = 'config.json') -> Dict[str, Any]:

    config_file = Path(__file__).parent / config_path
    
    if config_file.exists():
        try:
            with open(config_file, 'r') as f:
                config = json.load(f)
            print(f"✓ Loaded configuration from {config_file}")
            return config
        except json.JSONDecodeError as e:
            print(f"⚠ Error parsing config.json: {e}")
            print("Using default configuration")
            return DEFAULT_CONFIG
        except Exception as e:
            print(f"⚠ Error loading config.json: {e}")
            print("Using default configuration")
            return DEFAULT_CONFIG
    else:
        print(f"⚠ Config file not found at {config_file}")
        print("Using default configuration")
        return DEFAULT_CONFIG

# Load configuration
CONFIG = load_config()

# Setup logging
log_level = getattr(logging, CONFIG.get('logging', {}).get('level', 'INFO'))
log_file = CONFIG.get('logging', {}).get('file', 'cheapdeck.log')

logging.basicConfig(
    level=log_level,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(sys.stdout),
        logging.FileHandler(log_file)
    ]
)
logger = logging.getLogger(__name__)


class CheapDeck:
    def __init__(self, com_port: str, baud_rate: int = 9600, timeout: float = 1.0):
        self.com_port = com_port
        self.baud_rate = baud_rate
        self.timeout = timeout
        self.serial_connection = None
        self.volume_interface = None
        
    def connect(self) -> bool:
        try:
            # Setup Serial connection
            logger.info(f"Connecting to {self.com_port} at {self.baud_rate} baud...")
            self.serial_connection = serial.Serial(
                self.com_port, 
                self.baud_rate, 
                timeout=self.timeout
            )
            logger.info("Serial connection established")
            
            # Setup system audio control
            logger.info("Initializing audio control...")
            devices = AudioUtilities.GetSpeakers()
            interface = devices.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
            self.volume_interface = interface.QueryInterface(IAudioEndpointVolume)
            logger.info("Audio control initialized")
            
            return True
            
        except serial.SerialException as e:
            logger.error(f"Failed to connect to serial port: {e}")
            return False
        except Exception as e:
            logger.error(f"Failed to initialize audio control: {e}")
            return False

    def set_system_volume(self, percent: int) -> None:
        try:
            percent = max(0, min(100, percent))  # Clamp value between 0 and 100
            volume_level = percent / 100.0
            logger.info(f"Setting system volume to {percent}%")
            self.volume_interface.SetMasterVolumeLevelScalar(volume_level, None)
        except Exception as e:
            logger.error(f"Failed to set system volume: {e}")
    
    def set_app_volume(self, percent: int, app_name: Optional[str] = None) -> None:
        try:
            percent = max(0, min(100, percent))
            volume_level = percent / 100.0
            sessions = AudioUtilities.GetAllSessions()
            
            for session in sessions:
                if session.Process and (app_name is None or app_name.lower() in session.Process.name().lower()):
                    logger.info(f"Setting {session.Process.name()} volume to {percent}%")
                    session.SimpleAudioVolume.SetMasterVolume(volume_level, None)
        except Exception as e:
            logger.error(f"Failed to set app volume: {e}")
    
    def get_volume_by_name(self, name: str) -> Optional[int]:
        try:
            sessions = AudioUtilities.GetAllSessions()
            for session in sessions:
                if session.Process and name.lower() in session.Process.name().lower():
                    return int(session._volume.GetMasterVolume() * 100)
        except Exception as e:
            logger.error(f"Failed to get volume for {name}: {e}")
        return None
    
    def send_volume(self, tag: str, value: Optional[int]) -> None:
        if value is not None and self.serial_connection:
            try:
                msg = f"{tag}:{value}\n"
                self.serial_connection.write(msg.encode())
                logger.debug(f"Sent: {msg.strip()}")
            except Exception as e:
                logger.error(f"Failed to send volume data: {e}")

    def handle_command(self, cmd: str) -> None:
        if not cmd:
            return
            
        logger.info(f"Received command: {cmd}")
        
        try:
            # Extract button name from command
            if cmd.startswith("Action: "):
                button_name = cmd.replace("Action: ", "")
                self._handle_button_action(button_name)
            
            # Volume sliders
            elif "slider" in cmd.lower() and "value:" in cmd.lower():
                self._handle_slider_command(cmd)
                
        except Exception as e:
            logger.error(f"Error handling command '{cmd}': {e}")
    
    def _handle_button_action(self, button_name: str) -> None:
        buttons_config = CONFIG.get('buttons', {})
        button_config = buttons_config.get(button_name, {})
        
        if not button_config:
            logger.warning(f"No configuration found for button: {button_name}")
            return
        
        action = button_config.get('action')
        
        if action == 'key':
            key = button_config.get('value', '')
            if key:
                pyautogui.press(key)
                logger.info(f"Pressed key: {key}")
        
        elif action == 'hotkey':
            keys = button_config.get('value', [])
            if keys:
                pyautogui.hotkey(*keys)
                logger.info(f"Pressed hotkey: {'+'.join(keys)}")
        
        elif action == 'mute_toggle':
            self._toggle_mute()
        
        elif action == 'none':
            logger.info(f"{button_name} button pressed (no action configured)")
        
        else:
            logger.warning(f"Unknown action type: {action}")
    
    def _toggle_mute(self) -> None:
        try:
            nircmd_path = CONFIG.get('paths', {}).get('nircmd', 'nircmd.exe')
            subprocess.Popen([nircmd_path, "mutesysvolume", "2"])
            logger.info("Toggled system mute")
        except FileNotFoundError:
            nircmd_path = CONFIG.get('paths', {}).get('nircmd', 'nircmd.exe')
            logger.error(f"nircmd.exe not found at {nircmd_path}")
        except Exception as e:
            logger.error(f"Failed to toggle mute: {e}")
    
    def _handle_slider_command(self, cmd: str) -> None:
        match = re.search(r'slider\s+(\d+).*value[:\s]+(\d+)', cmd, re.IGNORECASE)
        if not match:
            logger.warning(f"Invalid slider command format: {cmd}")
            return
            
        slider_num = int(match.group(1))
        value = int(match.group(2))
        
        # Get slider configuration from config
        volume_config = CONFIG.get('volume', {}).get('applications', {})
        slider_key = f'slider_{slider_num}'
        
        if slider_key in volume_config:
            slider_config = volume_config[slider_key]
            app_name = slider_config.get('name', 'Unknown')
            app_process = slider_config.get('process')
            
            if app_process:
                self.set_app_volume(value, app_name=app_process)
                logger.info(f"{app_name} volume set to {value}%")
            else:
                self.set_system_volume(value)
                logger.info(f"System volume set to {value}%")
        else:
            logger.warning(f"No configuration found for slider {slider_num}")
    
    def update_volume_feedback(self) -> None:
        volume_config = CONFIG.get('volume', {}).get('applications', {})
        
        # Send volume for each configured slider
        for slider_key, slider_config in volume_config.items():
            app_name = slider_config.get('name', '')
            if app_name and app_name != 'System':
                vol = self.get_volume_by_name(app_name)
                tag = slider_key.replace('slider_', 'SL')
                self.send_volume(tag, vol)
    
    def run(self) -> None:
        logger.info("Starting main loop...")
        last_volume_update = time.time()
        
        try:
            while True:
                # Update volume feedback periodically
                current_time = time.time()
                update_interval = CONFIG.get('volume', {}).get('update_interval', 1.0)
                if current_time - last_volume_update >= update_interval:
                    self.update_volume_feedback()
                    last_volume_update = current_time
                
                # Read and process serial commands
                if self.serial_connection and self.serial_connection.in_waiting > 0:
                    try:
                        cmd = self.serial_connection.readline().decode('utf-8', errors='ignore').strip()
                        if cmd:
                            self.handle_command(cmd)
                    except UnicodeDecodeError as e:
                        logger.error(f"Failed to decode serial data: {e}")
                    except Exception as e:
                        logger.error(f"Error reading serial: {e}")
                
                time.sleep(0.01)  # Small delay to prevent CPU spinning
                
        except KeyboardInterrupt:
            logger.info("Shutting down...")
        finally:
            self.cleanup()
    
    def cleanup(self) -> None:
        if self.serial_connection and self.serial_connection.is_open:
            logger.info("Closing serial connection...")
            self.serial_connection.close()
        logger.info("Cleanup complete")


def main():
    logger.info("=" * 50)
    logger.info("Cheap Deck Controller Starting")
    logger.info("=" * 50)
    
    # Create and initialize controller
    serial_config = CONFIG.get('serial', {})
    controller = CheapDeck(
        com_port=serial_config.get('port', 'COM7'),
        baud_rate=serial_config.get('baud_rate', 9600),
        timeout=serial_config.get('timeout', 1.0)
    )
    
    # Connect to Arduino
    if not controller.connect():
        logger.error("Failed to initialize. Exiting.")
        sys.exit(1)
    
    # Run main loop
    controller.run()


if __name__ == "__main__":
    main()
