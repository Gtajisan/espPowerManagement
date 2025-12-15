# ESP SecMonitor - Power Management + WiFi Security Testing

## Project Overview

**ESP SecMonitor** is an advanced ESP8266-based system combining comprehensive power monitoring with WiFi security testing capabilities. It features a modern dark-themed Bootstrap 5 dashboard with WPS vulnerability detection based on 576+ device signatures.

**Version:** 2.1.1  
**Author:** Gtajisan (ffjisan804@gmail.com)  
**License:** MIT

## Project Structure

```
espPowerManagement/
├── src/
│   └── main.cpp              # ESP8266 firmware source code
├── data/
│   └── vulndb.txt            # Vulnerable device database
├── .github/
│   └── workflows/
│       └── build.yml         # GitHub Actions for automated builds
├── dashboard_preview/
│   └── app.py                # Web preview of ESP dashboard
├── platformio.ini            # PlatformIO build configuration
├── README.md                 # Main documentation
├── SETUP_GUIDE.md            # Complete setup instructions
├── FLASHING_GUIDE.md         # Firmware flashing guide
├── FEATURES.md               # Feature documentation
├── API.md                    # REST API documentation
├── CHANGELOG.md              # Version history
└── LICENSE                   # MIT License
```

## Key Features

### Power Monitoring
- Automatic power ON/OFF event detection
- Duration calculation for power outages
- Statistics: Today, 7-day, 15-day, monthly analysis
- Monthly auto-reset to prevent storage overflow

### WiFi Security Testing
- Network scanning with hidden network detection
- WPS vulnerability detection (576+ device signatures)
- Manufacturer identification using OUI database
- WiFi standard detection (WiFi 4/5/6)
- Security protocol analysis (WEP, WPA, WPA2, WPA3)

### Supported Vulnerable Devices
- **TP-Link:** Archer AX/C series, TL-WR/MR series, Deco mesh
- **Xiaomi/Redmi:** AX1800-9000, Mi Router series
- **Netgear:** RAX series, Nighthawk, WNR/WNDR
- **ASUS:** RT-AX series, TUF-AX, ZenWiFi
- **D-Link:** DIR-X series, EAGLE PRO AI
- **Others:** Huawei, ZTE, Tenda, Linksys, and 500+ more

## Usage

### For ESP8266 Development

1. Install PlatformIO or Arduino IDE
2. Clone or download the repository
3. Build: `pio run -e esp8266`
4. Flash: `pio run -e esp8266 -t upload`
5. Connect to `ESP_SecMonitor` WiFi (password: `12345678`)
6. Access dashboard: `http://192.168.4.1`

### Dashboard Preview (Replit)

The dashboard preview runs on port 5000 and demonstrates the web interface design.

## GitHub Actions

The project includes automated firmware builds:
- Triggers on push to main/master branches
- Builds for D1 Mini and NodeMCU boards
- Creates releases with firmware binaries on tags

## Credits

Based on:
- [espPowerManagement](https://github.com/anbuinfosec/espPowerManagement) by @anbuinfosec
- [FARHAN-Shot-v2](https://github.com/GazaOS/FARHAN-Shot-v2) by GazaOS
