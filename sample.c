// This code won't compile, it's just copy paste of functions that do transmission

//  ff FI IT TT HH SS GG ?R RR BD CC
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

#include <stdio.h>

int main()
{
    int i;
    unsigned char ref[9] = {0xa1, 0x82, 0x0e, 0x21, 0x02, 0x04, 0x00, 0x4e, 0x06};
    // sample transmission
    // id = 24 temp 12.6 hum 33% wind 135 wind avg 2.45 km/h wind gust 4.90 total rain 23.4 batt OK
    // as reported by rtlsdr
    
    for(i=0;i<sizeof(ref);i++) {
      printf("0x\%02.2hhx ", ref[i]);
    }
    
    printf("\n");
    unsigned char bytes[9] = {0};

    char dev_id = 24;
    int temp = 12.6 * 10 + 400;
    char humidity = 33;
    int rain = 78; // 23.4 / 0.3f
    
    bytes[0] = 0xA0 | dev_id >> 4;
    bytes[1] = dev_id << 4 | temp >> 8; // temp is 12 bit
    bytes[2] = temp; // remaining part
    bytes[3] = humidity; // in percent
    bytes[4] = 2; // wind speed in m/s * 1/3 e.g 3 = 1 m/s
    bytes[5] = 4; // wind speed in m/s * 1/3 e.g 3 = 1 m/s
    bytes[6] = rain >> 8; 
    bytes[7] = rain; 
    bytes[8] = 0x06; // wind direction 00 is N, 02 is NE, 04 is E, etc. up to 0F is SEE
    

    for(i=0;i<sizeof(bytes);i++) {
      printf("0x\%02.2hhx ", bytes[i]);
    }
    return 0;
}

