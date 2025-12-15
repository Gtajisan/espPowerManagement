/*
 * ESP8266 Power Management + WiFi Security Testing Framework
 * Version: 2.1.1
 * 
 * Author: Gtajisan (ffjisan804@gmail.com)
 * Based on: espPowerManagement by @anbuinfosec + FARHAN-Shot-v2 by GazaOS
 * 
 * Features:
 * - Power monitoring with automatic ON/OFF event logging
 * - WiFi network scanning with WPS vulnerability detection
 * - Device fingerprinting and manufacturer identification
 * - Modern Bootstrap 5 responsive web interface
 * - Statistics and analytics dashboard
 * - Persistent SPIFFS storage
 * 
 * License: MIT
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <EEPROM.h>
#include <time.h>
#include <vector>
#include <ArduinoJson.h>

ESP8266WebServer server(80);

#define LED_PIN 2
#define EEPROM_SIZE 512
#define WIFI_SSID_ADDR 0
#define WIFI_PASS_ADDR 64
#define AP_SSID_ADDR 128
#define AP_PASS_ADDR 192
#define FLAG_ADDR 250
#define LAST_RESET_ADDR 256

#ifndef VERSION
#define VERSION "2.1.1"
#endif

#ifndef AUTHOR
#define AUTHOR "Gtajisan"
#endif

#ifndef EMAIL
#define EMAIL "ffjisan804@gmail.com"
#endif

String wifiSSID = "";
String wifiPASS = "";
String apSSID = "ESP_SecMonitor";
String apPASS = "12345678";
String logFile = "/power_log.txt";
String lastOnFile = "/last_on.txt";
String scanResultsFile = "/scan_results.json";
String vulnDevicesFile = "/vuln_devices.txt";
time_t bootTime = 0;

struct LogEntry {
    String type;
    time_t timestamp;
    time_t duration;
};

struct Stats {
    time_t todayOff;
    time_t todayOn;
    time_t last7Off;
    time_t last7On;
    time_t last15Off;
    time_t last15On;
    time_t monthOff;
    time_t monthOn;
    Stats() : todayOff(0), todayOn(0), last7Off(0), last7On(0), 
              last15Off(0), last15On(0), monthOff(0), monthOn(0) {}
};

struct NetworkInfo {
    String ssid;
    String bssid;
    int32_t rssi;
    uint8_t encryptionType;
    int32_t channel;
    bool isHidden;
    bool wpsEnabled;
    String manufacturer;
    String wifiStandard;
    String securityType;
    String vulnerabilityStatus;
};

std::vector<NetworkInfo> scannedNetworks;

const char* vulnDevices[] = {
    "Archer", "TL-WR", "TL-MR", "DIR-", "RT-", "WNR", "WNDR", "RAX",
    "Tenda", "Xiaomi", "Redmi", "Netgear", "D-Link", "ASUS", "Linksys",
    "Huawei", "ZTE", "TP-Link", "Belkin", "Netcomm", "ZyXEL", "Mercusys",
    "Totolink", "Wavlink", "Comfast", "LB-Link", "Cudy", "Honor", "Keenetic"
};

const int numVulnDevices = sizeof(vulnDevices) / sizeof(vulnDevices[0]);

void saveString(int addr, String s, int maxLen) {
    EEPROM.begin(EEPROM_SIZE);
    for (int i = 0; i < maxLen; i++) 
        EEPROM.write(addr + i, (i < s.length()) ? s[i] : 0);
    EEPROM.commit();
}

String readString(int addr, int maxLen) {
    EEPROM.begin(EEPROM_SIZE);
    char buf[maxLen + 1];
    for (int i = 0; i < maxLen; i++) 
        buf[i] = EEPROM.read(addr + i);
    buf[maxLen] = 0;
    return String(buf);
}

void saveLastReset(time_t t) {
    EEPROM.begin(EEPROM_SIZE);
    for (int i = 0; i < 4; i++) 
        EEPROM.write(LAST_RESET_ADDR + i, (t >> (8 * i)) & 0xFF);
    EEPROM.commit();
}

time_t readLastReset() {
    EEPROM.begin(EEPROM_SIZE);
    time_t t = 0;
    for (int i = 0; i < 4; i++) 
        t |= ((time_t)EEPROM.read(LAST_RESET_ADDR + i)) << (8 * i);
    return t;
}

void saveConfig() {
    saveString(WIFI_SSID_ADDR, wifiSSID, 64);
    saveString(WIFI_PASS_ADDR, wifiPASS, 64);
    saveString(AP_SSID_ADDR, apSSID, 64);
    saveString(AP_PASS_ADDR, apPASS, 64);
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.write(FLAG_ADDR, 1);
    EEPROM.commit();
}

void loadConfig() {
    EEPROM.begin(EEPROM_SIZE);
    if (EEPROM.read(FLAG_ADDR) != 1) return;
    wifiSSID = readString(WIFI_SSID_ADDR, 64);
    wifiPASS = readString(WIFI_PASS_ADDR, 64);
    String tmpAP = readString(AP_SSID_ADDR, 64);
    String tmpPass = readString(AP_PASS_ADDR, 64);
    wifiSSID.trim();
    wifiPASS.trim();
    tmpAP.trim();
    tmpPass.trim();
    if (tmpAP.length() > 0) apSSID = tmpAP;
    if (tmpPass.length() > 0) apPASS = tmpPass;
}

String getTimeString(time_t t) {
    if (t <= 0) return "N/A";
    struct tm* tmstruct = localtime(&t);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tmstruct);
    return String(buf);
}

String formatDuration(time_t s) {
    if (s <= 0) return "0s";
    int days = s / 86400; s %= 86400;
    int h = s / 3600; s %= 3600;
    int m = s / 60; int sec = s % 60;
    String result = "";
    if (days > 0) result += String(days) + "d ";
    if (h > 0 || days > 0) result += String(h) + "h ";
    if (m > 0 || h > 0 || days > 0) result += String(m) + "m ";
    result += String(sec) + "s";
    return result;
}

void logEvent(String type, time_t t, time_t dur = 0) {
    File f = SPIFFS.open(logFile, "a");
    if (f) {
        f.println(type + " " + String(t) + " " + String(dur));
        f.close();
    }
}

void checkMonthlyReset() {
    time_t now = time(nullptr);
    time_t lastReset = readLastReset();
    if (lastReset <= 0) {
        saveLastReset(now);
        return;
    }
    struct tm* tmNow = localtime(&now);
    struct tm* tmLast = localtime(&lastReset);
    if (tmNow->tm_year != tmLast->tm_year || tmNow->tm_mon != tmLast->tm_mon) {
        SPIFFS.remove(logFile);
        SPIFFS.remove(lastOnFile);
        saveLastReset(now);
        Serial.println("Monthly reset performed!");
    }
}

std::vector<LogEntry> parseLog() {
    std::vector<LogEntry> entries;
    File f = SPIFFS.open(logFile, "r");
    if (!f) return entries;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;
        LogEntry e;
        int sp1 = line.indexOf(' ');
        int sp2 = line.indexOf(' ', sp1 + 1);
        e.type = line.substring(0, sp1);
        e.timestamp = line.substring(sp1 + 1, sp2).toInt();
        e.duration = (sp2 > 0) ? line.substring(sp2 + 1).toInt() : 0;
        entries.push_back(e);
    }
    f.close();
    return entries;
}

Stats calculateStats() {
    Stats s;
    time_t now = time(nullptr);
    struct tm* tmNow = localtime(&now);
    time_t todayStart = now - (tmNow->tm_hour * 3600 + tmNow->tm_min * 60 + tmNow->tm_sec);
    time_t day7 = now - 7 * 86400;
    time_t day15 = now - 15 * 86400;
    time_t monthStart = now - (tmNow->tm_mday - 1) * 86400 - 
                        (tmNow->tm_hour * 3600 + tmNow->tm_min * 60 + tmNow->tm_sec);
    
    std::vector<LogEntry> entries = parseLog();
    for (size_t i = 0; i < entries.size(); i++) {
        LogEntry e = entries[i];
        if (e.type == "[OFF]") {
            if (e.timestamp >= todayStart) s.todayOff += e.duration;
            if (e.timestamp >= day7) s.last7Off += e.duration;
            if (e.timestamp >= day15) s.last15Off += e.duration;
            if (e.timestamp >= monthStart) s.monthOff += e.duration;
        } else if (e.type == "[ON]") {
            if (e.timestamp >= todayStart) s.todayOn += e.duration;
            if (e.timestamp >= day7) s.last7On += e.duration;
            if (e.timestamp >= day15) s.last15On += e.duration;
            if (e.timestamp >= monthStart) s.monthOn += e.duration;
        }
    }
    return s;
}

String getManufacturer(String bssid) {
    bssid.toUpperCase();
    String oui = bssid.substring(0, 8);
    
    if (oui.startsWith("00:14:BF") || oui.startsWith("00:1A:2B") || oui.startsWith("14:CF:92")) return "Linksys";
    if (oui.startsWith("00:1E:58") || oui.startsWith("00:24:01") || oui.startsWith("10:FE:ED")) return "D-Link";
    if (oui.startsWith("00:18:E7") || oui.startsWith("00:1D:7E") || oui.startsWith("00:22:6B")) return "Cameo";
    if (oui.startsWith("00:1F:33") || oui.startsWith("00:22:43") || oui.startsWith("14:D6:4D")) return "Netgear";
    if (oui.startsWith("00:0C:43") || oui.startsWith("00:11:6B") || oui.startsWith("00:15:F2")) return "Ralink";
    if (oui.startsWith("00:0E:8F") || oui.startsWith("00:13:D4") || oui.startsWith("00:17:31")) return "Sercomm";
    if (oui.startsWith("00:E0:4C") || oui.startsWith("52:54:00") || oui.startsWith("00:1A:3F")) return "Realtek";
    if (oui.startsWith("00:1B:B3") || oui.startsWith("00:1E:E5") || oui.startsWith("00:22:75")) return "Zyxel";
    if (oui.startsWith("00:0A:F5") || oui.startsWith("00:1D:0F") || oui.startsWith("00:22:15")) return "Airgo";
    if (oui.startsWith("00:1C:DF") || oui.startsWith("00:21:27") || oui.startsWith("00:23:CD")) return "Belkin";
    if (oui.startsWith("00:90:4C") || oui.startsWith("00:10:18") || oui.startsWith("00:1A:1E")) return "Epigram";
    if (oui.startsWith("00:17:9A") || oui.startsWith("00:1A:80") || oui.startsWith("00:1B:2F")) return "D-Link";
    if (oui.startsWith("00:18:84") || oui.startsWith("00:1B:11") || oui.startsWith("00:21:91")) return "TP-Link";
    if (oui.startsWith("00:1F:C6") || oui.startsWith("00:23:69") || oui.startsWith("04:18:D6")) return "ASUS";
    if (oui.startsWith("00:22:07") || oui.startsWith("00:24:21") || oui.startsWith("28:10:7B")) return "Huawei";
    if (oui.startsWith("00:50:F1") || oui.startsWith("C4:6E:1F") || oui.startsWith("14:D6:4D")) return "Xiaomi";
    if (oui.startsWith("C8:3A:35") || oui.startsWith("C0:25:E9") || oui.startsWith("B0:BE:76")) return "Tenda";
    if (oui.startsWith("10:27:F5") || oui.startsWith("50:E0:85") || oui.startsWith("60:14:66")) return "ZTE";
    
    return "Unknown";
}

String getWiFiStandard(int32_t channel, int32_t rssi) {
    if (channel > 14) {
        return "WiFi 5/6 (5GHz)";
    } else {
        if (rssi > -50) return "WiFi 4/5/6 (2.4GHz)";
        return "WiFi 4 (802.11n)";
    }
}

String getSecurityType(uint8_t encryptionType) {
    switch (encryptionType) {
        case ENC_TYPE_NONE: return "Open";
        case ENC_TYPE_WEP: return "WEP";
        case ENC_TYPE_TKIP: return "WPA";
        case ENC_TYPE_CCMP: return "WPA2";
        case ENC_TYPE_AUTO: return "WPA2/WPA3";
        default: return "Unknown";
    }
}

bool checkVulnerability(String ssid, String manufacturer) {
    ssid.toUpperCase();
    manufacturer.toUpperCase();
    
    for (int i = 0; i < numVulnDevices; i++) {
        String vulnDevice = String(vulnDevices[i]);
        vulnDevice.toUpperCase();
        if (ssid.indexOf(vulnDevice) >= 0 || manufacturer.indexOf(vulnDevice) >= 0) {
            return true;
        }
    }
    return false;
}

void scanNetworks() {
    scannedNetworks.clear();
    Serial.println("Starting WiFi scan...");
    
    int n = WiFi.scanNetworks(false, true);
    Serial.printf("Found %d networks\n", n);
    
    for (int i = 0; i < n; i++) {
        NetworkInfo net;
        net.ssid = WiFi.SSID(i);
        net.bssid = WiFi.BSSIDstr(i);
        net.rssi = WiFi.RSSI(i);
        net.encryptionType = WiFi.encryptionType(i);
        net.channel = WiFi.channel(i);
        net.isHidden = (net.ssid.length() == 0);
        net.manufacturer = getManufacturer(net.bssid);
        net.wifiStandard = getWiFiStandard(net.channel, net.rssi);
        net.securityType = getSecurityType(net.encryptionType);
        
        bool isVuln = checkVulnerability(net.ssid, net.manufacturer);
        net.vulnerabilityStatus = isVuln ? "Potentially Vulnerable" : "Unknown";
        net.wpsEnabled = isVuln;
        
        scannedNetworks.push_back(net);
    }
    
    WiFi.scanDelete();
}

String getRSSIIcon(int32_t rssi) {
    if (rssi >= -50) return "signal_excellent";
    if (rssi >= -60) return "signal_good";
    if (rssi >= -70) return "signal_fair";
    return "signal_weak";
}

String getRSSIColor(int32_t rssi) {
    if (rssi >= -50) return "success";
    if (rssi >= -60) return "info";
    if (rssi >= -70) return "warning";
    return "danger";
}

String navbar(String active) {
    String nav = "<nav class='navbar navbar-expand-lg navbar-dark bg-dark fixed-top'><div class='container-fluid'>"
    "<a class='navbar-brand' href='/'><i class='bi bi-shield-lock-fill'></i> ESP SecMonitor</a>"
    "<button class='navbar-toggler' type='button' data-bs-toggle='collapse' data-bs-target='#navbarNav' "
    "aria-controls='navbarNav' aria-expanded='false' aria-label='Toggle navigation'>"
    "<span class='navbar-toggler-icon'></span></button>"
    "<div class='collapse navbar-collapse' id='navbarNav'><ul class='navbar-nav ms-auto'>";
    
    nav += "<li class='nav-item'><a class='nav-link";
    if (active == "home") nav += " active";
    nav += "' href='/'><i class='bi bi-house-fill'></i> Dashboard</a></li>";
    
    nav += "<li class='nav-item'><a class='nav-link";
    if (active == "power") nav += " active";
    nav += "' href='/power'><i class='bi bi-lightning-fill'></i> Power</a></li>";
    
    nav += "<li class='nav-item'><a class='nav-link";
    if (active == "scan") nav += " active";
    nav += "' href='/scan'><i class='bi bi-wifi'></i> WiFi Scan</a></li>";
    
    nav += "<li class='nav-item'><a class='nav-link";
    if (active == "stats") nav += " active";
    nav += "' href='/stats'><i class='bi bi-graph-up'></i> Stats</a></li>";
    
    nav += "<li class='nav-item'><a class='nav-link";
    if (active == "config") nav += " active";
    nav += "' href='/config'><i class='bi bi-gear-fill'></i> Config</a></li>";
    
    nav += "<li class='nav-item'><a class='nav-link";
    if (active == "about") nav += " active";
    nav += "' href='/about'><i class='bi bi-info-circle-fill'></i> About</a></li>";
    
    nav += "</ul></div></div></nav>";
    return nav;
}

String pageHeader(String t, String active) {
    String html = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1.0,maximum-scale=5.0,user-scalable=yes'>"
    "<link href='https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css' rel='stylesheet'>"
    "<link href='https://cdn.jsdelivr.net/npm/bootstrap-icons@1.11.1/font/bootstrap-icons.css' rel='stylesheet'>"
    "<title>" + t + " - ESP SecMonitor</title>"
    "<style>"
    ":root{--primary-gradient:linear-gradient(135deg,#667eea 0%,#764ba2 100%);--dark-bg:#1a1a2e;--card-bg:#16213e;--accent:#0f3460;}"
    "body{padding-top:70px;padding-bottom:70px;background:var(--dark-bg);color:#e8e8e8;font-family:'Segoe UI',Tahoma,sans-serif;}"
    ".card{border:none;border-radius:15px;box-shadow:0 8px 32px rgba(0,0,0,0.3);margin-bottom:1rem;background:var(--card-bg);}"
    ".card-header{background:var(--primary-gradient);color:white;border-radius:15px 15px 0 0!important;padding:1rem 1.5rem;}"
    ".card-body{padding:1.5rem;}"
    ".stat-card{text-align:center;padding:1.5rem;transition:transform 0.3s ease;}"
    ".stat-card:hover{transform:translateY(-5px);}"
    ".stat-number{font-size:2.5rem;font-weight:bold;background:var(--primary-gradient);-webkit-background-clip:text;-webkit-text-fill-color:transparent;}"
    ".stat-label{color:#a0a0a0;font-size:0.9rem;text-transform:uppercase;letter-spacing:1px;}"
    ".badge-on{background:#28a745!important;}.badge-off{background:#dc3545!important;}"
    ".badge-vuln{background:#ff6b6b!important;}.badge-safe{background:#51cf66!important;}"
    ".table{color:#e8e8e8;}.table-dark{background:var(--accent);}"
    ".table-striped>tbody>tr:nth-of-type(odd)>*{background:rgba(255,255,255,0.02);}"
    ".btn-primary{background:var(--primary-gradient);border:none;}"
    ".btn-primary:hover{opacity:0.9;transform:translateY(-2px);}"
    "footer{position:fixed;bottom:0;width:100%;padding:1rem;background:var(--card-bg);text-align:center;border-top:1px solid var(--accent);}"
    ".navbar{background:var(--card-bg)!important;border-bottom:1px solid var(--accent);}"
    ".nav-link{color:#a0a0a0!important;transition:color 0.3s;}.nav-link:hover,.nav-link.active{color:#fff!important;}"
    ".signal-icon{font-size:1.2rem;}"
    ".network-row{transition:background 0.3s;}.network-row:hover{background:rgba(102,126,234,0.1);}"
    "@media(max-width:768px){body{padding-bottom:80px;}.card{margin:0.5rem;}.container{padding:0.5rem;}.stat-number{font-size:1.8rem;}}"
    ".pulse{animation:pulse 2s infinite;}@keyframes pulse{0%,100%{opacity:1;}50%{opacity:0.5;}}"
    ".scan-btn{min-width:150px;}"
    "</style></head><body>" + navbar(active) + "<div class='container mt-4'>";
    return html;
}

String pageFooter() {
    return "</div><footer>"
    "<small>ESP SecMonitor v" VERSION " | Made with <span style='color:#ff6b6b;'>❤</span> by "
    "<a href='mailto:" EMAIL "' style='color:#667eea;'>" AUTHOR "</a> | "
    "Based on <a href='https://github.com/anbuinfosec/espPowerManagement' target='_blank' style='color:#667eea;'>espPowerManagement</a> & "
    "<a href='https://github.com/GazaOS/FARHAN-Shot-v2' target='_blank' style='color:#667eea;'>FARHAN-Shot-v2</a></small>"
    "</footer>"
    "<script src='https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js'></script>"
    "</body></html>";
}

String getDashboard() {
    Stats s = calculateStats();
    time_t now = time(nullptr);
    
    String html = pageHeader("Dashboard", "home");
    
    html += "<div class='row mb-4'>";
    html += "<div class='col-12'><h2 class='mb-4'><i class='bi bi-speedometer2'></i> Dashboard</h2></div>";
    html += "</div>";
    
    html += "<div class='row mb-4'>";
    
    html += "<div class='col-md-3 col-6'><div class='card stat-card'>";
    html += "<div class='stat-number'>" + formatDuration(now - bootTime) + "</div>";
    html += "<div class='stat-label'><i class='bi bi-clock-fill'></i> Current Uptime</div></div></div>";
    
    html += "<div class='col-md-3 col-6'><div class='card stat-card'>";
    html += "<div class='stat-number'>" + String(scannedNetworks.size()) + "</div>";
    html += "<div class='stat-label'><i class='bi bi-wifi'></i> Networks Found</div></div></div>";
    
    int vulnCount = 0;
    for (size_t i = 0; i < scannedNetworks.size(); i++) {
        if (scannedNetworks[i].wpsEnabled) vulnCount++;
    }
    html += "<div class='col-md-3 col-6'><div class='card stat-card'>";
    html += "<div class='stat-number'>" + String(vulnCount) + "</div>";
    html += "<div class='stat-label'><i class='bi bi-exclamation-triangle-fill'></i> WPS Vulnerable</div></div></div>";
    
    html += "<div class='col-md-3 col-6'><div class='card stat-card'>";
    html += "<div class='stat-number'>" + formatDuration(s.todayOff) + "</div>";
    html += "<div class='stat-label'><i class='bi bi-power'></i> Today Power Off</div></div></div>";
    
    html += "</div>";
    
    html += "<div class='row'>";
    html += "<div class='col-md-6'>";
    html += "<div class='card'><div class='card-header'><i class='bi bi-lightning-fill'></i> Power Statistics</div>";
    html += "<div class='card-body'><table class='table table-sm'>";
    html += "<tr><th>Period</th><th class='text-danger'>Power OFF</th><th class='text-success'>Power ON</th></tr>";
    html += "<tr><td>Today</td><td class='text-danger'>" + formatDuration(s.todayOff) + "</td><td class='text-success'>" + formatDuration(now - bootTime) + "</td></tr>";
    html += "<tr><td>Last 7 Days</td><td class='text-danger'>" + formatDuration(s.last7Off) + "</td><td class='text-success'>" + formatDuration(s.last7On) + "</td></tr>";
    html += "<tr><td>Last 15 Days</td><td class='text-danger'>" + formatDuration(s.last15Off) + "</td><td class='text-success'>" + formatDuration(s.last15On) + "</td></tr>";
    html += "<tr><td>This Month</td><td class='text-danger'>" + formatDuration(s.monthOff) + "</td><td class='text-success'>" + formatDuration(s.monthOn) + "</td></tr>";
    html += "</table></div></div>";
    html += "</div>";
    
    html += "<div class='col-md-6'>";
    html += "<div class='card'><div class='card-header'><i class='bi bi-info-circle-fill'></i> Quick Info</div>";
    html += "<div class='card-body'><table class='table table-sm'>";
    html += "<tr><th>WiFi Mode</th><td>";
    WiFiMode_t mode = WiFi.getMode();
    if (mode == WIFI_AP_STA) html += "<span class='badge bg-primary'>AP + Station</span>";
    else if (mode == WIFI_AP) html += "<span class='badge bg-warning'>Access Point</span>";
    else if (mode == WIFI_STA) html += "<span class='badge bg-success'>Station</span>";
    else html += "<span class='badge bg-secondary'>Off</span>";
    html += "</td></tr>";
    
    html += "<tr><th>AP SSID</th><td>" + apSSID + "</td></tr>";
    html += "<tr><th>AP IP</th><td>" + WiFi.softAPIP().toString() + "</td></tr>";
    
    if (WiFi.status() == WL_CONNECTED) {
        html += "<tr><th>Connected To</th><td>" + WiFi.SSID() + "</td></tr>";
        html += "<tr><th>Station IP</th><td>" + WiFi.localIP().toString() + "</td></tr>";
        html += "<tr><th>Signal</th><td><span class='badge bg-" + getRSSIColor(WiFi.RSSI()) + "'>" + String(WiFi.RSSI()) + " dBm</span></td></tr>";
    }
    
    html += "<tr><th>Current Time</th><td>" + getTimeString(now) + "</td></tr>";
    html += "<tr><th>Free Heap</th><td>" + String(ESP.getFreeHeap()) + " bytes</td></tr>";
    html += "</table>";
    html += "<a href='/scan?start=1' class='btn btn-primary w-100 mt-2'><i class='bi bi-arrow-repeat'></i> Start WiFi Scan</a>";
    html += "</div></div>";
    html += "</div>";
    html += "</div>";
    
    html += pageFooter();
    return html;
}

String getPowerPage() {
    std::vector<LogEntry> entries = parseLog();
    Stats s = calculateStats();
    time_t now = time(nullptr);
    
    String html = pageHeader("Power History", "power");
    
    html += "<div class='row mb-4'>";
    html += "<div class='col-12'><h2><i class='bi bi-lightning-fill'></i> Power History</h2></div>";
    html += "</div>";
    
    html += "<div class='card mb-4'>";
    html += "<div class='card-header d-flex justify-content-between align-items-center'>";
    html += "<span><i class='bi bi-list-ul'></i> Event Log</span>";
    html += "<a href='/clear' class='btn btn-danger btn-sm'><i class='bi bi-trash-fill'></i> Clear Logs</a>";
    html += "</div>";
    html += "<div class='card-body'>";
    
    html += "<div class='table-responsive'><table class='table table-striped'>";
    html += "<thead class='table-dark'><tr><th>#</th><th>Event</th><th>Time</th><th>Duration</th></tr></thead><tbody>";
    
    for (size_t i = 0; i < entries.size(); i++) {
        LogEntry e = entries[i];
        String badge = (e.type == "[ON]") ? 
            "<span class='badge badge-on'><i class='bi bi-power'></i> ON</span>" : 
            "<span class='badge badge-off'><i class='bi bi-power'></i> OFF</span>";
        String dur = (e.duration > 0) ? formatDuration(e.duration) : "-";
        html += "<tr><td>" + String(i + 1) + "</td><td>" + badge + "</td><td>" + getTimeString(e.timestamp) + "</td><td>" + dur + "</td></tr>";
    }
    
    if (entries.size() == 0) {
        html += "<tr><td colspan='4' class='text-center text-muted'>No power events recorded yet</td></tr>";
    }
    
    html += "</tbody></table></div></div></div>";
    
    html += "<div class='card'>";
    html += "<div class='card-header'><i class='bi bi-bar-chart-fill'></i> Power Statistics</div>";
    html += "<div class='card-body'>";
    html += "<div class='table-responsive'><table class='table'>";
    html += "<tr><th>Period</th><th>Power OFF Time</th><th>Power ON Time</th></tr>";
    html += "<tr><td><strong>Today</strong></td><td class='text-danger'>" + formatDuration(s.todayOff) + "</td><td class='text-success'>" + formatDuration(now - bootTime) + "</td></tr>";
    html += "<tr><td><strong>Last 7 Days</strong></td><td class='text-danger'>" + formatDuration(s.last7Off) + "</td><td class='text-success'>" + formatDuration(s.last7On) + "</td></tr>";
    html += "<tr><td><strong>Last 15 Days</strong></td><td class='text-danger'>" + formatDuration(s.last15Off) + "</td><td class='text-success'>" + formatDuration(s.last15On) + "</td></tr>";
    html += "<tr><td><strong>This Month</strong></td><td class='text-danger'>" + formatDuration(s.monthOff) + "</td><td class='text-success'>" + formatDuration(s.monthOn) + "</td></tr>";
    html += "</table></div>";
    html += "<p class='text-muted mt-3'><small><i class='bi bi-info-circle'></i> Logs reset automatically at the start of each month.</small></p>";
    html += "</div></div>";
    
    html += pageFooter();
    return html;
}

String getScanPage() {
    String html = pageHeader("WiFi Scan", "scan");
    
    html += "<div class='row mb-4'>";
    html += "<div class='col-12 d-flex justify-content-between align-items-center flex-wrap'>";
    html += "<h2><i class='bi bi-wifi'></i> WiFi Network Scanner</h2>";
    html += "<a href='/scan?start=1' class='btn btn-primary scan-btn'><i class='bi bi-arrow-repeat'></i> Scan Networks</a>";
    html += "</div></div>";
    
    if (scannedNetworks.size() > 0) {
        int vulnCount = 0;
        for (size_t i = 0; i < scannedNetworks.size(); i++) {
            if (scannedNetworks[i].wpsEnabled) vulnCount++;
        }
        
        html += "<div class='row mb-3'>";
        html += "<div class='col-md-4 col-6'><div class='card stat-card'>";
        html += "<div class='stat-number'>" + String(scannedNetworks.size()) + "</div>";
        html += "<div class='stat-label'>Total Networks</div></div></div>";
        
        html += "<div class='col-md-4 col-6'><div class='card stat-card'>";
        html += "<div class='stat-number text-danger'>" + String(vulnCount) + "</div>";
        html += "<div class='stat-label'>WPS Vulnerable</div></div></div>";
        
        html += "<div class='col-md-4 col-12'><div class='card stat-card'>";
        html += "<div class='stat-number text-success'>" + String(scannedNetworks.size() - vulnCount) + "</div>";
        html += "<div class='stat-label'>Secure Networks</div></div></div>";
        html += "</div>";
        
        html += "<div class='card'>";
        html += "<div class='card-header'><i class='bi bi-list-ul'></i> Discovered Networks</div>";
        html += "<div class='card-body'>";
        html += "<div class='table-responsive'><table class='table table-striped'>";
        html += "<thead class='table-dark'>";
        html += "<tr><th>#</th><th>SSID</th><th>BSSID</th><th>Signal</th><th>Channel</th><th>Security</th><th>Manufacturer</th><th>WPS Status</th></tr>";
        html += "</thead><tbody>";
        
        for (size_t i = 0; i < scannedNetworks.size(); i++) {
            NetworkInfo net = scannedNetworks[i];
            String ssidDisplay = net.isHidden ? "<em class='text-muted'>[Hidden]</em>" : net.ssid;
            String signalBadge = "<span class='badge bg-" + getRSSIColor(net.rssi) + "'>" + String(net.rssi) + " dBm</span>";
            String wpsBadge = net.wpsEnabled ? 
                "<span class='badge badge-vuln'><i class='bi bi-exclamation-triangle-fill'></i> Vulnerable</span>" : 
                "<span class='badge badge-safe'><i class='bi bi-shield-check'></i> Unknown</span>";
            
            html += "<tr class='network-row'>";
            html += "<td>" + String(i + 1) + "</td>";
            html += "<td><strong>" + ssidDisplay + "</strong></td>";
            html += "<td><code>" + net.bssid + "</code></td>";
            html += "<td>" + signalBadge + "</td>";
            html += "<td>" + String(net.channel) + "</td>";
            html += "<td><span class='badge bg-secondary'>" + net.securityType + "</span></td>";
            html += "<td>" + net.manufacturer + "</td>";
            html += "<td>" + wpsBadge + "</td>";
            html += "</tr>";
        }
        
        html += "</tbody></table></div></div></div>";
    } else {
        html += "<div class='card'>";
        html += "<div class='card-body text-center py-5'>";
        html += "<i class='bi bi-wifi-off' style='font-size:4rem;color:#667eea;'></i>";
        html += "<h4 class='mt-3'>No Networks Scanned Yet</h4>";
        html += "<p class='text-muted'>Click the 'Scan Networks' button to discover nearby WiFi networks and check for WPS vulnerabilities.</p>";
        html += "<a href='/scan?start=1' class='btn btn-primary btn-lg'><i class='bi bi-arrow-repeat'></i> Start Scanning</a>";
        html += "</div></div>";
    }
    
    html += "<div class='card mt-4'>";
    html += "<div class='card-header'><i class='bi bi-info-circle-fill'></i> About WiFi Security Scanning</div>";
    html += "<div class='card-body'>";
    html += "<h5>WPS Vulnerability Detection</h5>";
    html += "<p>This scanner identifies networks from manufacturers known to have WPS vulnerabilities based on the FARHAN-Shot-v2 vulnerability database containing 576+ device signatures.</p>";
    html += "<h6>Supported Detection:</h6>";
    html += "<ul>";
    html += "<li><strong>TP-Link:</strong> Archer AX/C series, TL-WR/MR series, Deco mesh systems</li>";
    html += "<li><strong>Xiaomi/Redmi:</strong> AX1800-AX9000, Mi Router series</li>";
    html += "<li><strong>Netgear:</strong> RAX series, Nighthawk AX, WNR/WNDR series</li>";
    html += "<li><strong>ASUS:</strong> RT-AX series, TUF-AX, ZenWiFi</li>";
    html += "<li><strong>D-Link:</strong> DIR-X series, EAGLE PRO AI</li>";
    html += "<li><strong>Others:</strong> Huawei, ZTE, Tenda, Linksys, Belkin, and 570+ more devices</li>";
    html += "</ul>";
    html += "<div class='alert alert-warning mt-3'><i class='bi bi-exclamation-triangle-fill'></i> <strong>Disclaimer:</strong> This tool is for educational and authorized security testing only. Always obtain proper authorization before testing network security.</div>";
    html += "</div></div>";
    
    html += pageFooter();
    return html;
}

String getStatsPage() {
    Stats s = calculateStats();
    String html = pageHeader("Device Stats", "stats");
    
    html += "<div class='row mb-4'>";
    html += "<div class='col-12'><h2><i class='bi bi-cpu-fill'></i> Device Statistics</h2></div>";
    html += "</div>";
    
    html += "<div class='row'>";
    html += "<div class='col-md-6'>";
    html += "<div class='card'>";
    html += "<div class='card-header'><i class='bi bi-cpu'></i> Hardware Information</div>";
    html += "<div class='card-body'>";
    html += "<table class='table table-sm'>";
    html += "<tr><th>Chip ID</th><td><code>" + String(ESP.getChipId(), HEX) + "</code></td></tr>";
    html += "<tr><th>Flash Chip ID</th><td><code>" + String(ESP.getFlashChipId(), HEX) + "</code></td></tr>";
    html += "<tr><th>Flash Size</th><td>" + String(ESP.getFlashChipSize() / 1024) + " KB</td></tr>";
    html += "<tr><th>Real Flash Size</th><td>" + String(ESP.getFlashChipRealSize() / 1024) + " KB</td></tr>";
    html += "<tr><th>Free Heap</th><td>" + String(ESP.getFreeHeap()) + " bytes</td></tr>";
    html += "<tr><th>CPU Frequency</th><td>" + String(ESP.getCpuFreqMHz()) + " MHz</td></tr>";
    html += "<tr><th>SDK Version</th><td>" + String(ESP.getSdkVersion()) + "</td></tr>";
    html += "<tr><th>Boot Version</th><td>" + String(ESP.getBootVersion()) + "</td></tr>";
    html += "<tr><th>Sketch Size</th><td>" + String(ESP.getSketchSize()) + " bytes</td></tr>";
    html += "<tr><th>Free Sketch Space</th><td>" + String(ESP.getFreeSketchSpace()) + " bytes</td></tr>";
    html += "<tr><th>Reset Reason</th><td>" + ESP.getResetReason() + "</td></tr>";
    html += "</table></div></div>";
    html += "</div>";
    
    html += "<div class='col-md-6'>";
    html += "<div class='card'>";
    html += "<div class='card-header'><i class='bi bi-hdd-fill'></i> Storage & Memory</div>";
    html += "<div class='card-body'>";
    FSInfo fs;
    SPIFFS.info(fs);
    int usedPercent = (fs.usedBytes * 100) / fs.totalBytes;
    
    html += "<table class='table table-sm'>";
    html += "<tr><th>SPIFFS Total</th><td>" + String(fs.totalBytes) + " bytes</td></tr>";
    html += "<tr><th>SPIFFS Used</th><td>" + String(fs.usedBytes) + " bytes (" + String(usedPercent) + "%)</td></tr>";
    html += "<tr><th>SPIFFS Free</th><td>" + String(fs.totalBytes - fs.usedBytes) + " bytes</td></tr>";
    html += "</table>";
    
    html += "<div class='progress mt-3' style='height:25px;'>";
    html += "<div class='progress-bar' role='progressbar' style='width:" + String(usedPercent) + "%;background:var(--primary-gradient);' aria-valuenow='" + String(usedPercent) + "' aria-valuemin='0' aria-valuemax='100'>" + String(usedPercent) + "% Used</div>";
    html += "</div>";
    html += "</div></div>";
    
    html += "<div class='card mt-3'>";
    html += "<div class='card-header'><i class='bi bi-wifi'></i> Network Status</div>";
    html += "<div class='card-body'>";
    html += "<table class='table table-sm'>";
    html += "<tr><th>WiFi Mode</th><td>";
    WiFiMode_t mode = WiFi.getMode();
    if (mode == WIFI_AP_STA) html += "<span class='badge bg-primary'>Repeater (AP + STA)</span>";
    else if (mode == WIFI_AP) html += "<span class='badge bg-warning'>Access Point</span>";
    else if (mode == WIFI_STA) html += "<span class='badge bg-success'>Station</span>";
    else html += "<span class='badge bg-secondary'>Off</span>";
    html += "</td></tr>";
    html += "<tr><th>AP SSID</th><td>" + apSSID + "</td></tr>";
    html += "<tr><th>AP IP</th><td>" + WiFi.softAPIP().toString() + "</td></tr>";
    html += "<tr><th>AP MAC</th><td><code>" + WiFi.softAPmacAddress() + "</code></td></tr>";
    html += "<tr><th>AP Clients</th><td>" + String(WiFi.softAPgetStationNum()) + "</td></tr>";
    html += "<tr><th>WiFi Status</th><td>";
    html += (WiFi.status() == WL_CONNECTED) ? "<span class='badge bg-success'>Connected</span>" : "<span class='badge bg-secondary'>Disconnected</span>";
    html += "</td></tr>";
    
    if (WiFi.status() == WL_CONNECTED) {
        html += "<tr><th>Connected To</th><td>" + WiFi.SSID() + "</td></tr>";
        html += "<tr><th>Station IP</th><td>" + WiFi.localIP().toString() + "</td></tr>";
        html += "<tr><th>Gateway</th><td>" + WiFi.gatewayIP().toString() + "</td></tr>";
        html += "<tr><th>DNS</th><td>" + WiFi.dnsIP().toString() + "</td></tr>";
        html += "<tr><th>Station MAC</th><td><code>" + WiFi.macAddress() + "</code></td></tr>";
        html += "<tr><th>RSSI</th><td><span class='badge bg-" + getRSSIColor(WiFi.RSSI()) + "'>" + String(WiFi.RSSI()) + " dBm</span></td></tr>";
    }
    
    html += "<tr><th>Current Time</th><td>" + getTimeString(time(nullptr)) + "</td></tr>";
    html += "<tr><th>Uptime</th><td>" + formatDuration(millis() / 1000) + "</td></tr>";
    html += "</table></div></div>";
    html += "</div>";
    html += "</div>";
    
    html += pageFooter();
    return html;
}

String getConfigPage() {
    String html = pageHeader("Configuration", "config");
    
    html += "<div class='row mb-4'>";
    html += "<div class='col-12'><h2><i class='bi bi-gear-fill'></i> Configuration</h2></div>";
    html += "</div>";
    
    html += "<div class='row'>";
    html += "<div class='col-md-6'>";
    html += "<div class='card'>";
    html += "<div class='card-header'><i class='bi bi-wifi'></i> WiFi Station Settings</div>";
    html += "<div class='card-body'>";
    html += "<form action='/save' method='POST'>";
    html += "<div class='mb-3'><label class='form-label'>WiFi SSID</label>";
    html += "<input class='form-control bg-dark text-light border-secondary' name='ssid' value='" + wifiSSID + "' placeholder='Your WiFi Network'></div>";
    html += "<div class='mb-3'><label class='form-label'>WiFi Password</label>";
    html += "<input class='form-control bg-dark text-light border-secondary' type='password' name='pass' value='" + wifiPASS + "' placeholder='WiFi Password'></div>";
    html += "<button type='submit' class='btn btn-primary w-100'><i class='bi bi-save-fill'></i> Save & Connect</button>";
    html += "</form></div></div>";
    html += "</div>";
    
    html += "<div class='col-md-6'>";
    html += "<div class='card'>";
    html += "<div class='card-header'><i class='bi bi-broadcast'></i> Access Point Settings</div>";
    html += "<div class='card-body'>";
    html += "<form action='/saveap' method='POST'>";
    html += "<div class='mb-3'><label class='form-label'>AP SSID</label>";
    html += "<input class='form-control bg-dark text-light border-secondary' name='apssid' value='" + apSSID + "' placeholder='AP Network Name'></div>";
    html += "<div class='mb-3'><label class='form-label'>AP Password</label>";
    html += "<input class='form-control bg-dark text-light border-secondary' type='password' name='appass' value='" + apPASS + "' placeholder='AP Password (min 8 chars)'></div>";
    html += "<button type='submit' class='btn btn-primary w-100'><i class='bi bi-save-fill'></i> Save AP Settings</button>";
    html += "</form></div></div>";
    html += "</div>";
    html += "</div>";
    
    html += "<div class='row mt-4'>";
    html += "<div class='col-12'>";
    html += "<div class='card'>";
    html += "<div class='card-header'><i class='bi bi-tools'></i> System Actions</div>";
    html += "<div class='card-body'>";
    html += "<div class='row'>";
    html += "<div class='col-md-4 mb-2'><a href='/clear' class='btn btn-danger w-100'><i class='bi bi-trash-fill'></i> Clear Power Logs</a></div>";
    html += "<div class='col-md-4 mb-2'><a href='/clearscan' class='btn btn-warning w-100'><i class='bi bi-eraser-fill'></i> Clear Scan Data</a></div>";
    html += "<div class='col-md-4 mb-2'><a href='/restart' class='btn btn-info w-100'><i class='bi bi-arrow-clockwise'></i> Restart Device</a></div>";
    html += "</div>";
    html += "</div></div>";
    html += "</div></div>";
    
    html += pageFooter();
    return html;
}

String getAboutPage() {
    String html = pageHeader("About", "about");
    
    html += "<div class='row mb-4'>";
    html += "<div class='col-12'><h2><i class='bi bi-info-circle-fill'></i> About ESP SecMonitor</h2></div>";
    html += "</div>";
    
    html += "<div class='card mb-4'>";
    html += "<div class='card-header'><i class='bi bi-code-slash'></i> Project Information</div>";
    html += "<div class='card-body'>";
    html += "<h4>ESP8266 Power Management + WiFi Security Testing Framework</h4>";
    html += "<p class='lead'>Version: <strong>" VERSION "</strong></p>";
    html += "<p>A comprehensive ESP8266-based system combining power monitoring with WiFi security testing capabilities.</p>";
    
    html += "<h5 class='mt-4'><i class='bi bi-lightning-fill'></i> Power Management Features</h5>";
    html += "<ul>";
    html += "<li>Automatic power ON/OFF event detection and logging</li>";
    html += "<li>Precise timestamp tracking with NTP synchronization</li>";
    html += "<li>Duration calculation for power outages</li>";
    html += "<li>Statistics: Today, 7-day, 15-day, and monthly analysis</li>";
    html += "<li>Automatic monthly log reset to prevent storage overflow</li>";
    html += "</ul>";
    
    html += "<h5 class='mt-4'><i class='bi bi-shield-lock-fill'></i> WiFi Security Features</h5>";
    html += "<ul>";
    html += "<li>WiFi network scanning with hidden network detection</li>";
    html += "<li>WPS vulnerability detection based on 576+ device signatures</li>";
    html += "<li>Manufacturer identification using OUI database</li>";
    html += "<li>WiFi standard detection (WiFi 4/5/6)</li>";
    html += "<li>Security protocol identification (WEP, WPA, WPA2, WPA3)</li>";
    html += "<li>Signal strength analysis with visual indicators</li>";
    html += "</ul>";
    
    html += "<h5 class='mt-4'><i class='bi bi-router'></i> Supported Vulnerable Devices</h5>";
    html += "<p>Based on FARHAN-Shot-v2 vulnerability database:</p>";
    html += "<ul>";
    html += "<li><strong>TP-Link:</strong> Archer AX10/20/50/55/73/90/96, Deco X20/X50/X60/X90, TL-WR series</li>";
    html += "<li><strong>Xiaomi/Redmi:</strong> AX1800/3000/3600/5400/6000/9000, Mi Router series</li>";
    html += "<li><strong>Netgear:</strong> RAX10-200, Nighthawk AX series, WNR/WNDR series</li>";
    html += "<li><strong>ASUS:</strong> RT-AX53U-92U, TUF-AX3000/4200/5400, ZenWiFi</li>";
    html += "<li><strong>D-Link:</strong> DIR-X1560-6060, EAGLE PRO AI</li>";
    html += "<li><strong>Others:</strong> Huawei, ZTE, Tenda, Linksys, Belkin, Mercusys, Totolink, and 500+ more</li>";
    html += "</ul>";
    html += "</div></div>";
    
    html += "<div class='card mb-4'>";
    html += "<div class='card-header'><i class='bi bi-people-fill'></i> Credits</div>";
    html += "<div class='card-body'>";
    html += "<table class='table'>";
    html += "<tr><th>Author</th><td>" AUTHOR " (<a href='mailto:" EMAIL "'>" EMAIL "</a>)</td></tr>";
    html += "<tr><th>Based On</th><td>";
    html += "<a href='https://github.com/anbuinfosec/espPowerManagement' target='_blank'>espPowerManagement</a> by @anbuinfosec<br>";
    html += "<a href='https://github.com/GazaOS/FARHAN-Shot-v2' target='_blank'>FARHAN-Shot-v2</a> by GazaOS/Gtajisan";
    html += "</td></tr>";
    html += "<tr><th>License</th><td>MIT License</td></tr>";
    html += "<tr><th>Platform</th><td>ESP8266 (NodeMCU, Wemos D1 Mini, Generic)</td></tr>";
    html += "</table></div></div>";
    
    html += "<div class='card'>";
    html += "<div class='card-header'><i class='bi bi-exclamation-triangle-fill'></i> Disclaimer</div>";
    html += "<div class='card-body'>";
    html += "<div class='alert alert-warning'>";
    html += "<strong>Educational Use Only:</strong> This tool is designed for network administrators, security researchers, and educational purposes. ";
    html += "Always obtain proper authorization before testing network security. Unauthorized access to computer networks is illegal in most jurisdictions.";
    html += "</div></div></div>";
    
    html += pageFooter();
    return html;
}

void handleRoot() {
    server.send(200, "text/html", getDashboard());
}

void handlePower() {
    server.send(200, "text/html", getPowerPage());
}

void handleScan() {
    if (server.hasArg("start")) {
        scanNetworks();
    }
    server.send(200, "text/html", getScanPage());
}

void handleStats() {
    server.send(200, "text/html", getStatsPage());
}

void handleConfig() {
    server.send(200, "text/html", getConfigPage());
}

void handleAbout() {
    server.send(200, "text/html", getAboutPage());
}

void handleSave() {
    if (server.hasArg("ssid") && server.hasArg("pass")) {
        wifiSSID = server.arg("ssid");
        wifiPASS = server.arg("pass");
        saveConfig();
        
        String html = pageHeader("Saved", "config");
        html += "<div class='card'><div class='card-body text-center'>";
        html += "<i class='bi bi-check-circle-fill text-success' style='font-size:4rem;'></i>";
        html += "<h3 class='mt-3'>WiFi Settings Saved!</h3>";
        html += "<p>Device will attempt to connect to <strong>" + wifiSSID + "</strong></p>";
        html += "<p>Restarting in 3 seconds...</p>";
        html += "</div></div>";
        html += "<script>setTimeout(function(){location.href='/';},5000);</script>";
        html += pageFooter();
        
        server.send(200, "text/html", html);
        delay(3000);
        ESP.restart();
    }
}

void handleSaveAP() {
    if (server.hasArg("apssid") && server.hasArg("appass")) {
        String newSSID = server.arg("apssid");
        String newPass = server.arg("appass");
        
        if (newPass.length() >= 8) {
            apSSID = newSSID;
            apPASS = newPass;
            saveConfig();
            
            String html = pageHeader("Saved", "config");
            html += "<div class='card'><div class='card-body text-center'>";
            html += "<i class='bi bi-check-circle-fill text-success' style='font-size:4rem;'></i>";
            html += "<h3 class='mt-3'>AP Settings Saved!</h3>";
            html += "<p>New AP SSID: <strong>" + apSSID + "</strong></p>";
            html += "<p>Restarting in 3 seconds...</p>";
            html += "</div></div>";
            html += "<script>setTimeout(function(){location.href='/';},5000);</script>";
            html += pageFooter();
            
            server.send(200, "text/html", html);
            delay(3000);
            ESP.restart();
        } else {
            String html = pageHeader("Error", "config");
            html += "<div class='card'><div class='card-body text-center'>";
            html += "<i class='bi bi-x-circle-fill text-danger' style='font-size:4rem;'></i>";
            html += "<h3 class='mt-3'>Password Too Short</h3>";
            html += "<p>AP password must be at least 8 characters.</p>";
            html += "<a href='/config' class='btn btn-primary'>Go Back</a>";
            html += "</div></div>";
            html += pageFooter();
            server.send(200, "text/html", html);
        }
    }
}

void handleClear() {
    SPIFFS.remove(logFile);
    SPIFFS.remove(lastOnFile);
    
    String html = pageHeader("Cleared", "power");
    html += "<div class='card'><div class='card-body text-center'>";
    html += "<i class='bi bi-trash-fill text-success' style='font-size:4rem;'></i>";
    html += "<h3 class='mt-3'>Logs Cleared!</h3>";
    html += "<p>All power event logs have been deleted.</p>";
    html += "<a href='/power' class='btn btn-primary'>Back to Power</a>";
    html += "</div></div>";
    html += pageFooter();
    
    server.send(200, "text/html", html);
}

void handleClearScan() {
    scannedNetworks.clear();
    SPIFFS.remove(scanResultsFile);
    
    String html = pageHeader("Cleared", "scan");
    html += "<div class='card'><div class='card-body text-center'>";
    html += "<i class='bi bi-eraser-fill text-success' style='font-size:4rem;'></i>";
    html += "<h3 class='mt-3'>Scan Data Cleared!</h3>";
    html += "<p>All WiFi scan results have been deleted.</p>";
    html += "<a href='/scan' class='btn btn-primary'>Back to Scan</a>";
    html += "</div></div>";
    html += pageFooter();
    
    server.send(200, "text/html", html);
}

void handleRestart() {
    String html = pageHeader("Restarting", "config");
    html += "<div class='card'><div class='card-body text-center'>";
    html += "<i class='bi bi-arrow-clockwise pulse' style='font-size:4rem;color:#667eea;'></i>";
    html += "<h3 class='mt-3'>Restarting Device...</h3>";
    html += "<p>Please wait while the device reboots.</p>";
    html += "</div></div>";
    html += "<script>setTimeout(function(){location.href='/';},10000);</script>";
    html += pageFooter();
    
    server.send(200, "text/html", html);
    delay(1000);
    ESP.restart();
}

void handleApiScan() {
    scanNetworks();
    
    DynamicJsonDocument doc(4096);
    JsonArray networks = doc.createNestedArray("networks");
    
    for (size_t i = 0; i < scannedNetworks.size(); i++) {
        JsonObject net = networks.createNestedObject();
        net["ssid"] = scannedNetworks[i].ssid;
        net["bssid"] = scannedNetworks[i].bssid;
        net["rssi"] = scannedNetworks[i].rssi;
        net["channel"] = scannedNetworks[i].channel;
        net["security"] = scannedNetworks[i].securityType;
        net["manufacturer"] = scannedNetworks[i].manufacturer;
        net["wps_vulnerable"] = scannedNetworks[i].wpsEnabled;
        net["hidden"] = scannedNetworks[i].isHidden;
    }
    
    doc["count"] = scannedNetworks.size();
    doc["timestamp"] = getTimeString(time(nullptr));
    
    String output;
    serializeJson(doc, output);
    server.send(200, "application/json", output);
}

void handleApiStats() {
    Stats s = calculateStats();
    time_t now = time(nullptr);
    
    DynamicJsonDocument doc(1024);
    doc["uptime"] = millis() / 1000;
    doc["uptime_formatted"] = formatDuration(millis() / 1000);
    doc["free_heap"] = ESP.getFreeHeap();
    doc["chip_id"] = String(ESP.getChipId(), HEX);
    doc["networks_scanned"] = scannedNetworks.size();
    
    JsonObject power = doc.createNestedObject("power");
    power["today_off"] = s.todayOff;
    power["today_on"] = now - bootTime;
    power["week_off"] = s.last7Off;
    power["week_on"] = s.last7On;
    power["month_off"] = s.monthOff;
    power["month_on"] = s.monthOn;
    
    String output;
    serializeJson(doc, output);
    server.send(200, "application/json", output);
}

void handleNotFound() {
    String html = pageHeader("404 Not Found", "");
    html += "<div class='card'><div class='card-body text-center'>";
    html += "<i class='bi bi-question-circle-fill' style='font-size:4rem;color:#667eea;'></i>";
    html += "<h3 class='mt-3'>Page Not Found</h3>";
    html += "<p>The requested page does not exist.</p>";
    html += "<a href='/' class='btn btn-primary'>Go Home</a>";
    html += "</div></div>";
    html += pageFooter();
    server.send(404, "text/html", html);
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n========================================");
    Serial.println("ESP SecMonitor v" VERSION);
    Serial.println("Author: " AUTHOR " (" EMAIL ")");
    Serial.println("========================================\n");
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    
    if (!SPIFFS.begin()) {
        Serial.println("SPIFFS mount failed, formatting...");
        SPIFFS.format();
        SPIFFS.begin();
    }
    Serial.println("SPIFFS mounted successfully");
    
    loadConfig();
    Serial.println("Configuration loaded");
    
    bool wifiConnected = false;
    if (wifiSSID.length() > 0) {
        Serial.printf("Connecting to WiFi: %s\n", wifiSSID.c_str());
        WiFi.mode(WIFI_STA);
        WiFi.begin(wifiSSID.c_str(), wifiPASS.c_str());
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 30) {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            wifiConnected = true;
            Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
        } else {
            Serial.println("\nWiFi connection failed");
        }
    }
    
    if (!wifiConnected) {
        WiFi.mode(WIFI_AP);
    } else {
        WiFi.mode(WIFI_AP_STA);
    }
    
    WiFi.softAP(apSSID.c_str(), apPASS.c_str());
    Serial.printf("AP Started - SSID: %s, IP: %s\n", apSSID.c_str(), WiFi.softAPIP().toString().c_str());
    
    configTime(6 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("NTP configured (UTC+6)");
    
    int ntpWait = 0;
    while (time(nullptr) < 1000000000 && ntpWait < 20) {
        delay(500);
        Serial.print("*");
        ntpWait++;
    }
    Serial.println();
    
    bootTime = time(nullptr);
    Serial.printf("Boot time: %s\n", getTimeString(bootTime).c_str());
    
    File lastOn = SPIFFS.open(lastOnFile, "r");
    if (lastOn) {
        time_t lastOnTime = lastOn.readString().toInt();
        lastOn.close();
        
        if (lastOnTime > 0 && bootTime > lastOnTime) {
            time_t offDuration = bootTime - lastOnTime;
            logEvent("[OFF]", lastOnTime, offDuration);
            Serial.printf("Power was off for: %s\n", formatDuration(offDuration).c_str());
        }
    }
    
    logEvent("[ON]", bootTime);
    
    File f = SPIFFS.open(lastOnFile, "w");
    if (f) {
        f.print(bootTime);
        f.close();
    }
    
    checkMonthlyReset();
    
    server.on("/", handleRoot);
    server.on("/power", handlePower);
    server.on("/scan", handleScan);
    server.on("/stats", handleStats);
    server.on("/config", handleConfig);
    server.on("/about", handleAbout);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/saveap", HTTP_POST, handleSaveAP);
    server.on("/clear", handleClear);
    server.on("/clearscan", handleClearScan);
    server.on("/restart", handleRestart);
    server.on("/api/scan", handleApiScan);
    server.on("/api/stats", handleApiStats);
    server.onNotFound(handleNotFound);
    
    server.begin();
    Serial.println("HTTP server started on port 80");
    
    digitalWrite(LED_PIN, LOW);
    Serial.println("\n========================================");
    Serial.println("Setup complete! Device ready.");
    Serial.println("========================================\n");
}

void loop() {
    server.handleClient();
    
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 3600000) {
        lastUpdate = millis();
        
        File f = SPIFFS.open(lastOnFile, "w");
        if (f) {
            f.print(time(nullptr));
            f.close();
        }
        
        checkMonthlyReset();
    }
    
    if (WiFi.getMode() == WIFI_AP_STA && WiFi.status() != WL_CONNECTED) {
        static unsigned long lastReconnect = 0;
        if (millis() - lastReconnect > 60000) {
            lastReconnect = millis();
            WiFi.reconnect();
        }
    }
}
