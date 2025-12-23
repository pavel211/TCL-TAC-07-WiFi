#pragma once
#include <Arduino.h>
#include <ESP8266WebServer.h>

/* ===== Public API ===== */

void WiFiConfig_begin(ESP8266WebServer& server);
void WiFiConfig_loop();

bool WiFiConfig_isConnected();
String WiFiConfig_ip();

bool GetApMode();

String footer();
String header(String t);

/* MQTT getters */
const char* MQTT_host();
const char* MQTT_user();
const char* MQTT_pass();
const char* MQTT_topic();
