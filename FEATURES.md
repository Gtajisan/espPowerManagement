# ✨ ESP SecMonitor - Complete Feature List

Comprehensive documentation of all features in ESP SecMonitor.

---

## 🔋 Power Monitoring Features

### Automatic Power Detection
- No external sensors required
- Monitors ESP8266's own power state
- Detects power-on and power-off events automatically
- Uses RTC memory for crash-resistant tracking

### Event Logging
- Records every power ON/OFF event
- Precise timestamps using NTP synchronization
- Persistent storage in SPIFFS filesystem
- Survives device restarts

### Duration Calculation
- Automatically calculates power-off duration
- Displays in human-readable format (days, hours, minutes, seconds)
- Tracks cumulative off-time for statistics

### Statistics Dashboard

| Period | Description |
|--------|-------------|
| **Today** | Current day's power statistics |
| **7 Days** | Last week's power patterns |
| **15 Days** | Bi-weekly power usage trends |
| **Monthly** | Complete month overview |

### Monthly Auto-Reset
- Automatic log cleanup at month start
- Prevents SPIFFS storage overflow
- Preserves device performance

---

## 🔒 WiFi Security Features

### Network Scanning
- Discovers all nearby WiFi networks
- Detects hidden (non-broadcast) networks
- Shows signal strength (RSSI) with visual indicators
- Identifies WiFi channels

### WPS Vulnerability Detection

Based on **FARHAN-Shot-v2** database with **576+ device signatures**:

#### Supported Manufacturers

| Manufacturer | Example Vulnerable Models |
|-------------|--------------------------|
| TP-Link | Archer AX/C series, TL-WR, Deco mesh |
| Xiaomi/Redmi | AX1800-9000, Mi Router series |
| Netgear | RAX series, Nighthawk, WNR/WNDR |
| ASUS | RT-AX series, TUF-AX, ZenWiFi |
| D-Link | DIR-X series, EAGLE PRO AI |
| Huawei/Honor | AX2/AX3, Honor Router series |
| ZTE | AX series, MC7010/MC888 |
| Tenda | AC/AX series, F/W series |
| Linksys | E/EA/WRT series, Velop |
| Belkin | F5D/F7D/F9K series |
| Netcomm | NB series |
| ZyXEL | NBG series |
| Mercusys | MW/MR/AC series |
| Totolink | A/N/X series |
| Wavlink | WL-WN series |
| Comfast | CF-E/WR series |
| LB-Link | BL series |
| Cudy | WR/AX/X series |
| GL.iNet | GL-AR/MT/AX series |
| Keenetic | Giga/Hero/Peak/Ultra series |
| Ubiquiti | UniFi 6, Dream Machine |
| Google | Nest WiFi Pro |
| Amazon | eero 6/Pro series |

### Manufacturer Identification
- OUI (Organizationally Unique Identifier) database
- Identifies device manufacturer from MAC address
- Supports major networking brands

### WiFi Standard Detection

| Detection | Description |
|-----------|-------------|
| WiFi 4 | 802.11n (2.4GHz) |
| WiFi 5 | 802.11ac (5GHz) |
| WiFi 6 | 802.11ax (detected by SSID patterns) |

### Security Protocol Analysis

| Protocol | Security Level |
|----------|---------------|
| Open | None ⚠️ |
| WEP | Legacy (weak) ⚠️ |
| WPA | Moderate |
| WPA2 | Good ✓ |
| WPA2/WPA3 | Excellent ✓ |
| WPA3 | Best ✓ |

### Signal Strength Indicators

| RSSI Range | Quality | Color |
|------------|---------|-------|
| > -50 dBm | Excellent | Green |
| -50 to -60 | Good | Blue |
| -60 to -70 | Fair | Yellow |
| < -70 dBm | Poor | Red |

---

## 🎨 Web Interface Features

### Modern Dark Theme
- Sleek dark background (#1a1a2e)
- Purple gradient accents
- High contrast for readability
- Eye-friendly for extended use

### Bootstrap 5 Framework
- Latest Bootstrap 5.3.2
- Bootstrap Icons integration
- Responsive grid system
- Modern UI components

### Mobile Responsive Design
- Works on all screen sizes
- Touch-friendly interface
- Collapsible navigation
- Optimized stat cards for mobile

### Navigation Bar
- Fixed position header
- Collapsible menu on mobile
- Active page highlighting
- Icon + text labels

### Dashboard Cards
- Gradient headers
- Shadow effects
- Hover animations
- Stat number highlighting

---

## ⚙️ Configuration Features

### WiFi Station Mode
- Connect to existing WiFi network
- Automatic reconnection on disconnect
- RSSI monitoring when connected

### Access Point Mode
- Creates standalone hotspot
- Customizable SSID and password
- Default: `ESP_SecMonitor` / `12345678`

### Fallback Mode
- AP mode activates if WiFi fails
- Always accessible via 192.168.4.1
- Automatic mode switching

### Persistent Settings
- Configuration stored in EEPROM
- Survives power cycles
- Factory reset via config page

---

## 💡 Smart Features

### LED Indicator

| LED State | Meaning |
|-----------|---------|
| Blinking | Connecting to WiFi |
| Solid ON | Ready and connected |
| Solid OFF | AP mode active |

### NTP Time Synchronization
- Automatic time sync on boot
- Configurable timezone (default: UTC+6)
- Fallback to multiple NTP servers

### Hourly Updates
- Periodic timestamp refresh
- Keeps time accurate
- Updates last-on file

### Auto-Reconnect
- Monitors WiFi connection
- Attempts reconnection every 60 seconds
- Maintains AP access during reconnection

---

## 📡 API Features

### RESTful Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/scan` | GET | WiFi scan results (JSON) |
| `/api/stats` | GET | Device statistics (JSON) |

### JSON Response Format
- Standard JSON structure
- Timestamp included
- Count/summary fields
- Nested objects for organization

### Use Cases
- Home automation integration
- Custom monitoring dashboards
- Scripted network audits
- Data logging systems

---

## 🛠️ System Features

### SPIFFS Filesystem
- Persistent data storage
- Power log files
- Scan results cache
- Configuration backup

### Memory Management
- Efficient string handling
- Dynamic allocation for scans
- Heap monitoring
- Garbage collection

### Serial Debug Output
- 115200 baud rate
- Boot messages
- Event logging
- Error reporting

---

## 📊 Hardware Information Display

### Chip Information
- Chip ID (unique identifier)
- Flash Chip ID
- Flash Size
- Real Flash Size

### Memory Statistics
- Free Heap (RAM)
- SPIFFS Total/Used/Free
- Sketch Size
- Free Sketch Space

### System Information
- CPU Frequency
- SDK Version
- Boot Version
- Boot Mode
- Reset Reason
- Reset Info

### Network Information
- WiFi Mode
- AP SSID/IP/MAC
- Connected Clients
- Station SSID/IP/MAC (if connected)
- Gateway/DNS
- RSSI

---

## 🔄 Version History

### v2.1.1 (Current)
- Combined power management + security testing
- 576+ WPS device signatures
- Modern dark theme dashboard
- Bootstrap 5.3.2
- REST API endpoints
- Manufacturer identification
- WiFi standard detection

### Based On
- espPowerManagement v2.0.1 by @anbuinfosec
- FARHAN-Shot-v2 by GazaOS/Gtajisan
