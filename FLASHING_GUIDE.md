# 🔌 ESP SecMonitor - Firmware Flashing Guide

Complete guide for flashing the ESP SecMonitor firmware to your ESP8266 device.

---

## 📋 Table of Contents

1. [Prerequisites](#prerequisites)
2. [Download Firmware](#download-firmware)
3. [Flashing Methods](#flashing-methods)
   - [ESPTool (Command Line)](#method-1-esptool-command-line)
   - [ESP Flash Download Tool (Windows GUI)](#method-2-esp-flash-download-tool-windows)
   - [NodeMCU Flasher (Windows)](#method-3-nodemcu-flasher-windows)
   - [Arduino IDE](#method-4-arduino-ide)
   - [PlatformIO](#method-5-platformio)
4. [Post-Flash Setup](#post-flash-setup)
5. [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Hardware Required
- ESP8266 board (NodeMCU, Wemos D1 Mini, or generic ESP-12)
- Micro USB cable (data capable, not charge-only)
- Computer with USB port

### Software Required
- USB-to-Serial drivers (usually auto-installed):
  - CH340: [Download](http://www.wch-ic.com/downloads/CH341SER_ZIP.html)
  - CP2102: [Download](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
  - FTDI: [Download](https://ftdichip.com/drivers/vcp-drivers/)

---

## Download Firmware

1. Go to the [Releases](https://github.com/Gtajisan/espPowerManagement/releases) page
2. Download the appropriate firmware:
   - `esp8266_d1mini_firmware.bin` - For Wemos D1 Mini
   - `esp8266_nodemcu_firmware.bin` - For NodeMCU v2
3. Note the download location

---

## Flashing Methods

### Method 1: ESPTool (Command Line)

**Best for:** Linux, macOS, advanced users

#### Installation

```bash
# Install via pip
pip install esptool

# Or via pipx (recommended)
pipx install esptool
```

#### Flashing Steps

```bash
# 1. Find your serial port
# Linux: ls /dev/ttyUSB* or ls /dev/ttyACM*
# macOS: ls /dev/cu.usbserial-*
# Windows: Check Device Manager for COM port

# 2. Erase flash (recommended for clean install)
esptool.py --chip esp8266 --port /dev/ttyUSB0 erase_flash

# 3. Flash firmware
esptool.py --chip esp8266 --port /dev/ttyUSB0 --baud 115200 write_flash 0x0 esp8266_d1mini_firmware.bin
```

#### Windows Example
```cmd
esptool.py --chip esp8266 --port COM3 --baud 115200 write_flash 0x0 esp8266_d1mini_firmware.bin
```

---

### Method 2: ESP Flash Download Tool (Windows)

**Best for:** Windows users, beginners

#### Download
[ESP Flash Download Tool](https://www.espressif.com/en/support/download/other-tools)

#### Steps

1. Extract and run `flash_download_tool.exe`
2. Select **ESP8266 DownloadTool**
3. Configure:
   - **SPI SPEED:** 40MHz
   - **SPI MODE:** QIO or DIO
   - **FLASH SIZE:** 4MB or match your board
4. Add firmware file:
   - Click `...` and select your `.bin` file
   - Set address: `0x0`
   - Check the checkbox to enable
5. Select your **COM port**
6. Set **BAUD:** 115200
7. Click **START**
8. Wait for "FINISH" message

---

### Method 3: NodeMCU Flasher (Windows)

**Best for:** Windows users with NodeMCU boards

#### Download
[NodeMCU Flasher](https://github.com/nodemcu/nodemcu-flasher/releases)

#### Steps

1. Run `ESP8266Flasher.exe`
2. Go to **Config** tab
3. Click gear icon and select your `.bin` file
4. Set address: `0x00000`
5. Go to **Operation** tab
6. Select COM port
7. Click **Flash**
8. Wait for completion (green checkmark)

---

### Method 4: Arduino IDE

**Best for:** Users familiar with Arduino

#### Setup

1. **Install ESP8266 Board Package:**
   - File → Preferences → Additional Board Manager URLs
   - Add: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
   - Tools → Board → Board Manager → Search "ESP8266" → Install

2. **Install ArduinoJson Library:**
   - Sketch → Include Library → Manage Libraries
   - Search "ArduinoJson" by Benoit Blanchon → Install

3. **Configure Board:**
   - Tools → Board → ESP8266 Boards → NodeMCU 1.0 (or your board)
   - Tools → Port → Select your COM/USB port
   - Tools → Upload Speed → 115200

4. **Upload:**
   - Open `src/main.cpp`
   - Save as `main.ino`
   - Click Upload button

---

### Method 5: PlatformIO

**Best for:** Developers, CI/CD pipelines

#### Installation

```bash
# Install PlatformIO Core
pip install platformio

# Or use VS Code extension
```

#### Build and Flash

```bash
# Clone repository
git clone https://github.com/Gtajisan/espPowerManagement.git
cd espPowerManagement

# Build for D1 Mini
pio run -e esp8266

# Build for NodeMCU
pio run -e esp8266_nodemcu

# Upload to D1 Mini
pio run -e esp8266 -t upload

# Monitor serial output
pio device monitor
```

---

## Post-Flash Setup

### 1. Connect to Device

After flashing, the ESP8266 will create a WiFi access point:

| Setting | Value |
|---------|-------|
| **SSID** | `ESP_SecMonitor` |
| **Password** | `12345678` |

### 2. Access Dashboard

1. Connect your phone/computer to `ESP_SecMonitor` WiFi
2. Open browser and go to: `http://192.168.4.1`

### 3. Configure WiFi (Optional)

To connect to your home/office WiFi:

1. Go to Config page
2. Enter your WiFi SSID and password
3. Click "Save & Connect"
4. Device will restart and connect to your network
5. Find the new IP in your router's DHCP client list or serial monitor

---

## Troubleshooting

### Device Not Detected

**Symptoms:** COM port not showing, device not recognized

**Solutions:**
1. Try different USB cable (must be data cable, not charge-only)
2. Install appropriate USB driver (CH340, CP2102, or FTDI)
3. Try different USB port
4. On Linux, add user to dialout group: `sudo usermod -a -G dialout $USER`

### Flash Failed

**Symptoms:** Connection timeout, sync failed

**Solutions:**
1. Hold FLASH/BOOT button while pressing RESET, then release
2. Try lower baud rate (9600)
3. Check if another program is using the COM port
4. Ensure correct board is selected

### Boot Loop / Crash

**Symptoms:** Device keeps restarting, LED blinking rapidly

**Solutions:**
1. Erase flash completely before re-flashing
2. Ensure correct flash size is selected
3. Try DIO mode instead of QIO
4. Check power supply (use USB from computer, not hub)

### Can't Connect to AP

**Symptoms:** `ESP_SecMonitor` WiFi not visible

**Solutions:**
1. Wait 30 seconds after power-on
2. Move closer to the device
3. Check if device LED is solid (not blinking)
4. Try 2.4GHz WiFi on your phone (ESP8266 is 2.4GHz only)

### Dashboard Not Loading

**Symptoms:** Can connect to WiFi but http://192.168.4.1 doesn't load

**Solutions:**
1. Clear browser cache
2. Try different browser
3. Disable VPN/proxy
4. Use IP address directly (not DNS)

---

## Serial Monitor Debug

To see debug output, connect via serial monitor at 115200 baud:

```bash
# PlatformIO
pio device monitor

# ESPTool
python -m serial.tools.miniterm /dev/ttyUSB0 115200

# Arduino IDE
Tools → Serial Monitor → Set baud to 115200
```

Expected output on successful boot:
```
========================================
ESP SecMonitor v2.1.1
Author: Gtajisan (ffjisan804@gmail.com)
========================================

SPIFFS mounted successfully
Configuration loaded
AP Started - SSID: ESP_SecMonitor, IP: 192.168.4.1
NTP configured (UTC+6)
HTTP server started on port 80

========================================
Setup complete! Device ready.
========================================
```

---

## Need Help?

- Check [Issues](https://github.com/Gtajisan/espPowerManagement/issues) for known problems
- Open a new issue with debug output
- Contact: [ffjisan804@gmail.com](mailto:ffjisan804@gmail.com)
