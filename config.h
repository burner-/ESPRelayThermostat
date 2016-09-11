#define READ_ERROR_VALUE  -85 //fake temp value
const char* host = "roomcontroller1";

struct stringConf {
  String ssid;
  String password;
  String ntpServerName;
  String DeviceName;
  
} stringConfig;

struct strConfig {
  char BOFmark = 'A';
  byte  IP[4] = {192,168,4,1};
  byte  Netmask[4] = {255,255,255,0};
  byte  Gateway[4] = {0,0,0,0};
  boolean dhcp = true;
  boolean AutoTurnOff = true;
  boolean AutoTurnOn = true;
  double temp_start_main = 0;
  double temp_stop_main = 0;
  double temp_start_backup = 0;
  double temp_stop_backup = 0;
  byte temp_main_sensor[8] = {0,0,0,0,0,0,0,0}; //OneWire address for limit 1 sensor
  byte temp_backup_sensor[8] = {0,0,0,0,0,0,0,0}; //OneWire address for backup sensor
    
  char EOFmark = 'E';
}   config;



boolean ReadStringConfig()
{
  DBG_OUTPUT_PORT.println("Reading string configuration");
  if (EEPROM.read(0) == 'C' && EEPROM.read(1) == 'F'  && EEPROM.read(2) == 'G' )
  {
    DBG_OUTPUT_PORT.println("Configuration Found");
    stringConfig.ssid = ReadStringFromEEPROM(32);
    stringConfig.password = ReadStringFromEEPROM(64);
    stringConfig.ntpServerName = ReadStringFromEEPROM(96);
    stringConfig.DeviceName= ReadStringFromEEPROM(128);
    return true;
  }
  else
  {
    DBG_OUTPUT_PORT.println("Configurarion not found!");
    return false;
  }
}

boolean ReadConfig()
{
  int count = EEPROM_readGeneric(300, config);
  DBG_OUTPUT_PORT.print("Bytes read: ");
  DBG_OUTPUT_PORT.println(count);
  if (config.BOFmark == 'A' && config.EOFmark == 'E')
  {
    DBG_OUTPUT_PORT.println("Configuration found");
    return ReadStringConfig();    
  }
  else 
  {
    DBG_OUTPUT_PORT.print("Cannot find configuration! BOF ");
    DBG_OUTPUT_PORT.print(config.BOFmark);
    DBG_OUTPUT_PORT.print(" EOF ");
    DBG_OUTPUT_PORT.println(config.EOFmark);
    return false;
  }
}

void WriteStringConfig()
{
  DBG_OUTPUT_PORT.println("Writing string Config");
  EEPROM.write(0,'C');
  EEPROM.write(1,'F');
  EEPROM.write(2,'G');
  WriteStringToEEPROM(32,stringConfig.ssid);
  WriteStringToEEPROM(64,stringConfig.password);
  WriteStringToEEPROM(96,stringConfig.ntpServerName);
  WriteStringToEEPROM(128,stringConfig.DeviceName);
  EEPROM.commit();
}

void WriteConfig()
{
  DBG_OUTPUT_PORT.print("Writing config to memory ");
  config.BOFmark = 'A';
  config.EOFmark = 'E';
  int count = EEPROM_writeGeneric(300, config);
  DBG_OUTPUT_PORT.print(count);
  EEPROM.commit();
  DBG_OUTPUT_PORT.println("Bytes writed");
  WriteStringConfig();
}



void resetconfig()
{
  DBG_OUTPUT_PORT.println("Restoring defaults");
      // DEFAULT CONFIG
    stringConfig.ssid = "MYSSID";
    stringConfig.password = "";
    config.dhcp = true;
    config.IP[0] = 192; config.IP[1] = 168; config.IP[2] = 1; config.IP[3] = 100;
    config.Netmask[0] = 255; config.Netmask[1] = 255; config.Netmask[2] = 255; config.Netmask[3] = 0;
    config.Gateway[0] = 192; config.Gateway[1] = 168; config.Gateway[2] = 1; config.Gateway[3] = 1;
    stringConfig.DeviceName = "Not Named";
    config.AutoTurnOff = false;
    config.AutoTurnOn = false;
    config.temp_start_main = 45;
    config.temp_stop_main = 30;
    config.temp_start_backup = 25;
    config.temp_stop_backup = 10; 
  
  //  WriteConfig();
    DBG_OUTPUT_PORT.println("General config applied");
 }

 



