# tx443-fineoffset

Sample code that transmit signal using FS1000A (TX443) with OOK modulation to Fineoffset WH1080 console from arduino

Parts of code from https://github.com/zagor/FineOffset


Example

```
Simple data transmission sequence to the Wh-1081 weather station. The transmission consists of: - 
1 byte of start 0xFF - 
9 byte of data - 
1 byte of Checksum (CRC8 with polynomial 100110001).
BIT 0 is a pulse of 1,5mS to 1 logic and 1mS to 0 logic (total 2,5mS) 
BIT 1 is a pulse of 0,5mS to 1 logic and 1mS to 0 logic (total 1,5mS) 
Example Transmission : 
START = 0XFF, ID DEVICE = 0XA2A, 
TEMP = 0X261, HUMIDITY = 0X3A, WIND = 0X03, BURST = 0X10, RAIN = 0X00, RAIN = 0X00, FLAG = 0X0, DIREZ = 0X4, CKS = 0X29.
```

```
ff FI IT TT HH SS GG ?R RR BD CC
ff - OOK header
F: 4 bit fixed message format
I: 8 bit device id
T: 12 bit temperature, offset 40 scale 10, i.e. 0.1C steps -40C (top 2 bits are sign, discard)
H: 8 bit humidity percent
S: 8 bit wind speed, 0.34m/s steps
G: 8 bit gust speed, 0.34m/s steps
R: 12 bit? rain, 0.3mm steps
B: 4 bit flags, 0x1 is battery_low
D: 4 bit wind direction: 00 is N, 02 is NE, 04 is E, etc. up to 0F is SEE
C: 8 bit checksum without header
```
```
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
}

uint8_t crc; 
uint8_t bytes[9] = {0};
char dev_id = 24;
int temp = 0 * 10 + 400;
char humidity = 0;
int rain = 0; // number of buckets with 0.3mm each 23.4 / 0.3f = 78
    
// the loop function runs over and over again forever
void loop() {
  dev_id = 24;
  temp = 22.5 * 10 + 400;
  humidity = 55;
  rain = 0;
  
  bytes[0] = 0xA0 | dev_id >> 4;
  bytes[1] = dev_id << 4 | temp >> 8; // temp is 12 bit
  bytes[2] = temp; // remaining part
  bytes[3] = humidity; // in percent
  bytes[4] = 0; // wind speed in m/s * 1/3 e.g 3 = 1 m/s
  bytes[5] = 0; // wind speed in m/s * 1/3 e.g 3 = 1 m/s
  bytes[6] = rain >> 8; // rain is 12 bit
  bytes[7] = rain; 
  bytes[8] = 0x10 | 0x00;  // 4 bit flags 0x1 is low battery and 4 bit wind direction 00 is N, 02 is NE, 04 is E, etc. up to 0F is SEE

  
  crc = crc8(bytes, sizeof(bytes));

  // Transmit signal
  send_byte(0xff); 
  for (int i=0; i<sizeof(bytes); i++)
    send_byte(bytes[i]);
  send_byte(crc);

  delay(5000);
}

```

Result from rtl_443
```
_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
time      : 2020-10-30 16:24:29
model     : Fine Offset Electronics WH1080/WH3080 Weather Station            Msg type  : 0             Station ID: 24
Temperature: 22.5 C      Humidity  : 55 %          Wind degrees: 0           Wind avg speed: 0.00
Wind gust : 0.00         Total rainfall: 0.0       Battery   : LOW           Integrity : CRC
```
