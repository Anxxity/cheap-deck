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

CONFIG = DEFAULT_CONFIG

class CheapDeck:
  def __init__(self, com_port: str, baud_rate: int = 9600, timeout: float = 1.0)
     self.com_port = com_port
     self.baud_rate = baud_rate
     self.timeout = timeout
     self.serial_connection = None
     self.volume_interface = None

