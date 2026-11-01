#include "reg52.h"

unsigned char code SMG_duanma[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};
unsigned int count_f=0;
unsigned int dat_f=0;
unsigned char count_t=0;

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

void Delay_SMG(unsigned int t)
{
	while(t--);
}

void DisplaySMG_bit(unsigned char value,unsigned char pos)
{
	P0=0xff;
	SelectHC573(6);
	P0=0x01<<pos;
	SelectHC573(7);
	P0=value;
}

void Display_Dynamic()
{
	DisplaySMG_bit(SMG_duanma[15],0);
	Delay_SMG(500);
	DisplaySMG_bit(0xff,1);
	Delay_SMG(500);
	DisplaySMG_bit(0xff,2);
	Delay_SMG(500);

	if(dat_f>9999)
	{
		DisplaySMG_bit(SMG_duanma[dat_f/10000],3);
	  Delay_SMG(500);
	}
	if(dat_f>999)
	{
		DisplaySMG_bit(SMG_duanma[(dat_f/1000)%10],4);
	  Delay_SMG(500);
	}
	if(dat_f>99)
	{
		DisplaySMG_bit(SMG_duanma[(dat_f/100)%10],5);
	  Delay_SMG(500);
	}
	if(dat_f>9)
	{
		DisplaySMG_bit(SMG_duanma[(dat_f/10)%10],6);
	  Delay_SMG(500);
	}
	DisplaySMG_bit(SMG_duanma[dat_f%10],7);
	Delay_SMG(500);
}

void Init_Timer()
{
	//定时器0用作计数，8位重装模式
	TH0=0xff;
	TL0=0xff;
	
	//定时器1用作定时，定时时间50ms
	TH1=(65536-50000)/256;
	TL1=(65536-50000)%256;
	
	TMOD=0x16;  //0001 0110  TOMD定时器高四位为定时/计数器1，第四位为定时计数器0
	
	ET0=1;
	ET1=1;
	EA=1;
	
	TR0=1;
	TR1=1;
}

void Service_T0() interrupt 1      //T0作用计算脉冲个数，最大值为255，每加1就会产生溢出，溢出后给一个值自加
{
	count_f++;
}

void Service_T1() interrupt 3      //T1作用定时
{
	TH1=(65535-50000+1)/256;
	TL1=(65536-50000)%256;
	
	count_t++;
	if(count_t==20)
	{
		dat_f=count_f;
		count_f=0;
		count_t=0;
	}
}

void Init_system()
{
	SelectHC573(5);
	P0=0x00;
	SelectHC573(4);
	P0=0xff;
}
void main()
{
	Init_system();
	Init_Timer();
	while(1)
	{
		Display_Dynamic();
	}
}