
ESP8266WebServer server ( 80 );


void addStringValue(String &string,String valuename, String value)
{
  string += valuename + "|" + value +  "|input\n";
}

void addFloatValue(String &string,String valuename, float value)
{
  string += valuename + "|" + (String) value +  "|input\n";
}

void addIntValue(String &string,String valuename, int value)
{
  string += valuename + "|" + (String) value +  "|input\n";
}


void setBooleanValue(const char parmname[], boolean *target)
{
  
  if (server.hasArg(parmname))
  {
    
    String argVal = server.arg(parmname);
    if (argVal == "1" || argVal == "true")
    {
      DBG_OUTPUT_PORT.print(parmname);
      DBG_OUTPUT_PORT.println(" true");
      *target = true;
    }
    else 
    {
      DBG_OUTPUT_PORT.print(parmname);
      DBG_OUTPUT_PORT.println(" false");
      *target = false;
    }
  }
}
int boolToInt(boolean bo)
{
  if (bo) return 1;
  else return 0;
}


void setAddressConfigValue(const char parmname[], byte target[])
{
  if (server.hasArg(parmname))
  {
    String argVal = server.arg(parmname);
    byte addr[8];
    if (argVal.length() == 16)
    {
      argVal.toUpperCase();
      getAddressBytes(argVal, addr);
      copyByteArray(addr, target, 8);
    }
    else
      DBG_OUTPUT_PORT.println("ignoring bogus sensor address");
  }
}
void setFloatConfigValue(const char parmname[], float *target)
{
  if (server.hasArg(parmname))
  {
    String argVal = server.arg(parmname);
    *target = argVal.toFloat();
  }
}

void setDoubleConfigValue(const char parmname[], double *target)
{
  if (server.hasArg(parmname))
  {
    String argVal = server.arg(parmname);
    *target = argVal.toFloat();
  }
}

void setIntConfigValue(const char parmname[], int *target)
{
  if (server.hasArg(parmname))
  {
    String argVal = server.arg(parmname);
    *target = argVal.toInt();
  }
}
String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
// Read static file from filesystem
bool handleFileRead(String path){
  if(path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  
  if(SPIFFS.exists(pathWithGz) || 
    SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    DBG_OUTPUT_PORT.println("handleFileRead: " + path);
    File file = SPIFFS.open(path, "r");
    server.sendHeader("Cache-Control", "no-transform,public,max-age=600,s-maxage=600");
    DBG_OUTPUT_PORT.println("Header sent");
    //yield();
    server.streamFile(file, contentType);
    DBG_OUTPUT_PORT.println("streamFile done");
    file.close();
    return true;
  }
  return false;
}

// Handle generic url
void handle_unknown()
{
  // Try find static file from filesystem
  if(!handleFileRead(server.uri()))
  {
      server.send(404, "text/plain", "FileNotFound");
      DBG_OUTPUT_PORT.println("");
      DBG_OUTPUT_PORT.print("404 Not found: ");
      DBG_OUTPUT_PORT.println(server.uri());
  }
      
}



void send_general_configuration_values_html()
{
  String values ="";
  values += "devicename|" +  (String)  stringConfig.DeviceName +  "|input\n";
  String sensorAddr;
  getHexString(config.temp_main_sensor, sensorAddr);
  addStringValue(values, "temp_main_sensor", sensorAddr);

  getHexString(config.temp_backup_sensor, sensorAddr);
  addStringValue(values, "temp_backup_sensor", sensorAddr);
   
  
  addFloatValue(values, "temp_start_main", config.temp_start_main);
  addFloatValue(values, "temp_stop_main", config.temp_stop_main);
  addFloatValue(values, "temp_start_backup", config.temp_start_backup);
  addFloatValue(values, "temp_stop_backup", config.temp_stop_backup);
  
  
  values += "toffenabled|" +  (String) (config.AutoTurnOff ? "checked" : "") + "|chk\n";
  values += "tonenabled|" +  (String) (config.AutoTurnOn ? "checked" : "") + "|chk\n";
  server.send ( 200, "text/plain", values);
  DBG_OUTPUT_PORT.println(__FUNCTION__); 
}

void send_general_html()
{
  
  if (server.args() > 0 )  // Save Settings
  {
    config.AutoTurnOn = false;
    config.AutoTurnOff = false;
    String temp = "";
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      if (server.argName(i) == "devicename") stringConfig.DeviceName = urldecode(server.arg(i)); 
      if (server.argName(i) == "tonenabled") config.AutoTurnOn = true; 
      if (server.argName(i) == "toffenabled") config.AutoTurnOff = true; 
    }
    
    setDoubleConfigValue("temp_start_main", &config.temp_start_main);
    setDoubleConfigValue("temp_stop_main", &config.temp_stop_main);
    setDoubleConfigValue("temp_start_backup", &config.temp_start_backup);
    setDoubleConfigValue("temp_stop_backup", &config.temp_stop_backup);
    
    setAddressConfigValue("temp_main_sensor", config.temp_main_sensor);
    setAddressConfigValue("temp_backup_sensor", config.temp_backup_sensor);
    
    WriteConfig();
    
  }
  server.sendHeader("Cache-Control", "no-transform,public,max-age=600,s-maxage=600");
  handleFileRead("/general.html");
  //server.send ( 200, "text/html", PAGE_AdminGeneralSettings ); 
  DBG_OUTPUT_PORT.println(__FUNCTION__); 
  
  
}

#include "PAGE_NetworkConfiguration.h"

