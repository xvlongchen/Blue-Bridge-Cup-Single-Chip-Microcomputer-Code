#include "reg52.h"

sbit L1=P0^0;
sbit L8=P0^7;

void Delay(unsigned int t)
{
	while(t--);
	while(t--);
}
void SelectHC573()
{
	P2=(P2&0x1f)|0x80;
}

void Working()
{
	L1=0;
	Delay(50000);
	L1=1;
	Delay(50000);
}

//*******************中断***********************//
void Init_INT0()
{
	IT0=1;
	EX0=1;
	EA=1;
	
}

void ServiceINT0() interrupt 0
{
	L8=0;
	Delay(50000);
	Delay(50000);
	Delay(50000);
	Delay(50000);
	Delay(50000);
	Delay(50000);
	L8=1;
	Delay(50000);
	Delay(50000);
	Delay(50000);
	Delay(50000);
	Delay(50000);
}
//**********************************************//
void main()
{
	Init_INT0();
	SelectHC573();
	while(1)
	{
		Working();
	}
}