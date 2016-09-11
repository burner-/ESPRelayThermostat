#define DBG_OUTPUT_PORT Serial
#include "H801WiFi.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <LinkedList.h>
#include <EEPROM.h>
#include <FS.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include "global.h"
#include "config.h"
#include "wifi.h"
#include "webserver.h"
#include "ota.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include <pb.h>
#include "communication.h"
#include "scada.h"


#define ACCESS_POINT_NAME "espsetup"


// sensor book keeping
bool allFound = false;
int sensorCount = 0;


void handle_gauge()
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["main_sensor"] = temp_main;
  root["backup_sensor"] = temp_backup;
  String buffer = "";
  char cbuf[512];
  root.prettyPrintTo(cbuf, sizeof(cbuf));
  buffer = cbuf;
  server.send(200, "text/plain", buffer);
}

void handle_sensors()
{
  DynamicJsonBuffer jsonBuffer;
  JsonArray& root = jsonBuffer.createArray();
  SensorInfo *sensor;
  for (int i = 0; i < sensors.size(); i++)
  {
    sensor = sensors.get(i);
    JsonObject& sensorinfo = root.createNestedObject();
    String sensorAddr;
    getHexString(sensor->SensorAddress, sensorAddr);
    sensorinfo["address"] = sensorAddr;
    sensorinfo["temp"] = sensor->Temperature;
  }

  String buffer = "";
  char cbuf[1024];
  root.printTo(cbuf, sizeof(cbuf));
  buffer = cbuf;
  server.send(200, "text/plain", buffer);
}


void handle_prometheus()
{
 SensorInfo *sensor;
 String retbuf = "";
 for (int i = 0; i < sensors.size(); i++)
 {
   sensor = sensors.get(i);
   String sensorAddr;
   getHexString(sensor->SensorAddress, sensorAddr);
  
   retbuf+="tempsensor";
   retbuf+="{address=\"";
   retbuf+=sensorAddr;
   retbuf+="\"} ";
   
   retbuf+=sensor->Temperature;
   retbuf+="\n";
 }
  server.send(200, "text/plain", retbuf);
}

void handle_switches()
{ 
  setBooleanValue("power", &powerOn);
  if (powerOn)
  {
    DBG_OUTPUT_PORT.println("Relay on by web");
    startByBackup = false;
  }
  else if (!powerOn)
  {
    DBG_OUTPUT_PORT.println("Relay off by web");
    startByBackup = false;
  } 
    
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  
  root["power"] = boolToInt(powerOn);
  
  String buffer = "";
  char cbuf[256];
  root.printTo(cbuf, sizeof(cbuf));
  buffer = cbuf;
  server.send(200, "text/plain", buffer);
  
}

void initWebserver()
{
  server.on("/switches", handle_switches);
  server.on("/general.html", send_general_html);
  
  server.on("/config.html", send_network_configuration_html);
  server.on("/admin/values", send_network_configuration_values_html);
  server.on("/admin/connectionstate", send_connection_state_values_html);
  server.on("/admin/generalvalues", send_general_configuration_values_html);
  server.on("/gauge", handle_gauge);
  
  server.on("/metrics", handle_prometheus);
  server.on("/tempsensors", handle_sensors);
  server.onNotFound(handle_unknown);
  server.begin();
  
}

void setup() 
{
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.printf("Sketch size: %u\n", ESP.getSketchSize());
  DBG_OUTPUT_PORT.printf("Free size: %u\n", ESP.getFreeSketchSpace());
  
  SPIFFS.begin();
  EEPROM.begin(2048);
  if (!ReadConfig())
  {
            resetconfig();
            AdminEnabled = true;
  } 
  else if (stringConfig.ssid != "MYSSID"){
          AdminEnabled = false;
  }
  if (!AdminEnabled)
  {
    WiFi.mode(WIFI_STA);
    WiFi.hostname(host);
    
    ConfigureWifi();
  }
  
  if (AdminEnabled)
  {
    //DBG_OUTPUT_PORT.setDebugOutput(true);
    DBG_OUTPUT_PORT.println("Entering to default config mode");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ACCESS_POINT_NAME);
        DBG_OUTPUT_PORT.print("SoftAP IP: ");
    DBG_OUTPUT_PORT.println(WiFi.softAPIP());
  }
  
  
  initOTA();
  initWebserver();
  
  initScada();
}


void loop() {
  handleOTA();
  server.handleClient();
  handleScada();
}

