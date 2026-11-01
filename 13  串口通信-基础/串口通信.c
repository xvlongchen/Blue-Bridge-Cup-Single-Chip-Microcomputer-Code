#include "reg52.h"

sfr AUXR=0x8e;            //51单片机中没有，蓝桥杯要用，所以定义一个辅助寄存器

unsigned char date;

void SendByte(unsigned char dat);
void InitUART()
{
	TMOD=0x20;
	TH1=0xfd;
	TL1=0xfd;
	TR1=1;
	
	SCON=0x50;
	AUXR=0x00;
	
	ES=1;
	EA=1;
}

void ServiceUART() interrupt 4
{
	if(RI==1)
	{
		RI=0;
		date=SBUF;
		SendByte(date+1);
	}
}

void SendByte(unsigned char dat)
{
	SBUF=dat;
	while(TI==0);
	TI=0;
}

void main()
{
	InitUART();
	SendByte(0x5a);
	SendByte(0xa5);
	while(1);
}