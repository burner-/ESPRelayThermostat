#include <OneWire.h>


#define pbufUDP_PORT 8889
#define jsonUDP_PORT 8890
#define ONEWIRE_PIN D3
double temp_main = READ_ERROR_VALUE;           //last temperature
double temp_backup = READ_ERROR_VALUE;           //last temperature
boolean powerOn = false;

//WiFiUDP jsonUdp;
WiFiUDP pbufUdp;
OneWire  ds(ONEWIRE_PIN);




class SensorInfo
{
public:
  byte SensorAddress[8] = {};
  float Temperature = 0;
  boolean online = false;
  boolean local = false;
};
// packet types
typedef enum packetType { 
  TYPE_SENDERROR = 0x5,
  TYPE_1WIRE = 0x10,
  TYPE_PID_INFO = 0x11,
  TYPE_PB_TEMP_INFO = 0x20,
  // server to node
  TYPE_PID_SET_ADDRESS = 0xF0,
  TYPE_PID_SET_SETPOINT = 0xF1,
  TYPE_PID_SET_TUNINGS = 0xF2,
  TYPE_SET_COMPRESSOR_SETTINGS = 0x70
};
void dumpType(byte type)
{
  if (type == TYPE_SENDERROR)
    DBG_OUTPUT_PORT.print( "SENDERROR");
  else if (type == TYPE_1WIRE)
    DBG_OUTPUT_PORT.print( "1WIRE");
  else if (type == TYPE_PID_INFO)
    DBG_OUTPUT_PORT.print( "PID_INFO");
  else if (type == TYPE_PID_SET_ADDRESS)
    DBG_OUTPUT_PORT.print( "PID_SET_ADDRESS");
  else if (type == TYPE_PID_SET_SETPOINT)
    DBG_OUTPUT_PORT.print( "PID_SET_SETPOINT");
  else if (type == TYPE_PID_SET_TUNINGS)
    DBG_OUTPUT_PORT.print( "PID_SET_TUNINGS");
  else if (type == TYPE_SET_COMPRESSOR_SETTINGS)
    DBG_OUTPUT_PORT.print( "TYPE_SET_COMPRESSOR_SETTINGS");
  else if (type == TYPE_PB_TEMP_INFO)
    DBG_OUTPUT_PORT.print("TYPE_PB_TEMP_INFO");
    
  else
    DBG_OUTPUT_PORT.print(type);
}

LinkedList<SensorInfo*> sensors = LinkedList<SensorInfo*>();

void updateTemp(byte address[8], float curTemp)
{
  if (ByteArrayCompare(config.temp_main_sensor, address, 8))
  {
    temp_main = curTemp;
  }
  else if (ByteArrayCompare(config.temp_backup_sensor, address, 8))
  {
    temp_backup = curTemp;
  }
}

void updateSensorInfo(byte address[8], float curTemp, boolean local)
{
  //update temp to static values
  updateTemp(address, curTemp);
  
  SensorInfo *sensor;
  for (int i = 0; i < sensors.size(); i++)
  {
    sensor = sensors.get(i);
    if (ByteArrayCompare(sensor->SensorAddress, address, 8))
    {
      sensor->online = true;
      sensor->local = local;
      sensor->Temperature = curTemp;
      
      return;
    }
  }

  SensorInfo *newsensor = new SensorInfo();
  copyByteArray(address, newsensor->SensorAddress, 8);
  newsensor->online = true;
  newsensor->local = local;
  newsensor->Temperature = curTemp;
  sensors.add(newsensor);
  //DBG_OUTPUT_PORT.println("");
  DBG_OUTPUT_PORT.println("Sensor added");
}



// Allow remote nodes to send temperature values. 
void netRecvProtoBuffTempInfo(byte packetBuffer[], int packetSize )
{
  TempInfo message;
  pb_istream_t stream = pb_istream_from_buffer(packetBuffer, packetSize);
  bool status = pb_decode(&stream, TempInfo_fields, &message);
  if (!status)
  {
    DBG_OUTPUT_PORT.print("Message decoding failed: ");
    DBG_OUTPUT_PORT.println(PB_GET_ERROR(&stream));
  } 
  else
  {
     
    if (message.SensorAddr.size == 8)
    {
     
      updateSensorInfo(message.SensorAddr.bytes,message.Temp, false);
      
     
    }
  }
}

bool readNetwork()
{
  //Process ethernet
  int packetSize =  pbufUdp.parsePacket();
  if(packetSize)
  {
    DBG_OUTPUT_PORT.print("Received udp packet of size ");
    DBG_OUTPUT_PORT.print(packetSize);
    DBG_OUTPUT_PORT.print(" From ");
    IPAddress remote =  pbufUdp.remoteIP();
    for (int i =0; i < 4; i++)
    {
      DBG_OUTPUT_PORT.print(remote[i], DEC);
      if (i < 3)
      {
        DBG_OUTPUT_PORT.print(".");
      }
    }
    DBG_OUTPUT_PORT.print(", port ");
    DBG_OUTPUT_PORT.print( pbufUdp.remotePort());
    int messagetype =  pbufUdp.read();
    DBG_OUTPUT_PORT.print(" messagetype: ");   
    dumpType(messagetype);
    DBG_OUTPUT_PORT.println();
    
    byte packetBuffer[UDP_TX_PACKET_MAX_SIZE - 1];
    pbufUdp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE - 1);
    yield();
    // parse messages by type
    if (messagetype == TYPE_SET_COMPRESSOR_SETTINGS)
    {
      //netRecvCompressorSettings(packetBuffer, (packetSize -1));
    } 
    else if (messagetype == TYPE_PB_TEMP_INFO)
    {
      netRecvProtoBuffTempInfo(packetBuffer, (packetSize -1));
    }
    return true;
  } 
  else 
    return false;
  
}

void doJob(){
  //yield();
  delay(1);
  server.handleClient();
  readNetwork();
}

void pollSleep(int time)
{
  time = time / 10;
  for (int i = 0; time > i; i++)
  {
    delay(10);
    server.handleClient();
    readNetwork();
  }
}





/*
void sendJsonTempInfo(byte address[8], float curTemp)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& sensorinfo = root.createNestedObject("tempinfo");
  String sensorAddr;
  getHexString(address, sensorAddr);
  sensorinfo["address"] = sensorAddr;
  sensorinfo["temp"] = curTemp;

  String jsonStringBuffer = "";
  char cbuf[256];
  size_t jsonLenght = root.printTo(cbuf, sizeof(cbuf));

  jsonUdp.beginPacket(mcast, jsonPort);
  jsonUdp.write(cbuf, jsonLenght);
  jsonUdp.endPacket();
}
*/



void readTempSensor(byte addr[8], bool needConversion = false)
{
  int i;
  byte present = 0;
  byte data[9];
  float tempVal = 0;
  if (needConversion) // Each temp sensor need internal temp conversion from sensor to scratchpad
  {
    ds.reset();
    doJob();
    ds.select(addr);
    doJob();
    ds.write(0x44);
    pollSleep(750);
  }
  present = ds.reset();
  doJob();
  ds.select(addr); // select sensor
  doJob();
  ds.write(0xBE);         // Read Scratchpad
  doJob();
  hexPrintArray(addr,8);
  DBG_OUTPUT_PORT.print(" \t");
  // read measurement data
  for (i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  if (OneWire::crc8(data, 8) != data[8])
  {
    DBG_OUTPUT_PORT.println(" Data CRC is not valid!");
    return;
  }
  int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;
  // print also as human readable mode  
  LowByte = data[0];
  HighByte = data[1];
  TReading = (HighByte << 8) + LowByte;
  SignBit = TReading & 0x8000;  // test most sig bit
  if (SignBit) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25
  Whole = Tc_100 / 100;  // separate off the whole and fractional portions
  Fract = Tc_100 % 100;
  tempVal = (float)Whole;
  tempVal += ((float)Fract) / 100.0;
  if (SignBit) // If its negative
  {
    tempVal = 0 - tempVal;
  }
  doJob();
  DBG_OUTPUT_PORT.print(tempVal);
  if (tempVal != 85.00 && tempVal != -85)
  {
    
//    updateTemp(addr, tempVal);
    DBG_OUTPUT_PORT.println("");
    updateSensorInfo(addr, tempVal, true);
  } 
  else 
    DBG_OUTPUT_PORT.println(" ERR");
}

// remove one offline sensor at each iteration
void cleanOfflineSensors()
{
  SensorInfo *sensor;
  for (int i = 0; i < sensors.size(); i++)
  {
    sensor = sensors.get(i);
    if (!sensor->online && sensor->local)
    {
      DBG_OUTPUT_PORT.println("Sensor removed");
      sensors.remove(i);
      //clean more than just first
      cleanOfflineSensors();
      return;
    }
  }
}

void markSensorsToOffline()
{
  SensorInfo *sensor;
  for (int i = 0; i < sensors.size(); i++)
  {
    sensor = sensors.get(i);
    if (sensor->local == true)
      sensor->online = false;
  }
}


void searchAllTempSensors()
{
  int count = 0;
  doJob();
  ds.reset_search();
  markSensorsToOffline();
  byte addr[8];
  while (ds.search(addr)) {
    // debug print sensor address
    DBG_OUTPUT_PORT.print("Found ");
    hexPrintArray(addr,8);
    DBG_OUTPUT_PORT.println("");
    if (OneWire::crc8(addr, 7) == addr[7]) {
      if (addr[0] == 0x10) {
        //#DBG_OUTPUT_PORT.print("Device is a DS18S20 family device.\n");
        count++;
        readTempSensor(addr, 1);
      }
      else if (addr[0] == 0x28) {
        count++;
        //#DBG_OUTPUT_PORT.print("Device is a DS18B20 family device.\n");
        readTempSensor(addr, true);
      }
      else {
        DBG_OUTPUT_PORT.print("Device family is not recognized: 0x");
        DBG_OUTPUT_PORT.println(addr[0], HEX);
      }
    } 
    else 
    {
      DBG_OUTPUT_PORT.println(" Address CRC is not valid!");
    }
  }
  // clean unfound sensors
  cleanOfflineSensors();
  DBG_OUTPUT_PORT.print("All ");
  DBG_OUTPUT_PORT.print(count);
  DBG_OUTPUT_PORT.println(" sensors found");
}


void readAllTempSensors()
{
    ds.reset();
    doJob();
    ds.skip();
    doJob();
    ds.write(0x44);
    pollSleep(750);
    SensorInfo *sensor;
    for (int i = 0; i < sensors.size(); i++)
    {
      sensor = sensors.get(i);
      if(sensor->local)
        readTempSensor(sensor->SensorAddress);
    }
}
void initScada()
{
  //jsonUdp.begin(localPort);
  pbufUdp.begin(pbufUDP_PORT);
  searchAllTempSensors();
}
bool startByBackup = false;

unsigned long nexttempread = 0;
void handleScada()
{
  if (nexttempread < millis())
  {
    readAllTempSensors();
    nexttempread = millis() + 5000;
  }
  
  while (readNetwork()){}
  
  if(temp_main > config.temp_start_main && config.AutoTurnOn && !powerOn)
  {
    DBG_OUTPUT_PORT.println("Relay on by main sensor");
    powerOn  = true;
  }
  else if(temp_backup > config.temp_start_backup && config.AutoTurnOn && !powerOn)
  {
    DBG_OUTPUT_PORT.println("Relay on by backup sensor");
    startByBackup = true;
    powerOn  = true;
  } 
  else if(!startByBackup && temp_main < config.temp_stop_main && config.AutoTurnOff && powerOn)
  {
    DBG_OUTPUT_PORT.println("Relay off by main sensor");
    powerOn  = false;
  }
  else if(startByBackup && temp_backup < config.temp_stop_backup && config.AutoTurnOff && powerOn)
  {
    DBG_OUTPUT_PORT.println("Relay off by backup sensor");
    startByBackup = false;
    powerOn  = false;
  }
    
}


