//    ff FI IT TT HH SS GG ?R RR BD CC
//- F: 4 bit fixed message format
//- I: 8 bit device id
//- T: 12 bit temperature, offset 40 scale 10, i.e. 0.1C steps -40C (top 2 bits are sign, discard)
//- H: 8 bit humidity percent
//- S: 8 bit wind speed, 0.34m/s steps
//- G: 8 bit gust speed, 0.34m/s steps
//- R: 12 bit? rain, 0.3mm steps
//- B: 4 bit flags, 0x1 is battery_low
//- D: 8 bit wind direction: 00 is N, 02 is NE, 04 is E, etc. up to 0F is seems
//- C: 8 bit checksum
//

#include <Wire.h>
#include <Adafruit_BMP085.h>
#define seaLevelPressure_hPa 1013.25

Adafruit_BMP085 bmp;

#include <AHT10.h>
#include <Wire.h>

uint8_t readStatus = 0;

AHT10 myAHT10(AHT10_ADDRESS_0X38);

static int wind_dir_degr[]= {0, 23, 45, 68, 90, 113, 135, 158, 180, 203, 225, 248, 270, 293, 315, 338};

#define pin PORTD6

void send_byte(uint8_t byte)
{
  for(int i=0; i<8; i++) {
    int delay = 1500;
    digitalWrite(pin, HIGH);
    if (byte & 0x80)
      delay = 500;
    delayMicroseconds(delay);
    digitalWrite(pin, LOW);
    delayMicroseconds(1000);
    byte <<= 1;
  }
}

uint8_t crc8( uint8_t *addr, uint8_t len)
{
  uint8_t crc = 0;

  // Indicated changes are from reference CRC-8 function in OneWire library
  while (len--) {
    uint8_t inbyte = *addr++;
    for (uint8_t i = 8; i; i--) {
      uint8_t mix = (crc ^ inbyte) & 0x80; // changed from & 0x01
      crc <<= 1; // changed from right shift
      if (mix) crc ^= 0x31;// changed from 0x8C;
      inbyte <<= 1; // changed from right shift
    }
  }
  return crc;
}

void setup() {
  pinMode(pin, OUTPUT);
  Serial.begin(9600);
  if (!bmp.begin()) 
  {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }
  Serial.println("BMP go");

  while (myAHT10.begin() != true)
  {
    Serial.println(F("AHT10 not connected or fail to load calibration coefficient")); //(F()) save string to flash & keeps dynamic memory free
    delay(5000);
  }
  Serial.println(F("AHT10 OK"));
  
}

uint8_t crc;

// Sample good readings
//uint8_t bytes[] = {0xa1, 0x82, 0x4c, 0x37, 0x00, 0x00, 0x00, 0x58, 0x80}; // ok values
//uint8_t bytes[] = {0xa1, 0x82, 0x0e, 0x21, 0x02, 0x04, 0x00, 0x4e, 0x06}; // temp 12.6 hum 33% wind 135 wind avg 2.45 wind gust 4.90 total rain 23.4 batt OK
//uint8_t bytes[] = {0xa1, 0x82, 0x48, 0x38, 0x00, 0x00, 0x00, 0x58, 0x15}; // bad battery
  
uint8_t bytes[9] = {0};
char dev_id = 24;
int temp = 0 * 10 + 400;
char humidity = 0;
int rain = 0; // number of buckets with 0.3mm each 23.4 / 0.3f = 78
    
// the loop function runs over and over again forever
void loop() {
  dev_id = 24;
  temp = myAHT10.readTemperature() * 10 + 400;
  humidity = myAHT10.readHumidity();
  rain = 0; // 23.4 / 0.3f = 78
  
  bytes[0] = 0xA0 | dev_id >> 4;
  bytes[1] = dev_id << 4 | temp >> 8; // temp is 12 bit
  bytes[2] = temp; // remaining part
  bytes[3] = humidity; // in percent
  bytes[4] = 0; // wind speed in m/s * 1/3 e.g 3 = 1 m/s
  bytes[5] = 0; // wind speed in m/s * 1/3 e.g 3 = 1 m/s
  bytes[6] = rain >> 8; 
  bytes[7] = rain; 
  bytes[8] = 0x00; // wind direction 00 is N, 02 is NE, 04 is E, etc. up to 0F is SEE

  
  crc = crc8(bytes, sizeof(bytes));

  // Transmit signal
  send_byte(0xff); 
  for (int i=0; i<sizeof(bytes); i++)
    send_byte(bytes[i]);
  send_byte(crc);

  Serial.println();
  Serial.println();
  Serial.println();
  Serial.print("Temperature = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");
  
  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure());
  Serial.println(" Pa");

  Serial.print("Altitude = ");
  Serial.print(bmp.readAltitude());
  Serial.println(" meters");

  Serial.print("Pressure at sealevel (calculated) = ");
  Serial.print(bmp.readSealevelPressure());
  Serial.println(" Pa");

  Serial.print("Real altitude = ");
  Serial.print(bmp.readAltitude(seaLevelPressure_hPa * 100));
  Serial.println(" meters");
  
  Serial.println();

  Serial.print(F("Temperature: ")); Serial.print(myAHT10.readTemperature()); Serial.println(F(" +-0.3C")); //by default "AHT10_FORCE_READ_DATA"
  Serial.print(F("Humidity...: ")); Serial.print(myAHT10.readHumidity());    Serial.println(F(" +-2%"));   //by default "AHT10_FORCE_READ_DATA"
  
  delay(5000);
}
