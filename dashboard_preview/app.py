"""
ESP SecMonitor Dashboard Preview
Demonstrates the web interface design for ESP8266 firmware
Author: Gtajisan (ffjisan804@gmail.com)
"""

from flask import Flask, render_template_string, jsonify
import random
import time
from datetime import datetime

app = Flask(__name__)

VERSION = "2.1.1"
AUTHOR = "Gtajisan"
EMAIL = "ffjisan804@gmail.com"
BOOT_TIME = time.time()

SAMPLE_NETWORKS = [
    {"ssid": "Archer_AX73_5G", "bssid": "AA:BB:CC:DD:EE:01", "rssi": -45, "channel": 36, "security": "WPA2", "manufacturer": "TP-Link", "wps_vulnerable": True},
    {"ssid": "ASUS_RT-AX86U", "bssid": "11:22:33:44:55:02", "rssi": -52, "channel": 6, "security": "WPA2/WPA3", "manufacturer": "ASUS", "wps_vulnerable": True},
    {"ssid": "Xiaomi_AX3000", "bssid": "66:77:88:99:AA:03", "rssi": -58, "channel": 11, "security": "WPA2", "manufacturer": "Xiaomi", "wps_vulnerable": True},
    {"ssid": "HomeNetwork", "bssid": "22:33:44:55:66:04", "rssi": -65, "channel": 1, "security": "WPA2", "manufacturer": "Netgear", "wps_vulnerable": False},
    {"ssid": "Office_WiFi", "bssid": "33:44:55:66:77:05", "rssi": -72, "channel": 9, "security": "WPA3", "manufacturer": "Cisco", "wps_vulnerable": False},
    {"ssid": "", "bssid": "44:55:66:77:88:06", "rssi": -78, "channel": 3, "security": "WPA2", "manufacturer": "Unknown", "wps_vulnerable": False, "hidden": True},
    {"ssid": "Tenda_AC21", "bssid": "55:66:77:88:99:07", "rssi": -55, "channel": 6, "security": "WPA2", "manufacturer": "Tenda", "wps_vulnerable": True},
    {"ssid": "D-Link_DIR-X4860", "bssid": "66:77:88:99:AA:08", "rssi": -48, "channel": 149, "security": "WPA2", "manufacturer": "D-Link", "wps_vulnerable": True},
]

POWER_EVENTS = [
    {"type": "[ON]", "timestamp": time.time() - 3600*24*2, "duration": 0},
    {"type": "[OFF]", "timestamp": time.time() - 3600*24*2 + 3600*8, "duration": 1800},
    {"type": "[ON]", "timestamp": time.time() - 3600*24*1, "duration": 0},
    {"type": "[OFF]", "timestamp": time.time() - 3600*24*1 + 3600*12, "duration": 900},
    {"type": "[ON]", "timestamp": time.time() - 3600*4, "duration": 0},
]

def format_duration(seconds):
    if seconds <= 0:
        return "0s"
    days = int(seconds // 86400)
    seconds %= 86400
    hours = int(seconds // 3600)
    seconds %= 3600
    minutes = int(seconds // 60)
    secs = int(seconds % 60)
    result = ""
    if days > 0:
        result += f"{days}d "
    if hours > 0 or days > 0:
        result += f"{hours}h "
    if minutes > 0 or hours > 0 or days > 0:
        result += f"{minutes}m "
    result += f"{secs}s"
    return result

def get_rssi_color(rssi):
    if rssi >= -50:
        return "success"
    if rssi >= -60:
        return "info"
    if rssi >= -70:
        return "warning"
    return "danger"

def get_time_string(timestamp):
    return datetime.fromtimestamp(timestamp).strftime("%Y-%m-%d %H:%M:%S")

BASE_HTML = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width,initial-scale=1.0">
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css" rel="stylesheet">
    <link href="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.11.1/font/bootstrap-icons.css" rel="stylesheet">
    <title>{{ title }} - ESP SecMonitor</title>
    <style>
        :root{--primary-gradient:linear-gradient(135deg,#667eea 0%,#764ba2 100%);--dark-bg:#1a1a2e;--card-bg:#16213e;--accent:#0f3460;}
        body{padding-top:70px;padding-bottom:70px;background:var(--dark-bg);color:#e8e8e8;font-family:'Segoe UI',Tahoma,sans-serif;}
        .card{border:none;border-radius:15px;box-shadow:0 8px 32px rgba(0,0,0,0.3);margin-bottom:1rem;background:var(--card-bg);}
        .card-header{background:var(--primary-gradient);color:white;border-radius:15px 15px 0 0!important;padding:1rem 1.5rem;}
        .card-body{padding:1.5rem;}
        .stat-card{text-align:center;padding:1.5rem;transition:transform 0.3s ease;}
        .stat-card:hover{transform:translateY(-5px);}
        .stat-number{font-size:2.5rem;font-weight:bold;background:var(--primary-gradient);-webkit-background-clip:text;-webkit-text-fill-color:transparent;}
        .stat-label{color:#a0a0a0;font-size:0.9rem;text-transform:uppercase;letter-spacing:1px;}
        .badge-on{background:#28a745!important;}.badge-off{background:#dc3545!important;}
        .badge-vuln{background:#ff6b6b!important;}.badge-safe{background:#51cf66!important;}
        .table{color:#e8e8e8;}.table-dark{background:var(--accent);}
        .table-striped>tbody>tr:nth-of-type(odd)>*{background:rgba(255,255,255,0.02);}
        .btn-primary{background:var(--primary-gradient);border:none;}
        .btn-primary:hover{opacity:0.9;transform:translateY(-2px);}
        footer{position:fixed;bottom:0;width:100%;padding:1rem;background:var(--card-bg);text-align:center;border-top:1px solid var(--accent);}
        .navbar{background:var(--card-bg)!important;border-bottom:1px solid var(--accent);}
        .nav-link{color:#a0a0a0!important;transition:color 0.3s;}.nav-link:hover,.nav-link.active{color:#fff!important;}
        .network-row{transition:background 0.3s;}.network-row:hover{background:rgba(102,126,234,0.1);}
        @media(max-width:768px){body{padding-bottom:80px;}.card{margin:0.5rem;}.container{padding:0.5rem;}.stat-number{font-size:1.8rem;}}
        .pulse{animation:pulse 2s infinite;}@keyframes pulse{0%,100%{opacity:1;}50%{opacity:0.5;}}
        .preview-badge{position:fixed;top:80px;right:20px;z-index:1000;background:#ff6b6b;color:white;padding:8px 16px;border-radius:20px;font-size:0.8rem;}
    </style>
</head>
<body>
    <div class="preview-badge"><i class="bi bi-eye-fill"></i> Dashboard Preview</div>
    <nav class="navbar navbar-expand-lg navbar-dark fixed-top">
        <div class="container-fluid">
            <a class="navbar-brand" href="/"><i class="bi bi-shield-lock-fill"></i> ESP SecMonitor</a>
            <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbarNav">
                <span class="navbar-toggler-icon"></span>
            </button>
            <div class="collapse navbar-collapse" id="navbarNav">
                <ul class="navbar-nav ms-auto">
                    <li class="nav-item"><a class="nav-link {{ 'active' if active=='home' else '' }}" href="/"><i class="bi bi-house-fill"></i> Dashboard</a></li>
                    <li class="nav-item"><a class="nav-link {{ 'active' if active=='power' else '' }}" href="/power"><i class="bi bi-lightning-fill"></i> Power</a></li>
                    <li class="nav-item"><a class="nav-link {{ 'active' if active=='scan' else '' }}" href="/scan"><i class="bi bi-wifi"></i> WiFi Scan</a></li>
                    <li class="nav-item"><a class="nav-link {{ 'active' if active=='stats' else '' }}" href="/stats"><i class="bi bi-graph-up"></i> Stats</a></li>
                    <li class="nav-item"><a class="nav-link {{ 'active' if active=='about' else '' }}" href="/about"><i class="bi bi-info-circle-fill"></i> About</a></li>
                </ul>
            </div>
        </div>
    </nav>
    <div class="container mt-4">
        {{ content | safe }}
    </div>
    <footer>
        <small>ESP SecMonitor v{{ version }} | Made with <span style="color:#ff6b6b;">❤</span> by 
        <a href="mailto:{{ email }}" style="color:#667eea;">{{ author }}</a> | 
        Based on <a href="https://github.com/anbuinfosec/espPowerManagement" target="_blank" style="color:#667eea;">espPowerManagement</a> & 
        <a href="https://github.com/GazaOS/FARHAN-Shot-v2" target="_blank" style="color:#667eea;">FARHAN-Shot-v2</a></small>
    </footer>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js"></script>
</body>
</html>
"""

@app.route('/')
def dashboard():
    uptime = time.time() - BOOT_TIME
    vuln_count = sum(1 for n in SAMPLE_NETWORKS if n.get('wps_vulnerable', False))
    
    content = f"""
    <div class="row mb-4">
        <div class="col-12"><h2 class="mb-4"><i class="bi bi-speedometer2"></i> Dashboard</h2></div>
    </div>
    <div class="row mb-4">
        <div class="col-md-3 col-6"><div class="card stat-card">
            <div class="stat-number">{format_duration(uptime)}</div>
            <div class="stat-label"><i class="bi bi-clock-fill"></i> Current Uptime</div>
        </div></div>
        <div class="col-md-3 col-6"><div class="card stat-card">
            <div class="stat-number">{len(SAMPLE_NETWORKS)}</div>
            <div class="stat-label"><i class="bi bi-wifi"></i> Networks Found</div>
        </div></div>
        <div class="col-md-3 col-6"><div class="card stat-card">
            <div class="stat-number">{vuln_count}</div>
            <div class="stat-label"><i class="bi bi-exclamation-triangle-fill"></i> WPS Vulnerable</div>
        </div></div>
        <div class="col-md-3 col-6"><div class="card stat-card">
            <div class="stat-number">{format_duration(2700)}</div>
            <div class="stat-label"><i class="bi bi-power"></i> Today Power Off</div>
        </div></div>
    </div>
    <div class="row">
        <div class="col-md-6">
            <div class="card"><div class="card-header"><i class="bi bi-lightning-fill"></i> Power Statistics</div>
            <div class="card-body"><table class="table table-sm">
                <tr><th>Period</th><th class="text-danger">Power OFF</th><th class="text-success">Power ON</th></tr>
                <tr><td>Today</td><td class="text-danger">{format_duration(2700)}</td><td class="text-success">{format_duration(uptime)}</td></tr>
                <tr><td>Last 7 Days</td><td class="text-danger">{format_duration(10800)}</td><td class="text-success">{format_duration(594000)}</td></tr>
                <tr><td>Last 15 Days</td><td class="text-danger">{format_duration(21600)}</td><td class="text-success">{format_duration(1274400)}</td></tr>
                <tr><td>This Month</td><td class="text-danger">{format_duration(43200)}</td><td class="text-success">{format_duration(2548800)}</td></tr>
            </table></div></div>
        </div>
        <div class="col-md-6">
            <div class="card"><div class="card-header"><i class="bi bi-info-circle-fill"></i> Quick Info</div>
            <div class="card-body"><table class="table table-sm">
                <tr><th>WiFi Mode</th><td><span class="badge bg-primary">AP + Station</span></td></tr>
                <tr><th>AP SSID</th><td>ESP_SecMonitor</td></tr>
                <tr><th>AP IP</th><td>192.168.4.1</td></tr>
                <tr><th>Current Time</th><td>{get_time_string(time.time())}</td></tr>
                <tr><th>Free Heap</th><td>35420 bytes</td></tr>
            </table>
            <a href="/scan" class="btn btn-primary w-100 mt-2"><i class="bi bi-arrow-repeat"></i> Start WiFi Scan</a>
            </div></div>
        </div>
    </div>
    """
    return render_template_string(BASE_HTML, title="Dashboard", active="home", content=content, version=VERSION, author=AUTHOR, email=EMAIL)

@app.route('/power')
def power():
    content = """
    <div class="row mb-4">
        <div class="col-12"><h2><i class="bi bi-lightning-fill"></i> Power History</h2></div>
    </div>
    <div class="card mb-4">
        <div class="card-header d-flex justify-content-between align-items-center">
            <span><i class="bi bi-list-ul"></i> Event Log</span>
            <button class="btn btn-danger btn-sm"><i class="bi bi-trash-fill"></i> Clear Logs</button>
        </div>
        <div class="card-body">
            <div class="table-responsive"><table class="table table-striped">
                <thead class="table-dark"><tr><th>#</th><th>Event</th><th>Time</th><th>Duration</th></tr></thead>
                <tbody>
    """
    for i, event in enumerate(POWER_EVENTS):
        badge = '<span class="badge badge-on"><i class="bi bi-power"></i> ON</span>' if event['type'] == '[ON]' else '<span class="badge badge-off"><i class="bi bi-power"></i> OFF</span>'
        dur = format_duration(event['duration']) if event['duration'] > 0 else "-"
        content += f'<tr><td>{i+1}</td><td>{badge}</td><td>{get_time_string(event["timestamp"])}</td><td>{dur}</td></tr>'
    
    content += """
                </tbody></table></div>
        </div>
    </div>
    """
    return render_template_string(BASE_HTML, title="Power History", active="power", content=content, version=VERSION, author=AUTHOR, email=EMAIL)

@app.route('/scan')
def scan():
    vuln_count = sum(1 for n in SAMPLE_NETWORKS if n.get('wps_vulnerable', False))
    
    content = f"""
    <div class="row mb-4">
        <div class="col-12 d-flex justify-content-between align-items-center flex-wrap">
            <h2><i class="bi bi-wifi"></i> WiFi Network Scanner</h2>
            <button class="btn btn-primary"><i class="bi bi-arrow-repeat"></i> Scan Networks</button>
        </div>
    </div>
    <div class="row mb-3">
        <div class="col-md-4 col-6"><div class="card stat-card">
            <div class="stat-number">{len(SAMPLE_NETWORKS)}</div>
            <div class="stat-label">Total Networks</div>
        </div></div>
        <div class="col-md-4 col-6"><div class="card stat-card">
            <div class="stat-number text-danger">{vuln_count}</div>
            <div class="stat-label">WPS Vulnerable</div>
        </div></div>
        <div class="col-md-4 col-12"><div class="card stat-card">
            <div class="stat-number text-success">{len(SAMPLE_NETWORKS) - vuln_count}</div>
            <div class="stat-label">Secure Networks</div>
        </div></div>
    </div>
    <div class="card">
        <div class="card-header"><i class="bi bi-list-ul"></i> Discovered Networks</div>
        <div class="card-body">
            <div class="table-responsive"><table class="table table-striped">
                <thead class="table-dark">
                    <tr><th>#</th><th>SSID</th><th>BSSID</th><th>Signal</th><th>Channel</th><th>Security</th><th>Manufacturer</th><th>WPS Status</th></tr>
                </thead><tbody>
    """
    
    for i, net in enumerate(SAMPLE_NETWORKS):
        ssid = '<em class="text-muted">[Hidden]</em>' if net.get('hidden') or not net['ssid'] else net['ssid']
        signal_color = get_rssi_color(net['rssi'])
        wps_badge = '<span class="badge badge-vuln"><i class="bi bi-exclamation-triangle-fill"></i> Vulnerable</span>' if net.get('wps_vulnerable') else '<span class="badge badge-safe"><i class="bi bi-shield-check"></i> Unknown</span>'
        
        content += f"""
        <tr class="network-row">
            <td>{i+1}</td>
            <td><strong>{ssid}</strong></td>
            <td><code>{net['bssid']}</code></td>
            <td><span class="badge bg-{signal_color}">{net['rssi']} dBm</span></td>
            <td>{net['channel']}</td>
            <td><span class="badge bg-secondary">{net['security']}</span></td>
            <td>{net['manufacturer']}</td>
            <td>{wps_badge}</td>
        </tr>
        """
    
    content += """
                </tbody></table></div>
        </div>
    </div>
    """
    return render_template_string(BASE_HTML, title="WiFi Scan", active="scan", content=content, version=VERSION, author=AUTHOR, email=EMAIL)

@app.route('/stats')
def stats():
    content = """
    <div class="row mb-4">
        <div class="col-12"><h2><i class="bi bi-cpu-fill"></i> Device Statistics</h2></div>
    </div>
    <div class="row">
        <div class="col-md-6">
            <div class="card">
                <div class="card-header"><i class="bi bi-cpu"></i> Hardware Information</div>
                <div class="card-body">
                    <table class="table table-sm">
                        <tr><th>Chip ID</th><td><code>abc123</code></td></tr>
                        <tr><th>Flash Size</th><td>4096 KB</td></tr>
                        <tr><th>Free Heap</th><td>35420 bytes</td></tr>
                        <tr><th>CPU Frequency</th><td>80 MHz</td></tr>
                        <tr><th>SDK Version</th><td>2.2.2-dev</td></tr>
                    </table>
                </div>
            </div>
        </div>
        <div class="col-md-6">
            <div class="card">
                <div class="card-header"><i class="bi bi-hdd-fill"></i> Storage</div>
                <div class="card-body">
                    <table class="table table-sm">
                        <tr><th>SPIFFS Total</th><td>1048576 bytes</td></tr>
                        <tr><th>SPIFFS Used</th><td>24576 bytes (2%)</td></tr>
                        <tr><th>SPIFFS Free</th><td>1024000 bytes</td></tr>
                    </table>
                    <div class="progress mt-3" style="height:25px;">
                        <div class="progress-bar" style="width:2%;background:var(--primary-gradient);">2% Used</div>
                    </div>
                </div>
            </div>
        </div>
    </div>
    """
    return render_template_string(BASE_HTML, title="Device Stats", active="stats", content=content, version=VERSION, author=AUTHOR, email=EMAIL)

@app.route('/about')
def about():
    content = f"""
    <div class="row mb-4">
        <div class="col-12"><h2><i class="bi bi-info-circle-fill"></i> About ESP SecMonitor</h2></div>
    </div>
    <div class="card mb-4">
        <div class="card-header"><i class="bi bi-code-slash"></i> Project Information</div>
        <div class="card-body">
            <h4>ESP8266 Power Management + WiFi Security Testing Framework</h4>
            <p class="lead">Version: <strong>{VERSION}</strong></p>
            <p>A comprehensive ESP8266-based system combining power monitoring with WiFi security testing capabilities.</p>
            <h5 class="mt-4"><i class="bi bi-lightning-fill"></i> Power Management Features</h5>
            <ul>
                <li>Automatic power ON/OFF event detection and logging</li>
                <li>Precise timestamp tracking with NTP synchronization</li>
                <li>Duration calculation for power outages</li>
                <li>Statistics: Today, 7-day, 15-day, and monthly analysis</li>
            </ul>
            <h5 class="mt-4"><i class="bi bi-shield-lock-fill"></i> WiFi Security Features</h5>
            <ul>
                <li>WiFi network scanning with hidden network detection</li>
                <li>WPS vulnerability detection based on 576+ device signatures</li>
                <li>Manufacturer identification using OUI database</li>
                <li>WiFi standard detection (WiFi 4/5/6)</li>
            </ul>
        </div>
    </div>
    <div class="card">
        <div class="card-header"><i class="bi bi-people-fill"></i> Credits</div>
        <div class="card-body">
            <table class="table">
                <tr><th>Author</th><td>{AUTHOR} (<a href="mailto:{EMAIL}">{EMAIL}</a>)</td></tr>
                <tr><th>Based On</th><td>
                    <a href="https://github.com/anbuinfosec/espPowerManagement" target="_blank">espPowerManagement</a> by @anbuinfosec<br>
                    <a href="https://github.com/GazaOS/FARHAN-Shot-v2" target="_blank">FARHAN-Shot-v2</a> by GazaOS
                </td></tr>
                <tr><th>License</th><td>MIT License</td></tr>
            </table>
        </div>
    </div>
    """
    return render_template_string(BASE_HTML, title="About", active="about", content=content, version=VERSION, author=AUTHOR, email=EMAIL)

@app.route('/api/scan')
def api_scan():
    return jsonify({
        "networks": SAMPLE_NETWORKS,
        "count": len(SAMPLE_NETWORKS),
        "timestamp": get_time_string(time.time())
    })

@app.route('/api/stats')
def api_stats():
    return jsonify({
        "uptime": int(time.time() - BOOT_TIME),
        "uptime_formatted": format_duration(time.time() - BOOT_TIME),
        "free_heap": 35420,
        "chip_id": "abc123",
        "networks_scanned": len(SAMPLE_NETWORKS),
        "power": {
            "today_off": 2700,
            "today_on": int(time.time() - BOOT_TIME),
            "week_off": 10800,
            "week_on": 594000,
        }
    })

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=False)
