#include <reg51.h>

sbit HC138_A=P2^5;
sbit HC138_B=P2^6;
sbit HC138_C=P2^7;

void Delay(unsigned int t)
{
	while(t--);
	while(t--);
}

void InitHc138(unsigned char n)
{
		switch(n)
		{
			case 4:HC138_C=1;HC138_B=0;HC138_A=0;break;
			case 5:HC138_C=1;HC138_B=0;HC138_A=1;break;
			case 6:HC138_C=1;HC138_B=1;HC138_A=0;break;
			case 7:HC138_C=1;HC138_B=1;HC138_A=1;break;
		}
}

void OutPutP0(unsigned char channel,unsigned char dat)
{
		InitHc138(channel);
	  P0=dat;
}

void LEDRuning()
{
	unsigned char i;
	for(i=0;i<3;i++)
	{
		OutPutP0(4,0x00);
		P0=0x00;
		Delay(60000);
		Delay(60000);
		OutPutP0(4,0xff);
		Delay(60000);
		Delay(60000);
	}
	for(i=1;i<=8;i++)
	{
		OutPutP0(4,0xff<<i);
		Delay(60000);
		Delay(60000);
	}
	
	OutPutP0(5,0x10);
	Delay(60000);
	Delay(60000);
	OutPutP0(5,0x00);
	
	for(i=1;i<=8;i++)
	{
		OutPutP0(4,~(0xff<<i));
		Delay(60000);
		Delay(60000);
	}
	OutPutP0(5,0x40);        
	Delay(60000);
	Delay(60000);
  OutPutP0(5,0x00);
}

void InitSystem()
{
	InitHc138(5);
	
	P0=0x00;
}

void main()
{
	InitSystem();
	while(1)
	{
		LEDRuning();
	}
}