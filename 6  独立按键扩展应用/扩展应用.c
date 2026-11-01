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
sbit LED8=P0^7;

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

void Delay(unsigned char y)
{
	while(y--);
}

unsigned char stat_k=0;     //为0，S6、S7都没有按下

void ScanKeys_Alone()
{
	if(S7==0)
	{
		Delay(100);
		if(S7==0)
		{
			if(stat_k==0)
			{
				LED1=0;
				stat_k=1;               //状态改变
			}
			else if(stat_k==1)
			{
				LED1=1;
				stat_k=0;
			}
			while(S7==0);
		}
	}
	if(S6==0)
	{
		Delay(100);
		if(S6==0)
		{
			if(stat_k==0)
			{
				LED2=0;
				stat_k=2;
			}
			else if(stat_k==2)
			{
				LED2=1;
				stat_k=0;
			}
			while(S6==0);
		}
	}
	if(S5==0)
	{
		Delay(100);
		if(S5==0)
		{
			if(stat_k==1)
			{
				LED3=0;
				while(S5==0);
				LED3=1;
			}
			else if(stat_k==2)
			{
				LED5=0;
				while(S4==0);
				LED5=1;
			}
		}
	}
	if(S4==0)
	{
		Delay(100);
		if(S4==0)
		{
			if(stat_k==1)
			{
				LED4=0;
				while(S4==0);
				LED4=1;
			}
			else if(stat_k==2)
			{
				LED6=0;
				while(S5==0);
				LED6=1;
			}
		}
	}
}

void main()
{
	SelectHC573(4);
	while(1)
	{
		ScanKeys_Alone();
	}
}