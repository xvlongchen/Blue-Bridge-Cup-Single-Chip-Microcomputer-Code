#ifndef _ONEWIRE_H
#define _ONEWIRE_H

#include "reg52.h"

#define OW_SKIP_ROM 0xcc
#define DS18B20_CONVERT 0x44
#define DS18B20_READ 0xbe

//ICÒı½Å¶¨Òå
sbit DQ = P1^4;

//º¯ÊıÉùÃ÷
void Delay_OneWire(unsigned int t);
void Write_DS18B20(unsigned char dat);     //å†™å‡½æ•°
bit Init_DS18B20(void);                   //å¤ä½å‡½æ•°
unsigned char Read_DS18B20(void);         //è¯»å‡½æ•°

#endif