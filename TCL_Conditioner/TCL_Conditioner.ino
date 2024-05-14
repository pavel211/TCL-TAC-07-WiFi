#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#include "TCL.h"
#include "EEPROM.h"

/////////////////////////////////////////
// EDIT HERE
/////////////////////////////////////////

// Main
const char* ssid        = "";
const char* password    = "";
const char* mqtt_server = "";
String      main_topic  = "tcl_conditioner";

// IP address
IPAddress ip(192,168,1,10);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

// OTA
#define OTAUSER         "admin"    // Логин для входа в OTA
#define OTAPASSWORD     "admin"    // Пароль для входа в ОТА

const char* name_device = "Кондиционер TCL";

/////////////////////////////////////////

ESP8266WebServer HttpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient espClient;
PubSubClient MQTT(espClient);

unsigned long CounterSend_current_temperature = 0;

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    TCL conditioner;

    unsigned long lastSecUpdate = 0;
    unsigned long MQTTlastUpdate = 0;
    bool set_new_data = false;

    int value_archive [] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

    void setup() 
    {
        // Загружаем параметры из ПЗУ
        conditioner.ac_display    = EepromReadInt (100, 0, 1, 1);
        conditioner.ac_beep       = EepromReadInt (105, 0, 1, 0);
        conditioner.v_swing_mode  = EepromReadInt (110, 0, 2, 0);
        conditioner.h_swing_mode  = EepromReadInt (115, 0, 2, 0);
        conditioner.v_fixing_mode = EepromReadInt (120, 0, 5, 0);
        conditioner.h_fixing_mode = EepromReadInt (125, 0, 5, 0);

        conditioner.target_temperature = EepromReadInt (130, 16, 31, 255);
        conditioner.ac_mode       = EepromReadInt (135, 1, 5, 255);
        conditioner.fan_mode      = EepromReadInt (140, 1, 9, 255);
        conditioner.swing_mode    = EepromReadInt (145, 0, 3, 255);
        conditioner.ac_turbo      = EepromReadInt (150, 0, 1, 255);
        conditioner.ac_power      = EepromReadInt (155, 0, 1, 255);

        Serial.begin(9600, SERIAL_8E1); // Настраиваем UART

        WiFi.mode(WIFI_STA);  
        WiFi.begin(ssid, password); 
        WiFi.config(ip, gateway, subnet);

        MQTT.setServer(mqtt_server, 1883);
        MQTT.setCallback(MQTT_new_message);

        httpUpdater.setup(&HttpServer, "/firmware", OTAUSER, OTAPASSWORD);
        HttpServer.onNotFound(handleNotFound);
        HttpServer.begin();
        HttpServer.on("/",[]()      { HttpServer.send(200, "text/html", WWW_MainPage()); });
        HttpServer.on("/setup",[]() { HttpServer.send(200, "text/html", WWW_setup()); });
        HttpServer.on("/edit",[]()  { HttpServer.send(200, "text/html", WWW_Edit()); });
    }

    void loop()
    {
      // Прослушивание HTTP-запросов от клиентов
      HttpServer.handleClient();

      if (millis()-lastSecUpdate > 99) // MQTT - 100 ms
       {
         MQTTlastUpdate = millis();
         // Обработка MQTT
         if (MQTT.connected()) { 
           MQTT.loop(); // Обработка входящих сообщений
           // Отправляем данные
           if (conditioner.target_temperature != value_archive[0]) { 
              value_archive[0] = conditioner.target_temperature; MQTT.publish((main_topic+"/get_target_temperature").c_str(),String(value_archive[0]).c_str(),true); }
           if (conditioner.ac_mode            != value_archive[1]) { 
              value_archive[1] = conditioner.ac_mode;            MQTT.publish((main_topic+"/get_ac_mode").c_str(),String(value_archive[1]).c_str(),true); }
           if (conditioner.fan_mode           != value_archive[2]) { 
              value_archive[2] = conditioner.fan_mode;           MQTT.publish((main_topic+"/get_fan_mode").c_str(),String(value_archive[2]).c_str(),true); }
           if (conditioner.swing_mode         != value_archive[3]) { 
              value_archive[3] = conditioner.swing_mode;         MQTT.publish((main_topic+"/get_swing_mode").c_str(),String(value_archive[3]).c_str(),true); }
           if (conditioner.ac_turbo           != value_archive[10]){ 
              value_archive[10] = conditioner.ac_turbo;          MQTT.publish((main_topic+"/get_ac_turbo").c_str(),String(value_archive[10]).c_str(),true); }
           if (conditioner.ac_power           != value_archive[12]){ 
              value_archive[12] = conditioner.ac_power;          MQTT.publish((main_topic+"/get_ac_power").c_str(),String(value_archive[12]).c_str(),true); }

           if (conditioner.real_current_temperature != value_archive[4]) { 
             if (millis() - CounterSend_current_temperature > 59999) // Отправляем только раз в 60 секунд
              {
                CounterSend_current_temperature = millis();
                value_archive[4] = conditioner.real_current_temperature; MQTT.publish((main_topic+"/real_current_temperature").c_str(),String(value_archive[4]).c_str(),true);
              }
           }
           
           if (conditioner.real_target_temperature != value_archive[5]) { 
              value_archive[5] = conditioner.real_target_temperature;  MQTT.publish((main_topic+"/real_target_temperature").c_str(),String(value_archive[5]).c_str(),true); }
           if (conditioner.real_ac_mode            != value_archive[6]) { 
              value_archive[6] = conditioner.real_ac_mode;             MQTT.publish((main_topic+"/real_ac_mode").c_str(),String(value_archive[6]).c_str(),true); }
           if (conditioner.real_fan_mode           != value_archive[7]) { 
              value_archive[7] = conditioner.real_fan_mode;            MQTT.publish((main_topic+"/real_fan_mode").c_str(),String(value_archive[7]).c_str(),true); }
           if (conditioner.real_swing_mode         != value_archive[8]) { 
              value_archive[8] = conditioner.real_swing_mode;          MQTT.publish((main_topic+"/real_swing_mode").c_str(),String(value_archive[8]).c_str(),true); }
           if (conditioner.real_ac_turbo           != value_archive[9]) { 
              value_archive[9] = conditioner.real_ac_turbo;            MQTT.publish((main_topic+"/real_ac_turbo").c_str(),String(value_archive[9]).c_str(),true); }
           if (conditioner.real_ac_power           != value_archive[11]){ 
              value_archive[11] = conditioner.real_ac_power;           MQTT.publish((main_topic+"/real_ac_power").c_str(),String(value_archive[11]).c_str(),true); }

         } else { if (MQTT.connect(WiFi.macAddress().c_str())) { MQTT.subscribe((main_topic+"/#").c_str()); } } // Если коннекта нет - переподключаемся
       }
     
      // Получаем данные от кондиционера
      if (conditioner.TCL_loop() != "") lastSecUpdate=millis(); // Если получено сообщение - обнуляем счетчик

      if ((millis()-lastSecUpdate > 499) and (set_new_data)) // Задание параметров не чаще, чем раз в 0.5 сек
       { 
        conditioner.control();
        set_new_data = false; 
        lastSecUpdate=millis();     // Следующее считывание только через 1.5 сек
       }
      if ((millis()-lastSecUpdate > 1499) and (!set_new_data)) // Раз в 1.5 секунды шлем запрос
       { 
         lastSecUpdate=millis();
         conditioner.GetValue();
       }
    }

    void MQTT_new_message(String topic, byte* message, unsigned int length) 
    {
       // Конвертируем
       String messageTemp;
       bool message_int_bool=true; for (int i = 0; i < length; i++) { messageTemp += (char)message[i]; if (!isdigit(message[i])) { message_int_bool = false; } }
       int message_int = messageTemp.toInt();

       if (message_int_bool){
         if (topic == (main_topic+"/set_target_temperature").c_str()) {
           conditioner.target_temperature = message_int; set_new_data = true; EepromSaveInt (130, messageTemp); 
         } else
         if (topic == (main_topic+"/set_ac_mode").c_str()) {
           conditioner.ac_mode            = message_int; set_new_data = true; EepromSaveInt (135, messageTemp); 
         } else
         if (topic == (main_topic+"/set_fan_mode").c_str()) {
           conditioner.fan_mode           = message_int; set_new_data = true; EepromSaveInt (140, messageTemp); 
         } else
         if (topic == (main_topic+"/set_swing_mode").c_str()) {
           conditioner.swing_mode         = message_int; set_new_data = true; EepromSaveInt (145, messageTemp); 
         }
         if (topic == (main_topic+"/set_ac_turbo").c_str()) {
           conditioner.ac_turbo           = message_int; set_new_data = true; EepromSaveInt (150, messageTemp); 
         }
         if (topic == (main_topic+"/set_ac_power").c_str()) {
           conditioner.ac_power           = message_int; set_new_data = true; EepromSaveInt (155, messageTemp); 
         }
      }
    }
    
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/* Выводить надпись, если такой страницы ненайдено */
void handleNotFound() {
  HttpServer.send(404, "text/plain", "404: Not found - Conditioner_spalnja_2");
}

String WWW_setup()
{
    return header("Setup") +  
    "<form action='/firmware'><input type=submit value=\"UPDATE Firmware\"></form>" +
    "</center>" +
    "<hr><left><form action='/'><input type=submit value=\"<< Back to main page\"></form></left>" + footer();
}

String WWW_Edit() 
{
    if (HttpServer.arg("ac_display") != "") { 
       conditioner.ac_display = HttpServer.arg("ac_display").toInt(); set_new_data = true;       EepromSaveInt (100, HttpServer.arg("ac_display")); 
    }
    if (HttpServer.arg("ac_beep") != "") { 
       conditioner.ac_beep = HttpServer.arg("ac_beep").toInt();  set_new_data = true;            EepromSaveInt (105, HttpServer.arg("ac_beep"));  
    }
    if (HttpServer.arg("v_swing_mode") != "") {
       conditioner.v_swing_mode = HttpServer.arg("v_swing_mode").toInt(); set_new_data = true;   EepromSaveInt (110, HttpServer.arg("v_swing_mode")); 
    }
    if (HttpServer.arg("h_swing_mode") != "") { 
       conditioner.h_swing_mode = HttpServer.arg("h_swing_mode").toInt();  set_new_data = true;  EepromSaveInt (115, HttpServer.arg("h_swing_mode")); 
    }
    if (HttpServer.arg("v_fixing_mode") != "") {
       conditioner.v_fixing_mode = HttpServer.arg("v_fixing_mode").toInt(); set_new_data = true; EepromSaveInt (120, HttpServer.arg("v_fixing_mode")); 
    }
    if (HttpServer.arg("h_fixing_mode") != "") {
       conditioner.h_fixing_mode = HttpServer.arg("h_fixing_mode").toInt(); set_new_data = true; EepromSaveInt (125, HttpServer.arg("h_fixing_mode")); 
    }

    if (HttpServer.arg("target_temperature") != "") { 
       conditioner.target_temperature = HttpServer.arg("target_temperature").toInt(); set_new_data = true; EepromSaveInt (130, HttpServer.arg("target_temperature")); 
    }
    if (HttpServer.arg("ac_mode") != "") { 
       conditioner.ac_mode = HttpServer.arg("ac_mode").toInt(); set_new_data = true;             EepromSaveInt (135, HttpServer.arg("ac_mode")); 
    }
    if (HttpServer.arg("fan_mode") != "") { 
       conditioner.fan_mode = HttpServer.arg("fan_mode").toInt(); set_new_data = true;           EepromSaveInt (140, HttpServer.arg("fan_mode")); 
    }
    if (HttpServer.arg("swing_mode") != "") {
       conditioner.swing_mode = HttpServer.arg("swing_mode").toInt(); set_new_data = true;       EepromSaveInt (145, HttpServer.arg("swing_mode")); 
    }
    if (HttpServer.arg("ac_turbo") != "") { 
       conditioner.ac_turbo = HttpServer.arg("ac_turbo").toInt();  set_new_data = true;          EepromSaveInt (150, HttpServer.arg("ac_turbo"));  
    }
    if (HttpServer.arg("ac_power") != "") { 
       conditioner.ac_power = HttpServer.arg("ac_power").toInt();  set_new_data = true;          EepromSaveInt (155, HttpServer.arg("ac_power"));  
    }

    return "<head><meta http-equiv=refresh content='2;URL=/' /></head><body style='background-color: #252525;'></body></html>";
}

String WWW_MainPage()
{
    return header("") +
    "<table border=0>"+
    "<tr><td colspan=2><h3>Основные параметры (задаются по MQTT):</h3></td></tr>" +

    "<tr><td>Температура в помещении: </td><td> <b> " + String(conditioner.real_current_temperature) + " *C</b></td></tr>" +

    WWW_view_bool_real (conditioner.ac_power, conditioner.real_ac_power, "Питание", "ac_power", false) +
    WWW_view_target_temperature(conditioner.target_temperature,conditioner.real_target_temperature) + 
    WWW_view_ac_mode(conditioner.ac_mode,conditioner.real_ac_mode) + 
    WWW_view_fan_mode(conditioner.fan_mode,conditioner.real_fan_mode) + 
    WWW_view_swing_mode(conditioner.swing_mode,conditioner.real_swing_mode) +
    WWW_view_bool_real (conditioner.ac_turbo, conditioner.real_ac_turbo, "Режим 'турбо'", "ac_turbo", true) +

    "<tr><td colspan=2><h3>Дополнительные параметры, хранятся в ПЗУ сетевой карты:</h3></td></tr>" +

    WWW_view_bool (conditioner.ac_display, "Дисплей (если ВЫКЛ -> режим АВТО)", "ac_display") +
    WWW_view_bool (conditioner.ac_beep, "Пищалка", "ac_beep") +
    WWW_v_swing_mode (conditioner.v_swing_mode) +
    WWW_h_swing_mode (conditioner.h_swing_mode) +
    WWW_v_fixing_mode (conditioner.v_fixing_mode) +
    WWW_h_fixing_mode (conditioner.h_fixing_mode) +
   
    "</table>"+

    "<div><b>Normal receive date: " + String(conditioner.normal_receive_date) + "</b></div>"+
    //"<div><b>RAW date: --" + CondGetRAW1 + "--</b></div>"+
    "<div><b>Work time: "     + String(int(millis()/1000/60/60/24)) + " days "+ String(int(millis()/1000/60/60%24)) + " hours " + String(int(millis()/1000/60%60))+ " min</b></div>" +

    "<form action='/setup'><input type=submit value=\"Setup\"></form>" +
    //"<form action='/reboot'><input type=submit value=\"Reboot\" style='background-color: #d43535;'></form>" +
    "</center>" + footer();
}

String footer() { return "</div><div class=q><left><a>&#169; RU4PAA, 2024</a></left></div>"; }
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
