"""Cheap Deck - Arduino-based Stream Deck Controller

This script handles serial communication with the Arduino and executes
corresponding actions like keyboard shortcuts and volume control.
"""

import serial
import subprocess
import pyautogui
import re
import time
import logging
import sys
import json
import os
import ast
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
        'PLAY': {'action': 'youtube_play', 'value': 'k'},
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

# def load_config(config_path: str = 'config.json') -> Dict[str, Any]:
#     """Load configuration from JSON file with fallback to defaults.
    
#     Args:
#         config_path: Path to config file
        
#     Returns:
#         Configuration dictionary
#     """
#     config_file = Path(__file__).parent / config_path
#     if config_file.exists():
#         try:
#             with open(config_file, 'r') as f:
#                 config = json.load(f)
#             print(f"✓ Loaded configuration from {config_file}")
#             return config
#         except json.JSONDecodeError as e:
#             print(f"⚠ Error parsing config.json: {e}")
#             print("Using default configuration")
#             return DEFAULT_CONFIG
#         except Exception as e:
#             print(f"⚠ Error loading config.json: {e}")
#             print("Using default configuration")
#             return DEFAULT_CONFIG
#     else:
#         print(f"⚠ Config file not found at {config_file}")
#         print("Using default configuration")
#         return DEFAULT_CONFIG
def load_config(config_path: str = 'config.json') -> Dict[str, Any]:
    """Load configuration from JSON file with fallback to defaults."""

    # Detect if running as a compiled executable
    if getattr(sys, 'frozen', False):
        base_path = Path(sys.executable).parent  # Folder containing the .exe
    else:
        base_path = Path(__file__).parent

    config_file = base_path / config_path

    if config_file.exists():
        try:
            with open(config_file, 'r') as f:
                config = json.load(f)
            print(f"✓ Loaded configuration from {config_file}")
            return config
        except json.JSONDecodeError as e:
            print(f"⚠ Error parsing config.json: {e}")
        except Exception as e:
            print(f"⚠ Error loading config.json: {e}")

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
    """Main controller class for Cheap Deck."""
    
    def __init__(self, com_port: str, baud_rate: int = 9600, timeout: float = 1.0):
        """Initialize the Cheap Deck controller.
        
        Args:
            com_port: Serial port name (e.g., 'COM7')
            baud_rate: Serial communication baud rate
            timeout: Serial read timeout in seconds
        """
        self.com_port = com_port
        self.baud_rate = baud_rate
        self.timeout = timeout
        self.serial_connection = None
        self.volume_interface = None
        # Debug mode (prints extra matching info). Can be enabled in config.json
        self.debug = CONFIG.get('debug', False)
        
    def connect(self) -> bool:
        """Establish serial connection and initialize audio control.
        
        Returns:
            True if connection successful, False otherwise
        """
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
            com_portIN = input("INPUT THE COM PORT FAILED FROM CONFIG")
            self.serial_connection = serial.Serial(
                com_portIN, 
                self.baud_rate, 
                timeout=self.timeout
            )
            
            return True
        except Exception as e:
            logger.error(f"Failed to initialize audio control: {e}")
            return False

    def set_system_volume(self, percent: int) -> None:
        """Set system master volume.
        
        Args:
            percent: Volume level (0-100)
        """
        try:
            percent = max(0, min(100, percent))  # Clamp value between 0 and 100
            volume_level = percent / 100.0
            logger.info(f"Setting system volume to {percent}%")
            self.volume_interface.SetMasterVolumeLevelScalar(volume_level, None)
        except Exception as e:
            logger.error(f"Failed to set system volume: {e}")
    
    def set_app_volume(self, percent: int, app_name: Optional[str] = None) -> None:
        """Set volume for a specific application or all applications.
        
        Args:
            percent: Volume level (0-100)
            app_name: Name of the application (optional)
        """
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
        """Get current volume level for an application.
        
        Args:
            name: Application name to search for
            
        Returns:
            Volume level (0-100) or None if not found
        """
        try:
            sessions = AudioUtilities.GetAllSessions()
            for session in sessions:
                if session.Process and name.lower() in session.Process.name().lower():
                    # Check if volume interface is available
                    if session._volume is None:
                        logger.debug(f"No volume interface for {session.Process.name()}")
                        continue
                    return int(session._volume.GetMasterVolume() * 100)
        except Exception as e:
            logger.error(f"Failed to get volume for {name}: {e}")
        return None
    
    def send_volume(self, tag: str, value: Optional[int]) -> None:
        """Send volume level to Arduino via serial.
        
        Args:
            tag: Identifier tag for the volume type
            value: Volume level to send
        """
        if value is not None and self.serial_connection:
            try:
                msg = f"{tag}:{value}\n"
                self.serial_connection.write(msg.encode())
                logger.debug(f"Sent: {msg.strip()}")
            except Exception as e:
                logger.error(f"Failed to send volume data: {e}")

    def handle_command(self, cmd: str) -> None:
        """Process a command received from Arduino.
        
        Args:
            cmd: Command string from serial
        """
        if not cmd:
            return
            
        logger.info(f"Received command: {cmd}")
        
        try:
            # Extract button name from command
            if cmd.startswith("Action: "):
                button_name = cmd.replace("Action: ", "")
                self._handle_button_action(button_name)
                return

            # Direct volume commands: `volume_<target>:<value>`
            m_vol = re.match(r'volume[_\s]?([^:]+)[:\s]*(\d+)', cmd, re.IGNORECASE)
            if m_vol:
                target = m_vol.group(1).lower()
                value = int(m_vol.group(2))
                if target == 'system':
                    logger.info(f"Applying system volume: {value}%")
                    self.set_system_volume(value)
                else:
                    # Try to find matching configured application by process/name
                    volume_config = CONFIG.get('volume', {}).get('applications', {})
                    matched = False
                    for slider_key, slider_config in volume_config.items():
                        proc = (slider_config.get('process') or '').lower()
                        name = (slider_config.get('name') or '').lower()
                        proc_norm = proc.replace('.exe', '')
                        if proc_norm and proc_norm.startswith(target):
                            app_identifier = slider_config.get('process') or slider_config.get('name')
                            logger.info(f"Applying {app_identifier} volume: {value}%")
                            self.set_app_volume(value, app_name=app_identifier)
                            matched = True
                            break
                        if target in name.replace(' ', ''):
                            app_identifier = slider_config.get('process') or slider_config.get('name')
                            logger.info(f"Applying {app_identifier} volume: {value}%")
                            self.set_app_volume(value, app_name=app_identifier)
                            matched = True
                            break
                    if not matched:
                        logger.warning(f"No matching app for volume target '{target}'")
                return

            # Short slider format: `slider_3:95` or `slider 3:95`
            m_sl = re.match(r'slider[_\s]?(\d+)[:\s]*(\d+)', cmd, re.IGNORECASE)
            if m_sl:
                slider_num = int(m_sl.group(1))
                value = int(m_sl.group(2))
                # Lookup slider config and apply
                volume_config = CONFIG.get('volume', {}).get('applications', {})
                slider_key = f'slider_{slider_num}'
                if slider_key in volume_config:
                    slider_config = volume_config[slider_key]
                    app_process = slider_config.get('process')
                    if app_process:
                        logger.info(f"Applying {slider_config.get('name')} volume: {value}%")
                        self.set_app_volume(value, app_name=app_process)
                    else:
                        logger.info(f"Applying system volume: {value}% (from slider_{slider_num})")
                        self.set_system_volume(value)
                else:
                    logger.warning(f"No configuration found for slider {slider_num}")
                return

            # Legacy slider command containing the word 'value:'
            if "slider" in cmd.lower() and "value:" in cmd.lower():
                self._handle_slider_command(cmd)

        except Exception as e:
            logger.error(f"Error handling command '{cmd}': {e}")
    
    def _handle_button_action(self, button_name: str) -> None:
        """Handle button action based on config.
        
        Args:
            button_name: Name of the button pressed
        """
        buttons_config = CONFIG.get('buttons', {})
        button_config = buttons_config.get(button_name, {})
        
        # Fallback: if button not found, try to match by action type
        if not button_config:
            for btn_name, btn_cfg in buttons_config.items():
                if btn_cfg.get('action') == button_name:
                    button_config = btn_cfg
                    button_name = btn_name
                    logger.debug(f"Matched button '{button_name}' by action type '{button_name}'")
                    break
        
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
            keys_config = button_config.get('value', [])
            keys = ast.literal_eval(keys_config)
            if keys:
                pyautogui.hotkey(*keys)
                logger.info(f"Pressed hotkey: {'+'.join(keys)}")
        
        elif action == 'mute_toggle':
            self._toggle_mute()
        
        elif action == 'open':
            # YouTube play/pause - press 'k'
            path = button_config.get('value', [])
            if path:
             os.system(path)
             subprocess.call(path)
        elif action == 'openb':
            path = button_config.get('value')
            web = button_config.get('web')
            brave_path = path

            # Build argument list for subprocess
            args = [brave_path, "--new-tab"]
            if web:
                args.append(web)

            try:
                subprocess.Popen(args)
            except FileNotFoundError:
                print(f"[ERROR] Could not find Brave at {brave_path}")
            except Exception as e:
                print(f"[ERROR] Failed to open Brave: {e}")
   
        
        elif action == 'none':
            logger.info(f"{button_name} button pressed (no action configured)")
        
        else:
            logger.warning(f"Unknown action type: {action}")
    
    def _toggle_mute(self) -> None:
        """Toggle system mute using nircmd."""
       
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
        """Handle slider volume commands.
        
        Args:
            cmd: Slider command string
        """
        # Accept flexible formats: "slider 3 value: 95", "slider_3:95", "slider 3:95"
        match = re.search(r'slider[_\s]?(\d+).*?value[:\s]*(\d+)', cmd, re.IGNORECASE)
        if not match:
            # Try short form like "slider_3:95" or "slider 3:95"
            match = re.search(r'slider[_\s]?(\d+)[:\s]*(\d+)', cmd, re.IGNORECASE)

        if not match:
            logger.warning(f"Invalid slider command format: {cmd}")
            return

        slider_num = int(match.group(1))
        value = int(match.group(2))

        # Get slider configuration from config
        volume_config = CONFIG.get('volume', {}).get('applications', {})
        slider_key = f'slider_{slider_num}'

        if slider_key not in volume_config:
            logger.warning(f"No configuration found for slider {slider_num}")
            return

        slider_config = volume_config[slider_key]
        app_name_cfg = slider_config.get('name')
        app_process_cfg = slider_config.get('process')

        if self.debug:
            # Print parsed info and configured candidates
            logger.debug(f"[DEBUG] slider_cmd='{cmd}' -> slider={slider_num}, value={value}")
            logger.debug(f"[DEBUG] configured name='{app_name_cfg}', process='{app_process_cfg}'")

        # If a process is explicitly configured, prefer that
        if app_process_cfg:
            if self.debug:
                logger.debug(f"[DEBUG] Applying by configured process '{app_process_cfg}'")
            try:
                self.set_app_volume(value, app_name=app_process_cfg)
                logger.info(f"{app_name_cfg or app_process_cfg} volume set to {value}%")
            except Exception as e:
                logger.error(f"Failed to set app volume for {app_process_cfg}: {e}")
            return

        # No explicit process; attempt robust matching among running sessions
        target = (app_name_cfg or '').lower().replace(' ', '')
        candidates = []
        try:
            sessions = AudioUtilities.GetAllSessions()
            for session in sessions:
                if session.Process:
                    proc_name = session.Process.name()
                    candidates.append(proc_name)
                    proc_norm = proc_name.lower().replace('.exe', '')
                    # Exact/substring matches
                    if target and (target in proc_norm or target in proc_name.lower()):
                        if self.debug:
                            logger.debug(f"[DEBUG] matched running process '{proc_name}' for target '{target}'")
                        try:
                            self.set_app_volume(value, app_name=proc_name)
                            logger.info(f"{proc_name} volume set to {value}% (matched by running process)")
                        except Exception as e:
                            logger.error(f"Failed to set app volume for {proc_name}: {e}")
                        return
        except Exception as e:
            logger.debug(f"[DEBUG] Could not enumerate sessions for matching: {e}")

        # If nothing matched, fall back to system volume
        if self.debug:
            logger.debug(f"[DEBUG] candidate processes: {candidates}")
            logger.debug(f"[DEBUG] falling back to system volume for slider_{slider_num}")

        self.set_system_volume(value)
        logger.info(f"System volume set to {value}%")
    
    def update_volume_feedback(self) -> None:
        """Send current volume levels to Arduino for display."""
        volume_config = CONFIG.get('volume', {}).get('applications', {})
        
        # Send volume for each configured slider
        for slider_key, slider_config in volume_config.items():
            app_name = slider_config.get('name', '')
            if app_name and app_name != 'System':
                vol = self.get_volume_by_name(app_name)
                tag = slider_key.replace('slider_', 'SL')
                self.send_volume(tag, vol)
    
    def run(self) -> None:
        """Main loop to read serial commands and update volume feedback."""
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
        """Clean up resources."""
        if self.serial_connection and self.serial_connection.is_open:
            logger.info("Closing serial connection...")
            self.serial_connection.close()
        logger.info("Cleanup complete")


def main():
    """Main entry point."""
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
