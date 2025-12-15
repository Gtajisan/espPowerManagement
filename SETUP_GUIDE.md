# 📘 ESP SecMonitor - Complete Setup Guide

Comprehensive guide to setting up and using the ESP SecMonitor power management and WiFi security testing system.

---

## 🎯 Overview

ESP SecMonitor is an ESP8266-based system that provides:
- **Power Monitoring:** Automatic tracking of power ON/OFF events
- **WiFi Security Testing:** Network scanning with WPS vulnerability detection
- **Modern Dashboard:** Bootstrap 5 dark-themed web interface

---

## 📦 What's Included

| Feature | Description |
|---------|-------------|
| Power Event Logging | Automatic detection of power state changes |
| Duration Calculation | Time between power events |
| Statistics Dashboard | Today, 7-day, 15-day, monthly analysis |
| Network Scanner | Discovers nearby WiFi networks |
| WPS Detection | Identifies potentially vulnerable routers |
| Manufacturer ID | OUI-based device identification |
| Web Interface | Responsive dashboard on port 80 |
| REST API | JSON endpoints for automation |

---

## 🔧 Hardware Setup

### Supported Boards

| Board | Tested | Notes |
|-------|--------|-------|
| Wemos D1 Mini | ✅ | Recommended, compact |
| NodeMCU v2 | ✅ | Good for development |
| NodeMCU v3 | ✅ | Larger form factor |
| ESP-12E/F | ✅ | Requires adapter |
| ESP-01 | ⚠️ | Limited memory |

### Wiring (for generic ESP-12)

For boards without USB, you'll need a USB-to-Serial adapter:

| ESP8266 Pin | Adapter Pin |
|-------------|-------------|
| VCC | 3.3V |
| GND | GND |
| TX | RX |
| RX | TX |
| EN (CH_PD) | 3.3V |
| GPIO0 | GND (for flashing) |

---

## 💻 Software Installation

### Prerequisites

1. **Python 3.8+** (for PlatformIO/ESPTool)
2. **Git** (for cloning repository)
3. **USB Drivers** (CH340, CP2102, or FTDI)

### Method 1: Pre-compiled Binary

See [FLASHING_GUIDE.md](FLASHING_GUIDE.md) for detailed instructions.

### Method 2: Build from Source

```bash
# Install PlatformIO
pip install platformio

# Clone repository
git clone https://github.com/Gtajisan/espPowerManagement.git
cd espPowerManagement

# Build
pio run -e esp8266

# Upload
pio run -e esp8266 -t upload

# Monitor
pio device monitor
```

---

## 🌐 Network Configuration

### Default Access Point

On first boot, the device creates a WiFi hotspot:

| Setting | Value |
|---------|-------|
| **SSID** | `ESP_SecMonitor` |
| **Password** | `12345678` |
| **IP Address** | `192.168.4.1` |
| **Subnet** | `255.255.255.0` |

### Connecting to Your WiFi

1. Connect to `ESP_SecMonitor` AP
2. Open `http://192.168.4.1`
3. Go to **Config** page
4. Enter your WiFi credentials
5. Click **Save & Connect**
6. Device restarts and connects to your network

### Finding Device IP

After connecting to your network:

1. **Serial Monitor:** IP is printed on boot
2. **Router Admin:** Check DHCP client list
3. **Network Scanner:** Use `nmap -sP 192.168.1.0/24`
4. **mDNS:** (if enabled) `http://espsecmonitor.local`

---

## 📱 Web Interface Guide

### Dashboard (/)

The main overview page showing:
- **Current Uptime** - Time since last power on
- **Networks Found** - Total WiFi networks scanned
- **WPS Vulnerable** - Networks with potential vulnerabilities
- **Today Power Off** - Power off duration today
- **Power Statistics Table** - Detailed power analysis
- **Quick Info** - Device status summary

### Power History (/power)

- Complete log of all power ON/OFF events
- Color-coded badges (green=ON, red=OFF)
- Duration calculations for each event
- Statistics table with period breakdowns
- Clear logs button

### WiFi Scan (/scan)

- **Scan Networks** button to start scan
- Summary cards (Total, Vulnerable, Secure)
- Detailed network table:
  - SSID (with hidden network detection)
  - BSSID (MAC address)
  - Signal strength (color-coded)
  - Channel
  - Security type
  - Manufacturer
  - WPS vulnerability status

### Device Stats (/stats)

Hardware information:
- Chip ID
- Flash size
- Free heap memory
- CPU frequency
- SDK version

Storage information:
- SPIFFS total/used/free
- Usage percentage bar

Network status:
- WiFi mode
- AP/Station details
- Connected clients
- RSSI (if connected)

### Configuration (/config)

**WiFi Station Settings:**
- SSID and password for connecting to existing network

**Access Point Settings:**
- Custom AP name and password

**System Actions:**
- Clear power logs
- Clear scan data
- Restart device

### About (/about)

- Version information
- Feature list
- Supported devices
- Credits and links
- License information

---

## 🔌 API Reference

### GET /api/scan

Performs WiFi scan and returns JSON:

```json
{
  "networks": [
    {
      "ssid": "MyNetwork",
      "bssid": "AA:BB:CC:DD:EE:FF",
      "rssi": -65,
      "channel": 6,
      "security": "WPA2",
      "manufacturer": "TP-Link",
      "wps_vulnerable": true,
      "hidden": false
    }
  ],
  "count": 15,
  "timestamp": "2025-12-15 10:30:45"
}
```

### GET /api/stats

Returns device statistics:

```json
{
  "uptime": 3600,
  "uptime_formatted": "1h 0m 0s",
  "free_heap": 35420,
  "chip_id": "abc123",
  "networks_scanned": 15,
  "power": {
    "today_off": 1800,
    "today_on": 3600,
    "week_off": 10800,
    "week_on": 604800,
    "month_off": 43200,
    "month_on": 2592000
  }
}
```

---

## 🔐 Security Considerations

### Network Security

1. **Change default AP password** immediately after setup
2. **Use strong WiFi passwords** (12+ characters)
3. **Update firmware** regularly for security patches
4. **Limit network exposure** - don't expose to internet without auth

### Ethical Usage

This tool is for:
- ✅ Testing your own networks
- ✅ Authorized security assessments
- ✅ Educational purposes
- ✅ Network administration

This tool is NOT for:
- ❌ Unauthorized network access
- ❌ Attacking networks without permission
- ❌ Any illegal activities

---

## 🛠️ Customization

### Changing Default Settings

Edit `src/main.cpp`:

```cpp
// Default AP settings
String apSSID = "ESP_SecMonitor";  // Change AP name
String apPASS = "12345678";        // Change AP password

// Version info
#define VERSION "2.1.1"
#define AUTHOR "YourName"
#define EMAIL "your@email.com"
```

### Adding New Features

The codebase is modular:
- `pageHeader()` / `pageFooter()` - HTML structure
- `navbar()` - Navigation menu
- `getXxxPage()` - Individual page content
- `handleXxx()` - Request handlers
- `scanNetworks()` - WiFi scanning logic

---

## 📊 Power Monitoring Details

### How It Works

1. **Boot Detection:** On startup, ESP records boot time
2. **Last ON Check:** Reads last known ON timestamp from SPIFFS
3. **OFF Duration:** Calculates difference as power-off time
4. **Event Logging:** Logs [ON]/[OFF] events with timestamps
5. **Periodic Update:** Updates last-on timestamp every hour
6. **Monthly Reset:** Clears logs at month change

### Log Format

Stored in `/power_log.txt`:
```
[ON] 1702627200 0
[OFF] 1702627200 3600
[ON] 1702630800 0
```

Format: `[TYPE] TIMESTAMP DURATION`

---

## ❓ FAQ

**Q: Why doesn't the device detect my WiFi network?**
A: ESP8266 only supports 2.4GHz WiFi. 5GHz networks are not visible.

**Q: How accurate is the power-off duration?**
A: Within seconds of the actual duration, depending on NTP sync.

**Q: Can I use this with batteries?**
A: Yes, but power monitoring requires continuous mains power to be meaningful.

**Q: How many networks can be scanned?**
A: ESP8266 can typically detect 50-100 networks depending on memory.

**Q: Is the vulnerability detection 100% accurate?**
A: No, it's based on known device signatures. Some false positives/negatives may occur.

---

## 📞 Support

- **Issues:** [GitHub Issues](https://github.com/Gtajisan/espPowerManagement/issues)
- **Email:** [ffjisan804@gmail.com](mailto:ffjisan804@gmail.com)
- **Discussions:** [GitHub Discussions](https://github.com/Gtajisan/espPowerManagement/discussions)

---

## 📜 License

MIT License - see [LICENSE](LICENSE) file.
