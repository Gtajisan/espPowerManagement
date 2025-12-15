# ⚡🔒 ESP SecMonitor - Power Management + WiFi Security Testing

[![Platform](https://img.shields.io/badge/platform-ESP8266-blue?style=flat-square)](https://www.espressif.com/en/products/socs/esp8266)
[![Version](https://img.shields.io/badge/version-2.1.1-brightgreen?style=flat-square)](https://github.com/Gtajisan/espPowerManagement/releases)
[![License](https://img.shields.io/badge/license-MIT-green?style=flat-square)](LICENSE)
[![Build Status](https://img.shields.io/github/actions/workflow/status/Gtajisan/espPowerManagement/build.yml?style=flat-square)](https://github.com/Gtajisan/espPowerManagement/actions)
[![Author](https://img.shields.io/badge/author-Gtajisan-ff69b4?style=flat-square)](mailto:ffjisan804@gmail.com)

**Advanced ESP8266 system combining comprehensive power monitoring with WiFi security testing capabilities. Features a modern dark-themed Bootstrap 5 dashboard with WPS vulnerability detection.**

---

## 🌟 Features

### ⚡ Power Management
- **Automatic Power Detection** - Monitors device power state without external sensors
- **Event Logging** - Records every power ON/OFF event with precise timestamps
- **Duration Calculation** - Automatically calculates power outage durations
- **Statistics Dashboard** - Today, 7-day, 15-day, and monthly power analysis
- **Monthly Auto-Reset** - Prevents storage overflow with automatic log cleanup

### 🔒 WiFi Security Testing
- **Network Scanning** - Discovers all nearby WiFi networks including hidden ones
- **WPS Vulnerability Detection** - Identifies potentially vulnerable routers from 576+ device signatures
- **Manufacturer Identification** - Detects device manufacturers using OUI database
- **WiFi Standard Detection** - Identifies WiFi 4/5/6 networks
- **Security Protocol Analysis** - Shows WEP, WPA, WPA2, WPA3 encryption types
- **Signal Strength Indicators** - Visual RSSI analysis with color-coded badges

### 🎨 Modern Web Interface
- **Dark Theme Dashboard** - Sleek, modern UI with purple gradient accents
- **Fully Responsive** - Works on desktop, tablet, and mobile devices
- **Bootstrap 5** - Latest Bootstrap framework with Bootstrap Icons
- **Real-time Updates** - Live statistics and network information
- **RESTful API** - JSON endpoints for programmatic access

### 📡 Supported Vulnerable Devices (576+ Signatures)

Based on the **FARHAN-Shot-v2** vulnerability database:

| Manufacturer | Vulnerable Models |
|-------------|-------------------|
| **TP-Link** | Archer AX10/20/50/55/73/75/90/96, Deco X20/X50/X60/X90, TL-WR/MR series |
| **Xiaomi/Redmi** | AX1800/3000/3200/3600/5400/6000/6S/9000, Mi Router 3/3G/4/4A/4C/AC2100 |
| **Netgear** | RAX10-200, Nighthawk AX4/6/8/12, WNR/WNDR series, R6000-R8000 |
| **ASUS** | RT-AX53U-92U, TUF-AX3000/4200/5400, ZenWiFi AX/XD series |
| **D-Link** | DIR-X1560-6060, EAGLE PRO AI, DIR-615/655/809/819/842 |
| **Huawei/Honor** | AX2/AX3/AX3 Pro, WiFi AX2/AX3, Honor Router 3/4/X3 |
| **ZTE** | AX1800/3000/3000 Pro/5400, MC7010/MC888 |
| **Tenda** | AC6-23, AX3/9/12/1803/3000, TX3/9, F3/6/9, W15E/18E/30E |
| **Others** | Linksys, Belkin, Mercusys, Totolink, Cudy, Wavlink, Comfast, and 400+ more |

---

## 🛠️ Hardware Requirements

- **ESP8266** (NodeMCU, Wemos D1 Mini, or Generic ESP-12)
- **USB Cable** for programming
- **5V Power Supply** (for continuous monitoring)
- **No external sensors, RTC, or SD card required**

---

## 🚀 Quick Start

### Option 1: Pre-compiled Binary (Easiest)

1. Download the latest firmware from [Releases](https://github.com/Gtajisan/espPowerManagement/releases)
2. Flash using [ESP Flash Download Tool](https://www.espressif.com/en/support/download/other-tools) or ESPTool:

```bash
# Using ESPTool
esptool.py --chip esp8266 --port /dev/ttyUSB0 --baud 115200 write_flash 0x0 esp8266_d1mini_firmware.bin
```

3. Connect to `ESP_SecMonitor` WiFi (password: `12345678`)
4. Open browser: `http://192.168.4.1`

### Option 2: PlatformIO (Recommended for Development)

```bash
# Clone repository
git clone https://github.com/Gtajisan/espPowerManagement.git
cd espPowerManagement

# Build and upload
pio run -e esp8266 -t upload

# Monitor serial output
pio device monitor
```

### Option 3: Arduino IDE

1. Install ESP8266 board support in Arduino IDE
2. Install libraries: `ArduinoJson`
3. Open `src/main.cpp` and save as `.ino`
4. Select Board: Tools → Board → ESP8266 → NodeMCU 1.0
5. Upload

---

## 📖 Usage Guide

### First-Time Setup

1. **Power on the ESP8266**
2. **Connect to Access Point:**
   - SSID: `ESP_SecMonitor`
   - Password: `12345678`
3. **Open Dashboard:** `http://192.168.4.1`
4. **Configure WiFi (Optional):**
   - Go to Config page
   - Enter your WiFi credentials
   - Click "Save & Connect"
5. **Access via Station IP:** Check serial monitor for assigned IP

### Web Interface Pages

| Page | URL | Description |
|------|-----|-------------|
| **Dashboard** | `/` | Overview with stats cards and quick info |
| **Power** | `/power` | Power event history and statistics |
| **WiFi Scan** | `/scan` | Network scanner with vulnerability detection |
| **Stats** | `/stats` | Device hardware and storage information |
| **Config** | `/config` | WiFi and AP configuration settings |
| **About** | `/about` | Project information and credits |

### API Endpoints

```bash
# Get WiFi scan results as JSON
GET /api/scan

# Get device statistics as JSON
GET /api/stats
```

---

## 📁 Project Structure

```
espPowerManagement/
├── src/
│   └── main.cpp              # Main firmware source code
├── .github/
│   └── workflows/
│       └── build.yml         # GitHub Actions CI/CD
├── platformio.ini            # PlatformIO configuration
├── README.md                 # This file
├── SETUP_GUIDE.md           # Detailed setup instructions
├── FEATURES.md              # Complete feature documentation
├── API.md                   # API documentation
├── FLASHING_GUIDE.md        # Firmware flashing guide
├── CHANGELOG.md             # Version history
└── LICENSE                  # MIT License
```

---

## 🔧 Configuration

### Default Access Point Settings

| Setting | Default Value |
|---------|---------------|
| SSID | `ESP_SecMonitor` |
| Password | `12345678` |
| IP Address | `192.168.4.1` |

### Build Configuration (platformio.ini)

```ini
[env:esp8266]
platform = espressif8266
board = d1_mini
framework = arduino
monitor_speed = 115200
build_flags = 
    -DVERSION=\"2.1.1\"
    -DAUTHOR=\"Gtajisan\"
    -DEMAIL=\"ffjisan804@gmail.com\"
```

---

## 📊 Statistics Tracked

### Power Statistics
- **Current Uptime** - Time since last boot
- **Today's Power Off** - Total power off duration today
- **7-Day Analysis** - Weekly power patterns
- **15-Day Analysis** - Bi-weekly trends
- **Monthly Overview** - Complete month statistics

### Network Statistics
- **Total Networks** - Number of WiFi networks found
- **WPS Vulnerable** - Networks with potential vulnerabilities
- **Signal Strength** - RSSI levels with visual indicators

---

## ⚠️ Disclaimer

**This tool is designed for educational purposes, authorized network security testing, and personal power monitoring only.**

- Always obtain proper authorization before testing network security
- Unauthorized access to computer networks is illegal in most jurisdictions
- The developers are not responsible for any misuse of this tool
- Use responsibly and ethically

---

## 🙏 Credits

| Role | Name | Contact |
|------|------|---------|
| **Author** | Gtajisan | [ffjisan804@gmail.com](mailto:ffjisan804@gmail.com) |
| **Power Monitoring Base** | @anbuinfosec | [espPowerManagement](https://github.com/anbuinfosec/espPowerManagement) |
| **Security Testing Base** | GazaOS/Gtajisan | [FARHAN-Shot-v2](https://github.com/GazaOS/FARHAN-Shot-v2) |

---

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

---

<p align="center">
  Made with ❤️ by <a href="mailto:ffjisan804@gmail.com">Gtajisan</a>
  <br>
  ⭐ Star this repository if you find it useful!
</p>
