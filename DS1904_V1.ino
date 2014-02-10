#include <OneWire.h>
#include <Time.h>

// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 
OneWire  ds(2);  // on pin 10 (a 4.7K resistor is necessary)

void setup(void) {
  Serial.begin(9600);
  printMenu();
  //Serial.printKeyboard.begin();
}

void loop()
{
  // Wait for a serial byte to be received:
  while( !Serial.available() )
    ;
  char c = Serial.read();
  // Once received, act on the serial input:
  switch (c)
  {
  case 'r':
    readTime();
    break;
  case 'w':
    writeTime();
    break;
  case 'z':
    readSystemTime();
    break;
  case 'x':
    writeSystemTime();
    break;
  case 'h':
    printMenu();
    break;
  }
  if (timeStatus() == timeSet) {
    digitalWrite(13, HIGH); // LED on if synced
  } else {
    digitalWrite(13, LOW);  // LED off if needs refresh
  }
}

void printMenu()
{
  Serial.println();
  Serial.println(F("DS1904 iButton Menu"));
  Serial.println(F("====================================="));
  Serial.println(F("\t r) Read Time from iButton"));
  Serial.println(F("\t w) Write Time to iButton"));
  Serial.println(F("\t z) Read System time"));
  Serial.println(F("\t x) Write System time (followed immediately by time_t integer)"));
  Serial.println(F("\t h) Print help menu"));
  Serial.println();
}

void readTime() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  long timestamp = 0;
  String arduinoTime;
  
  if ( !ds.search(addr)) {
    //Serial.println("No more addresses.");
    //Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  //Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    //Serial.write(' ');
    if (addr[i] < 10) {
      //Serial.print('0');
      //Serial.print(addr[i],HEX);
    } else {
      //Serial.print(addr[i], HEX);
    }
  }

/*  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
*/
  //Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x24:
      //Serial.println("  Chip = DS1904");
      break;
    default:
      //Serial.println("Device unknown.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x66);        // 0x66=Read; 0x99=Write
  
  delay(20);    
  for ( i = 0; i < 5; i++) {           // we need 5 bytes
    data[i] = ds.read();
    //Serial.print(data[i], HEX);
    //Serial.print(" ");
  }
  //Serial.println();
  timestamp = convertRawDS1904(&data[0]);
  //setArduinoTime(timestamp);
  //arduinoTime = getArduinoTime();
  Serial.println(timestamp);
  
  //setDS1904(timestamp, &data[0]);
  delay(1000);
}

void setArduinoTime(long time_stamp) {
  setTime(time_stamp);
}

String getArduinoTime() {
  String strTime;
  strTime = String(month()) + String("/") + String(day()) + String("/") + String(year()) + 
  String(" ") + String(hour()) + String(":") + String(minute()) + String(":") + String(second());
  return strTime;
  /*
  Serial.print(month());
  Serial.print("/");
  Serial.print(day());
  Serial.print("/");
  Serial.print(year()); 
  Serial.print(" ");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println();
  */
}

long convertRawDS1904(byte *d) {
  long a, b;
  a = d[4];
  a = a << 24;
  b = d[3];
  b = b << 16;
  a = a | b;
  b = d[2];
  b = b << 8;
  a = a | b;
  a = a | d[1];
  return a;
  //return (d[4] * 16777216) + (d[3] * 65536) + (d[2] * 256) + d[1];
}

void writeTime() {
  byte i;
  long a;
  byte b;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  long timeStamp = 0;
  String arduinoTime;
  
  if ( !ds.search(addr)) {
    ds.reset_search();
    Serial.println("Unable to find DS1904");
    delay(2000);
    return;
  }
  for( i = 0; i < 8; i++) {
    if (addr[i] < 10) {
      //Serial.print('0');
      //Serial.print(addr[i],HEX);
    } else {
      //Serial.print(addr[i], HEX);
    }
  }
/*
  if (OneWire::crc8(addr, 7) != addr[7]) {
      //Serial.println("CRC is not valid!");
      return;
  }
*/
  
  switch (addr[0]) {
    case 0x24:
      Serial.println("  Chip = DS1904");
      break;
    default:
      Serial.println("Device unknown.");
      return;
  } 

  timeStamp = now();
  data[0] = 0x0C; //Set control byte
  data[1] = timeStamp & 0xFF;
  a = timeStamp << 16;
  data[2] = a >> 24;
  a = timeStamp << 8;
  data[3] = a >> 24;
  data[4] = timeStamp >> 24;
  //Now write to the DS1904...
  ds.reset();
  ds.select(addr);
  ds.write(0x99);        // 0x66=Read; 0x99=Write
  delay(20);     // maybe 750ms is enough, maybe not
  for (i = 0;i < 5; i++) {
    ds.write(data[i]);
  }
  ds.reset();
  Serial.println("Set DS1904");
  delay(2000);
}
/*
void setDS1904(long timestamp, byte *d) {
  long a;
  byte b;
  d[0] = 0x0C; //Set control byte
  d[1] = timestamp & 0xFF; //LSB, DD
  a = timestamp << 16; //LShift 2 Bytes, CC DD 00 00
  d[2] = a >> 24; // RShift 3 Bytes 00 00 00 CC
  a = timestamp << 8; //Lshift 1 Byte BB CC DD 00
  d[3] = a >> 24; // RShift 3 Bytes 00 00 00 BB
  d[4] = timestamp >> 24; //RShift 3 Bytes 00 00 00 AA
}
*/
void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void writeSystemTime() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
     Serial.println("Setting time...");
     for (int i; i < 20000; i++) {
       //Do nothing...
     }
     pctime = Serial.parseInt();
     if( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
       setTime(pctime); // Sync Arduino clock to the time received on the serial port
     }
}

void readSystemTime() {
  String a;
  a = getArduinoTime();
  Serial.println(a);
}

time_t requestSync()
{
  Serial.write(TIME_REQUEST);  
  return 0; // the time will be sent later in response to serial mesg
}

