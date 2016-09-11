#include <ArduinoOTA.h>

//const uint16_t ota_port = 8266;
//WiFiUDP OTA;
bool otaEnabled = false;
void initOTA(){
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    DBG_OUTPUT_PORT.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    DBG_OUTPUT_PORT.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DBG_OUTPUT_PORT.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DBG_OUTPUT_PORT.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) DBG_OUTPUT_PORT.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) DBG_OUTPUT_PORT.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) DBG_OUTPUT_PORT.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) DBG_OUTPUT_PORT.println("Receive Failed");
    else if (error == OTA_END_ERROR) DBG_OUTPUT_PORT.println("End Failed");
  });
  
  ArduinoOTA.setHostname(host);
  ArduinoOTA.begin();
  DBG_OUTPUT_PORT.println("OTA Ready");
}
void handleOTA(){
  ArduinoOTA.handle();
}
