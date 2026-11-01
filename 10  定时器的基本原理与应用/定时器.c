#include "reg52.h"

sbit LED1=P0^0;
sbit LED8=P0^7;

void SelectHC573()
{
	P2=(P2&0x1f)|0x80;
}

//********定时器**********//
void InitTime() //初始化函数
{
	TMOD=0x01;
	TH0=(65535-50000)/256;
	TL0=(65535-50000)%256;
	
	ET0=1;
	EA=1;
	TR0=1;
}

unsigned char count=0;

void ServiceTimer() interrupt 1
{
	TH0=(65535-50000)/256;
	TL0=(65535-50000)%256;
	
	count++;
	if(count%10==0)
	{
		LED1=~LED1;
	}
	if(count%100==0)
	{
		LED8=~LED8;
	}
}
//************************//
void main()
{
	SelectHC573();
	InitTime();
	while(1)
	{
		
	}
}