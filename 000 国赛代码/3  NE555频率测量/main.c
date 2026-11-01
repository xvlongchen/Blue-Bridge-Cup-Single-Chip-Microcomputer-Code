#include "reg52.h"

unsigned char code SMG_NODAT[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};

unsigned int count_1=0;      //10ms计时
unsigned int freq=0;         //频率

//====================寄存器通道选择================//
void Selcet_HC573(unsigned char channel,unsigned char dat)
{
	P0=dat;
	switch(channel)
	{
		case 4:P2=(P2&0x1f)|0x80;break;
		case 5:P2=(P2&0x1f)|0xa0;break;
		case 6:P2=(P2&0x1f)|0xc0;break;
		case 7:P2=(P2&0x1f)|0xe0;break;
		case 0:P2=(P2&0x1f)|0x00;break;
	}
	P2=(P2&0x1f)|0x00;
}
//=======================关闭所有数码管=============//
void OffSMG_All(unsigned char value)
{
	Selcet_HC573(6,0xff);
	Selcet_HC573(7,value);
}
//======================显示单个数码管==============//
//延时函数
void Delay_SMG(unsigned int t)
{
	while(t--)
	{
		t--;
	}
}

void DisplaySMG_Bit(unsigned char pos,unsigned char value)
{
	Selcet_HC573(6,0x01<<pos);
	Selcet_HC573(7,value);
	Delay_SMG(500);
	Selcet_HC573(6,0x01<<pos);
	Selcet_HC573(7,0xff);
}
//=====================数码管显示NE555频率==========//
void Display_SMG()
{
	if(freq>9999)
	{
		DisplaySMG_Bit(3,SMG_NODAT[(freq/10000)%10]);
	}
	if(freq>999)
	{
		DisplaySMG_Bit(4,SMG_NODAT[(freq/1000)%10]);
	}
	if(freq>99)
	{
		DisplaySMG_Bit(5,SMG_NODAT[(freq/100)%10]);
	}
	if(freq>9)
	{
		DisplaySMG_Bit(6,SMG_NODAT[(freq/10)%10]);
	}
	DisplaySMG_Bit(7,SMG_NODAT[freq%10]);
}
//======================定时器初始化================//
void Init_Timer()
{
	//定时器1：10ms
	TH1=(65536-10000)/256;
	TL1=(65536-10000)%256;
	//计数器0，初值为0x0000
	TH0=0;
	TL0=0;
	
	TMOD=0x15;
	
	ET0=1;
	ET1=1;
	EA=1;
	
	TR0=1;
	TR1=1;
}
//======================定时器T1的中断服务函数======//
void Service_Timer() interrupt 3
{
	TH1=(65536-10000)/256;
	TL1=(65536-10000)%256;
	count_1++;
	
	if(count_1==100)
	{
		count_1=0;
		freq=(TH0<<8)|TL0;
		TH0=0;
		TL0=0;
	}
}
//======================初始化系统==================//
void Init_System()
{
	Selcet_HC573(0,0x00);
	Selcet_HC573(5,0x00);
	Selcet_HC573(4,0xff);
	OffSMG_All(0xff);
}
void main()
{
	Init_System();
	Init_Timer();
	while(1)
	{
		Display_SMG();
	}
}