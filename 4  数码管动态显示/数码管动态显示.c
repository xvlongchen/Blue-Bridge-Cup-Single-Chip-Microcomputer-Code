#include "reg52.h"

unsigned char code SMG_duanma[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};
unsigned char yu=1;
void Display_Dynamic();

void Delay_SMG(unsigned int t)
{
	while(t--);
}

void Delay_yu(unsigned int t)
{
	while(t--)
	{
		Display_Dynamic();
	}
}
void SelectHC573(unsigned char n)
{
	switch(n)
	{
		case 4:P2=(P2&0x1f)|0x80;break;
		case 5:P2=(P2&0x1f)|0xa0;break;
		case 6:P2=(P2&0x1f)|0xc0;break;
		case 7:P2=(P2&0x1f)|0xe0;break;
	}
}

void DisplaySMG_bit(unsigned char value,unsigned char pos)
{
	SelectHC573(6);
	P0=0x01<<pos;
	SelectHC573(7);
	P0=value;
}

void Display_Dynamic()
{
	DisplaySMG_bit(SMG_duanma[2],0);
	Delay_SMG(500);
	DisplaySMG_bit(SMG_duanma[0],1);
	Delay_SMG(500);
	DisplaySMG_bit(SMG_duanma[1],2);
	Delay_SMG(500);
	DisplaySMG_bit(SMG_duanma[8],3);
	Delay_SMG(500);
	DisplaySMG_bit(SMG_duanma[16],4);
	Delay_SMG(500);
	DisplaySMG_bit(SMG_duanma[16],5);
	Delay_SMG(500);
	DisplaySMG_bit(SMG_duanma[yu/10],6);
	Delay_SMG(500);
	DisplaySMG_bit(SMG_duanma[yu%10],7);
	Delay_SMG(500);
	
}

void main()
{
	while(1)
	{
		Display_Dynamic();
		yu++;
		if(yu>12)
		{
			yu=1;
		}
		Delay_yu(100);
	}
}