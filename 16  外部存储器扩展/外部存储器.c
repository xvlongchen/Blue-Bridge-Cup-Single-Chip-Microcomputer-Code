#include "reg52.h"
#include "absacc.h"

void Delay(unsigned int t)
{
	while(t--);
	while(t--);
}

void LEDRuning()
{
	XBYTE[0x8000]=0xf0;
	Delay(65535);
	Delay(65535);
	XBYTE[0x8000]=0x0f;
	Delay(65535);
	Delay(65535);
	XBYTE[0x8000]=0xff;
	Delay(65535);
	Delay(65535);
}

void  SMGRuning()
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		XBYTE[0xc000]=0x01<<i;
		XBYTE[0xe000]=0x00;
		Delay(65535);
		Delay(65535);
	}
	XBYTE[0xe000]=0xff;
	Delay(65535);
	Delay(65535);
}

void main()
{
	while(1)
	{
		LEDRuning();
		SMGRuning(); 
	}
}