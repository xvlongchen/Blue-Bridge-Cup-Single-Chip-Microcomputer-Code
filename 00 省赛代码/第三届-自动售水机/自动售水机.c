#include "reg52.h"

unsigned char code SMG_duanma_NO[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};
unsigned char code SMG_duanma[16]={0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10,0x08,0x03,0x46,0x21,0x06,0x0e};

sbit s7=P3^0;
sbit s6=P3^1;
unsigned int water_OFF_NO=0;   //水流状态，0打开，1关闭
unsigned int xianshi=0; 
unsigned int SMG_TIME_TRO=0;

void Display_SMG();
void Display_different_SMG();

void SelectHC573(unsigned char channel)
{
	switch(channel)
	{
		case 4:P2=(P2&0x1f)|0x80;break;
		case 5:P2=(P2&0x1f)|0xa0;break;
		case 6:P2=(P2&0x1f)|0xc0;break;
		case 7:P2=(P2&0x1f)|0xe0;break;
		case 0:P2=(P2&0x1f)|0x00;break;
	}
}
//======================延时函数=========//
void Delay_KEY(unsigned int t)
{
	while(t--)
	{
		Display_SMG();
	}
}

void Delay_SMG(unsigned int t)
{
	while(t--);
}
//=============数码管定义============//
void Init_SMG(unsigned char pos,unsigned char dat)
{
	P0=0xff;
	SelectHC573(6);
	P0=0x01<<pos;
	SelectHC573(7);
	P0=dat;
}

void Display_SMG()
{
	Init_SMG(1,SMG_duanma[0]);
	Delay_SMG(100);
	Init_SMG(2,SMG_duanma_NO[5]);
	Delay_SMG(100);
	Init_SMG(3,SMG_duanma_NO[0]);
	Delay_SMG(100);
	
	Init_SMG(4,SMG_duanma_NO[xianshi/1000]);
	Delay_SMG(100);
	Init_SMG(5,SMG_duanma[(xianshi%1000)/100]);
	Delay_SMG(100);
	Init_SMG(6,SMG_duanma_NO[((xianshi%1000)%100)/10]);
	Delay_SMG(100);
	Init_SMG(7,SMG_duanma_NO[((xianshi%1000)%100)%10]);
	Delay_SMG(100);
}
//===================定时器=============//
void Init_Timer()
{
	TMOD=0x01;
	TH0=(65535-10000)/256;
	TL0=(65535-10000)%256;
	
	EA=1;
	ET0=1;
	TR0=0;
}

void ServiceTimer0() interrupt 1
{
	TH0=(65535-10000)/256;
	TL0=(65535-10000)%256;
	
	SMG_TIME_TRO++;  //0.01秒，每0.01秒进入1mL的水
}
//===================按键扫描======================//
void Scan_Key()      
{
	if(s7==0)
	{
		Delay_KEY(100);
		if(s7==0)
		{
			TR0=1;
			water_OFF_NO=0;
			SelectHC573(5);
			P0=0x10;
			SelectHC573(0);
		}
	}
	if(s6==0)
	{
		Delay_KEY(100);
		if(s6==0)
		{
			TR0=0;
			water_OFF_NO=1;
			SelectHC573(5);
			P0=0x00;
			SelectHC573(0);
		}
	}
}
//==================数码管显示不同状态=======================//
void Display_different_SMG()
{
	if(water_OFF_NO==0)
	{
		xianshi=SMG_TIME_TRO;
		if(xianshi==9999)
		{
			water_OFF_NO=1;
		}
	}
	else
	{
		while(s6==0)
		{
			xianshi=SMG_TIME_TRO*0.5;
		}
		SMG_TIME_TRO=0;
	}
}
//==============================================================//
void IninSystem()
{
	SelectHC573(5);
	P0=0x00;
	SelectHC573(4);
	P0=0xff;
	SelectHC573(0);
	
}

void main()
{
	IninSystem();
	Init_Timer();
	while(1)
	{
		Scan_Key();
		Display_SMG();
		Display_different_SMG();
	}
}