#include <OneWire.h>
#include <Time.h>

// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

OneWire  ds(2);  // on pin 10 (a 4.7K resistor is necessary)

void setup(void) {
  Serial.begin(9600);
  //Serial.printKeyboard.begin();
}

void loop(void) {
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

  if (OneWire::crc8(addr, 7) != addr[7]) {
      //Serial.println("CRC is not valid!");
      return;
  }
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
  
  delay(100);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  //present = ds.reset();
  //ds.select(addr);    
  //ds.write(0xBE);         // Read Scratchpad

  //Serial.print("  Data = ");
  //Serial.print(present, HEX);
  //Serial.print(" ");
  for ( i = 0; i < 5; i++) {           // we need 5 bytes
    data[i] = ds.read();
    //Serial.print(data[i], HEX);
    //Serial.print(" ");
  }
  //Serial.println();
  //timestamp = data[4] << 24;
  //timestamp = data[4] * 16777216;
  timestamp = convertRawDS1904(&data[0]);
  //timestamp = (data[4] * 16777216) + (data[3] * 65536) + (data[2] * 256) + data[1];
  setArduinoTime(timestamp);
  //Serial.println(timestamp, DEC);
  arduinoTime = getArduinoTime();
  Serial.println(arduinoTime);
  
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

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

