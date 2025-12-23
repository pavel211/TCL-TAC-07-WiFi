#include "wifi_config.h"
#include <ESP8266WiFi.h>
#include <EEPROM.h>

const char* name_device = "Кондиционер TCL";

/* ================== EEPROM layout ================== */
#define EEPROM_TOTAL_SIZE  512

#define EEPROM_WIFI_BASE   200
#define EEPROM_MQTT_BASE   270

/* ================== Timings ================== */
#define WIFI_CONNECT_TIMEOUT 15000UL
#define AP_REBOOT_TIMEOUT    180000UL   // 3 минуты

/////////////////////////////////////////
// EDIT HERE
/////////////////////////////////////////

/* ================== AP params ================== */

#define AP_SSID "TCL_AC_Setup"
#define AP_PASS "12345678"

/////////////////////////////////////////

/* ================== Structures ================== */
struct WifiCreds {
  char ssid[32];
  char pass[32];
  uint8_t valid;
};

struct MqttCreds {
  char host[40];
  char user[32];
  char pass[32];
  char topic[32];
  uint8_t valid;
};

/* ================== Static state ================== */
static WifiCreds wifi;
static MqttCreds mqtt;

static ESP8266WebServer* web = nullptr;

static bool apMode = false;
static unsigned long apStartMs = 0;

/* ================== EEPROM ================== */
static void loadEEPROM() {
  EEPROM.begin(EEPROM_TOTAL_SIZE);

  EEPROM.get(EEPROM_WIFI_BASE, wifi);
  if (wifi.valid != 1) { // Проверяем, не мусорное ли значение
    memset(&wifi, 0, sizeof(wifi));
    wifi.valid = 0;
  }

  EEPROM.get(EEPROM_MQTT_BASE, mqtt);
  if (mqtt.valid != 1) { // Проверяем, не мусорное ли значение
    memset(&mqtt, 0, sizeof(mqtt));
    mqtt.valid = 0;
  }
}


static void saveWiFi() {
  wifi.valid = 1;
  EEPROM.put(EEPROM_WIFI_BASE, wifi);
  EEPROM.commit();
}

static void saveMQTT() {
  mqtt.valid = 1;
  EEPROM.put(EEPROM_MQTT_BASE, mqtt);
  EEPROM.commit();
}

bool GetApMode() {
   return apMode;  
}

/* ================== Web UI design ================== */

String footer() { return "</div><div class=q><left><a>&#169; RU4PAA, 2024-2025</a></left></div>"; }

String header(String t) {
  String CSS = "article { background: #f2f2f2; padding: 1.3em; }" 
    "body { color: #eaeaea; background-color: #252525; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; }"
    "div { padding: 0.5em; }"
    "h1 { margin: 0.5em 0 0 0; padding: 0.5em; }"
    "input { color: #faffff; background-color: #1fa3ec; line-height: 1.5em; border-radius: 0.5em; width: 335px; padding: 5px 10px; margin: 8px 0; box-sizing: border-box; border: 1px solid #555555; font-size: 21px; font-weight: bold; }"
    "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
    "select{ color: #faffff; background-color: #1fa3ec; line-height: 1.5em; border-radius: 0.5em; width: 335px; padding: 5px 10px; margin: 8px 0; box-sizing: border-box; border: 1px solid #555555; font-size: 21px; font-weight: bold; }"
    "nav { background: #0066ff; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
    "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
    "textarea { width: 100%; }";
    
  String h = "<!DOCTYPE html><html>";
    h = h + "<head><title>"+name_device+"</title><meta http-equiv='refresh' content='150'><meta http-equiv=\"Content-type\" content=\"text/html; charset=utf-8\" />";
    h = h + "<meta name=viewport content=\"width=device-width,initial-scale=1\">";
    h = h + "<style>"+CSS+"</style></head>";
    h = h + "<body><nav>"+name_device+"</nav><div>";   
    if (t != "") h = h + "<h1>"+t+"</h1>";    
    h = h + "</div><div><center>";
  return h; }
  
/* ================== Web UI pages ================== */

static void handleWiFiPage() {
  String p = header("WiFi settings");
  p += "<form method='POST' action='/save_wifi'><table border=0>"
       "<tr><td>SSID:</td><td><input name='ssid' value='" + String(wifi.ssid) + "'></td></tr>"
       "<tr><td>Password:</td><td><input name='pass' type='password' value=''><br>Leave the field empty to save the current password.</td></tr>"
       "<tr><td colspan=2><input type=submit value='Save & reboot'></td></tr>"
       "</table></form>"
       "</center><hr><left><form action='/'><input type=submit value=\"<< Back to main page\"></form></left>" + footer();
  web->send(200, "text/html", p);
}

static void handleMQTTPage() {
  String p = header("MQTT settings");
  p += "<form method='POST' action='/save_mqtt'><table border=0>"
       "<tr><td>Server:</td><td><input name='host' value='" + String(mqtt.host) + "'></td></tr>"
       "<tr><td>Login:</td><td><input name='user' value='" + String(mqtt.user) + "'></td></tr>"
       "<tr><td>Password:</td><td><input name='pass' type='password' value=''><br>Leave the field empty to save the current password.</td></tr>"
       "<tr><td>Main topic:</td><td><input name='topic' value='" + String(MQTT_topic()) + "'></td></tr>"
       "<tr><td colspan=2><input type=submit value='Save & reboot'></td></tr>"
       "</table></form>"
       "</center><hr><left><form action='/'><input type=submit value=\"<< Back to main page\"></form></left>" + footer();
  web->send(200, "text/html", p);
}

static void handleSaveWiFi() {
  strncpy(wifi.ssid, web->arg("ssid").c_str(), sizeof(wifi.ssid) - 1);
  if (web->arg("pass").length() > 0) { // Записываем только, если введен пароль
    strncpy(wifi.pass, web->arg("pass").c_str(), sizeof(wifi.pass) - 1);
  }
  saveWiFi();
  web->send(200, "text/html", "<meta http-equiv='refresh' content='6;url=/'>WiFi saved. Rebooting...");
  delay(400);
  ESP.restart();
}

static void handleSaveMQTT() {
  strncpy(mqtt.host,  web->arg("host").c_str(),  sizeof(mqtt.host)  - 1);
  strncpy(mqtt.user,  web->arg("user").c_str(),  sizeof(mqtt.user)  - 1);
  if (web->arg("pass").length() > 0) { // Записываем только, если введен пароль
    strncpy(mqtt.pass, web->arg("pass").c_str(), sizeof(mqtt.pass) - 1); 
  }
  strncpy(mqtt.topic, web->arg("topic").c_str(), sizeof(mqtt.topic) - 1);
  saveMQTT();
  web->send(200, "text/html", "<meta http-equiv='refresh' content='6;url=/'>MQTT saved. Rebooting...");
  delay(400);
  ESP.restart();
}

static void handleReboot() {
  web->send(200, "text/html", "<meta http-equiv='refresh' content='6;url=/'>Rebooting...");
  delay(400);
  ESP.restart();
}

/* ================== Public API ================== */

void WiFiConfig_begin(ESP8266WebServer& server) {
  web = &server;

  loadEEPROM();

  if (wifi.valid) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi.ssid, wifi.pass);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < WIFI_CONNECT_TIMEOUT) {
      delay(100);
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    apMode = true;
    apStartMs = millis();
  }

  /* Регистрируем страницы */
  web->on("/wifi", HTTP_GET, handleWiFiPage);
  web->on("/mqtt", HTTP_GET, handleMQTTPage);
  web->on("/save_wifi", HTTP_POST, handleSaveWiFi);
  web->on("/save_mqtt", HTTP_POST, handleSaveMQTT);
  web->on("/reboot", HTTP_GET, handleReboot);
}

void WiFiConfig_loop() {
  if (apMode && millis() - apStartMs > AP_REBOOT_TIMEOUT) {
    ESP.restart();
  }
}

bool WiFiConfig_isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String WiFiConfig_ip() {
  return WiFi.localIP().toString();
}

/* MQTT getters */
const char* MQTT_host()  { return mqtt.valid ? mqtt.host  : ""; }
const char* MQTT_user()  { return mqtt.valid ? mqtt.user  : ""; }
const char* MQTT_pass()  { return mqtt.valid ? mqtt.pass  : ""; }
const char* MQTT_topic() { return mqtt.valid ? mqtt.topic : "tcl_conditioner"; }
