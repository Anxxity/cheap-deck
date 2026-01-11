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


