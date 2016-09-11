//
// Check the Values is between 0-255
//
boolean checkRange(String Value)
{
   if (Value.toInt() < 0 || Value.toInt() > 255)
   {
     return false;
   }
   else
   {
     return true;
   }
}



void hexPrint(byte b)
{
  if (b < 0x10)
    DBG_OUTPUT_PORT.print("0");
  DBG_OUTPUT_PORT.print(b, HEX);
}

void hexPrintArray(byte bytes[], int len)
{
  for (int i = 0; i < len; i++) {
    hexPrint(bytes[i]);
  }
}



void copyByteArray(byte inBytes[], byte outBytes[], int lenght)
{
  if (lenght > 8)
    lenght = 8;
  for (int i = 0; i < lenght; i++)
  {
    outBytes[i] = inBytes[i];
  }
}

boolean ByteArrayCompare(byte a[], byte b[], int array_size)
{
  for (int i = 0; i < array_size; ++i)
    if (a[i] != b[i])
      return(false);
  return(true);
}

// convert a single hex digit character to its integer value (from https://code.google.com/p/avr-netino/)
unsigned char h2int(char c)
{
    if (c >= '0' && c <='9'){
        return((unsigned char)c - '0');
    }
    if (c >= 'a' && c <='f'){
        return((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <='F'){
        return((unsigned char)c - 'A' + 10);
    }
    return(0);
}
String urldecode(String input) // (based on https://code.google.com/p/avr-netino/)
{
   char c;
   String ret = "";
   
   for(byte t=0;t<input.length();t++)
   {
     c = input[t];
     if (c == '+') c = ' ';
         if (c == '%') {


         t++;
         c = input[t];
         t++;
         c = (h2int(c) << 4) | h2int(input[t]);
     }
    
     ret.concat(c);
   }
   return ret;
  
}

template <class T> int EEPROM_writeGeneric(int ee, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  return i;
}

template <class T> int EEPROM_readGeneric(int ee, T& value)
{
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}

void WriteStringToEEPROM(int beginaddress, String string)
{
  char  charBuf[string.length()+1];
  string.toCharArray(charBuf, string.length()+1);
  for (int t=  0; t<sizeof(charBuf);t++)
  {
      EEPROM.write(beginaddress + t,charBuf[t]);
  }
}
String  ReadStringFromEEPROM(int beginaddress)
{
  byte counter=0;
  char rChar;
  String retString = "";
  while (1)
  {
    rChar = EEPROM.read(beginaddress + counter);
    if (rChar == 0) break;
    if (counter > 31) break;
    counter++;
    retString.concat(rChar);

  }
  return retString;
}
void EEPROMWritelong(int address, long value)
{
      byte four = (value & 0xFF);
      byte three = ((value >> 8) & 0xFF);
      byte two = ((value >> 16) & 0xFF);
      byte one = ((value >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, four);
      EEPROM.write(address + 1, three);
      EEPROM.write(address + 2, two);
      EEPROM.write(address + 3, one);
}
long EEPROMReadlong(long address)
      {
      //Read the 4 bytes from the eeprom memory.
      long four = EEPROM.read(address);
      long three = EEPROM.read(address + 1);
      long two = EEPROM.read(address + 2);
      long one = EEPROM.read(address + 3);

      //Return the recomposed long by using bitshift.
      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}
byte getHexVal(byte c)
{
  if (c >= '0' && c <= '9')
    return (byte)(c - '0');
  else
    return (byte)(c - 'A' + 10);
}
void getHexString(byte bytes[], String &retstring)
{
        retstring = "";
  for (int i = 0; i < 8; i++)
  {
    byte b = bytes[i];
    if (b < 0x10)
      retstring += "0";// zero pad to two digit
    retstring += String(b, HEX);
  }
  retstring.toUpperCase();
  return;
}
void getAddressBytes(String string, byte retBytes[])
{
  //string += "AA";
  byte byteArr[18];
  string.getBytes(byteArr, 17); //use over sized buffer & value because that ass rape shit function corrupt last byte
  int bytepos = 0;
  for (int i = 0; i < 16; i += 2)
  {
    byte b = getHexVal(byteArr[i + 1]) + (getHexVal(byteArr[i]) << 4);
    retBytes[bytepos] = b;
    bytepos++;
  }
  DBG_OUTPUT_PORT.println("");
  return;
}
