#include "reg52.h"

sbit S7=P3^0;
sbit S6=P3^1;
sbit S5=P3^2;
sbit S4=P3^3;

sbit LED1=P0^0;
sbit LED2=P0^1;
sbit LED3=P0^2;
sbit LED4=P0^3;
sbit LED5=P0^4;
sbit LED6=P0^5;
sbit LED7=P0^6;

void Delay(unsigned char y)
{
	while(y--);
}
void SelectHC573(unsigned int t)
{
	switch(t)
	{
		case 4:P2=(P2&0x1f)|0x80;break;
		case 5:P2=(P2&0x1f)|0xa0;break;
		case 6:P2=(P2&0x1f)|0xc0;break;
		case 7:P2=(P2&0x1f)|0xe0;break;
	}
}

void ScanKeys_Alone()
{
	if(S7==0)
	{
		Delay(100);
		if(S7==0)
		{
			LED1=0;
			while(S7==0);
			LED1=1;
		}
	}
	if(S6==0)
	{
		Delay(100);
		if(S6==0)
		{
			LED2=0;
			while(S6==0);
			LED2=1;
		}
	}
	if(S5==0)
	{
		Delay(100);
		if(S5==0)
		{
			LED3=0;
			while(S5==0);
			LED3=1;
		}
	}
	if(S4==0)
	{
		Delay(100);
		if(S4==0)
		{
			LED4=0;
			while(S4==0);
			LED4=1;
		}
	}
}

void InitSystem()
{
	SelectHC573(5);
	P0=0xaf;
}
void main()
{
	InitSystem();
	SelectHC573(4);
	while(1)
	{
		ScanKeys_Alone();
	}
}