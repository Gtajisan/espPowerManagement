# 📡 ESP SecMonitor - API Documentation

REST API documentation for programmatic access to ESP SecMonitor.

---

## Base URL

When connected to the device:
- **AP Mode:** `http://192.168.4.1`
- **Station Mode:** `http://<device-ip>`

---

## Endpoints

### Web Pages

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/` | Dashboard home page |
| GET | `/power` | Power history page |
| GET | `/scan` | WiFi scan page |
| GET | `/scan?start=1` | Trigger scan and show page |
| GET | `/stats` | Device statistics page |
| GET | `/config` | Configuration page |
| GET | `/about` | About page |

### API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/scan` | Scan networks and return JSON |
| GET | `/api/stats` | Get device statistics as JSON |

### Actions

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/clear` | Clear power logs |
| GET | `/clearscan` | Clear scan results |
| GET | `/restart` | Restart device |
| POST | `/save` | Save WiFi credentials |
| POST | `/saveap` | Save AP settings |

---

## API: /api/scan

Performs a WiFi network scan and returns results as JSON.

### Request

```http
GET /api/scan HTTP/1.1
Host: 192.168.4.1
```

### Response

```json
{
  "networks": [
    {
      "ssid": "HomeNetwork",
      "bssid": "AA:BB:CC:DD:EE:FF",
      "rssi": -45,
      "channel": 6,
      "security": "WPA2",
      "manufacturer": "TP-Link",
      "wps_vulnerable": true,
      "hidden": false
    },
    {
      "ssid": "OfficeWiFi",
      "bssid": "11:22:33:44:55:66",
      "rssi": -72,
      "channel": 11,
      "security": "WPA2/WPA3",
      "manufacturer": "ASUS",
      "wps_vulnerable": false,
      "hidden": false
    },
    {
      "ssid": "",
      "bssid": "77:88:99:AA:BB:CC",
      "rssi": -80,
      "channel": 1,
      "security": "WPA2",
      "manufacturer": "Unknown",
      "wps_vulnerable": false,
      "hidden": true
    }
  ],
  "count": 3,
  "timestamp": "2025-12-15 14:30:45"
}
```

### Response Fields

| Field | Type | Description |
|-------|------|-------------|
| `networks` | Array | List of discovered networks |
| `networks[].ssid` | String | Network name (empty if hidden) |
| `networks[].bssid` | String | MAC address of access point |
| `networks[].rssi` | Integer | Signal strength in dBm |
| `networks[].channel` | Integer | WiFi channel (1-14) |
| `networks[].security` | String | Security protocol |
| `networks[].manufacturer` | String | Device manufacturer |
| `networks[].wps_vulnerable` | Boolean | WPS vulnerability status |
| `networks[].hidden` | Boolean | Hidden network flag |
| `count` | Integer | Total networks found |
| `timestamp` | String | Scan timestamp |

### RSSI Interpretation

| RSSI | Quality |
|------|---------|
| > -50 | Excellent |
| -50 to -60 | Good |
| -60 to -70 | Fair |
| < -70 | Poor |

---

## API: /api/stats

Returns device statistics and system information.

### Request

```http
GET /api/stats HTTP/1.1
Host: 192.168.4.1
```

### Response

```json
{
  "uptime": 86400,
  "uptime_formatted": "1d 0h 0m 0s",
  "free_heap": 35420,
  "chip_id": "abc123",
  "networks_scanned": 15,
  "power": {
    "today_off": 1800,
    "today_on": 82800,
    "week_off": 10800,
    "week_on": 594000,
    "month_off": 43200,
    "month_on": 2548800
  }
}
```

### Response Fields

| Field | Type | Description |
|-------|------|-------------|
| `uptime` | Integer | Uptime in seconds |
| `uptime_formatted` | String | Human-readable uptime |
| `free_heap` | Integer | Free memory in bytes |
| `chip_id` | String | ESP8266 chip ID (hex) |
| `networks_scanned` | Integer | Networks in last scan |
| `power` | Object | Power statistics |
| `power.today_off` | Integer | Today's off time (seconds) |
| `power.today_on` | Integer | Today's on time (seconds) |
| `power.week_off` | Integer | 7-day off time (seconds) |
| `power.week_on` | Integer | 7-day on time (seconds) |
| `power.month_off` | Integer | Monthly off time (seconds) |
| `power.month_on` | Integer | Monthly on time (seconds) |

---

## POST: /save

Save WiFi station credentials.

### Request

```http
POST /save HTTP/1.1
Host: 192.168.4.1
Content-Type: application/x-www-form-urlencoded

ssid=MyNetwork&pass=MyPassword123
```

### Parameters

| Parameter | Required | Description |
|-----------|----------|-------------|
| `ssid` | Yes | WiFi network name |
| `pass` | Yes | WiFi password |

### Response

HTML confirmation page. Device restarts after 3 seconds.

---

## POST: /saveap

Save Access Point settings.

### Request

```http
POST /saveap HTTP/1.1
Host: 192.168.4.1
Content-Type: application/x-www-form-urlencoded

apssid=MyESP&appass=SecurePassword123
```

### Parameters

| Parameter | Required | Description |
|-----------|----------|-------------|
| `apssid` | Yes | AP network name |
| `appass` | Yes | AP password (min 8 chars) |

### Response

HTML confirmation page. Device restarts after 3 seconds.

---

## Usage Examples

### cURL

```bash
# Get scan results
curl -s http://192.168.4.1/api/scan | jq

# Get statistics
curl -s http://192.168.4.1/api/stats | jq

# Configure WiFi
curl -X POST http://192.168.4.1/save \
  -d "ssid=MyNetwork&pass=MyPassword"

# Restart device
curl http://192.168.4.1/restart
```

### Python

```python
import requests
import json

BASE_URL = "http://192.168.4.1"

# Scan networks
response = requests.get(f"{BASE_URL}/api/scan")
data = response.json()

print(f"Found {data['count']} networks:")
for net in data['networks']:
    vuln = "⚠️ VULNERABLE" if net['wps_vulnerable'] else "✓ OK"
    print(f"  {net['ssid']:20} {net['rssi']:4}dBm {vuln}")

# Get statistics
stats = requests.get(f"{BASE_URL}/api/stats").json()
print(f"\nUptime: {stats['uptime_formatted']}")
print(f"Free Memory: {stats['free_heap']} bytes")
```

### JavaScript

```javascript
// Scan networks
fetch('http://192.168.4.1/api/scan')
  .then(res => res.json())
  .then(data => {
    console.log(`Found ${data.count} networks`);
    data.networks.forEach(net => {
      console.log(`${net.ssid}: ${net.rssi}dBm`);
    });
  });

// Get stats
fetch('http://192.168.4.1/api/stats')
  .then(res => res.json())
  .then(stats => {
    console.log(`Uptime: ${stats.uptime_formatted}`);
  });
```

### Home Assistant (REST Sensor)

```yaml
sensor:
  - platform: rest
    name: "ESP Networks"
    resource: http://192.168.4.1/api/stats
    value_template: "{{ value_json.networks_scanned }}"
    json_attributes:
      - uptime_formatted
      - free_heap
    scan_interval: 300
```

---

## Error Handling

### HTTP Status Codes

| Code | Meaning |
|------|---------|
| 200 | Success |
| 404 | Endpoint not found |

### Error Response

For 404 errors, an HTML page is returned with a "Page Not Found" message.

---

## Rate Limiting

There is no rate limiting implemented. However:
- WiFi scans take 2-5 seconds
- Rapid requests may cause memory issues
- Recommend minimum 5 second interval between scans

---

## CORS

CORS headers are not set by default. For browser-based access from different origins, you may need to:
1. Use a proxy
2. Modify firmware to add CORS headers
3. Access from same origin
