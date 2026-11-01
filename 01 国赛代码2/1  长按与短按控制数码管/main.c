#include "reg52.h"

unsigned char code SMG_NODAT[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e};

sbit S4=P3^3;
//数码管控制数
unsigned char num=28;

unsigned char count_f=0;       //50ms计数
unsigned char F_key=0;
//===================寄存器通道选择============//
void Select_HC573(unsigned char channel,unsigned char dat)
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
//================单个数码管显示===============//
//延时函数
void Delay_SMG(unsigned int t)
{
	while(t--);
}
void DisplaySMG_bit(unsigned char pos,unsigned char value)
{
	Select_HC573(6,0x01<<pos);
	Select_HC573(7,value);
	Delay_SMG(500);
	Select_HC573(6,0x01<<pos);
	Select_HC573(7,0xff);
}
//================关闭全部数码管=============//
void OffSMG_All(unsigned char value)
{
	Select_HC573(6,0xff);
	Select_HC573(7,value);
}
//===============数码管显示按键控制数========//
void DisplaySMG_anjian()
{
	DisplaySMG_bit(7,SMG_NODAT[num%10]);
	DisplaySMG_bit(6,SMG_NODAT[num/10]);
}
//===================定时器0初始化===========//
void Init_Timer0()
{
	TH0=(0-50000)/256;
	TL0=(0-50000)%256;
	
	ET0=1;
	EA=1;
	TR0=1;
}
//===================定时器0服务函数=========//
void Service_Timer0() interrupt 1
{
	TH0=(0-50000)/256;
	TL0=(0-50000)%256;
	
	if(F_key==1)
	{
		count_f++;
	}
}
//=====================按键扫描函数==========//
void Key_scan()
{
	if(S4==0)
	{
		Delay_SMG(500);
		if(S4==0)
		{
			count_f=0;
			F_key=1;
			while(S4==0)
			{
				DisplaySMG_anjian();
			}
			F_key=0;
			
			if(count_f>20)
			{
				num=0;
			}
			else
			{
				num++;
				if(num==100)
				{
					num=0;
				}
			}
		}
	}
}
//====================系统初始化=============//
void Init_System()
{
	Select_HC573(0,0x00);
	Select_HC573(5,0x00);
	Select_HC573(4,0xff);
	OffSMG_All(0xff);
}
//==================主函数===================//
void main()
{
	Init_System();
	Init_Timer0();
	while(1)
	{
		DisplaySMG_anjian();
		Key_scan();
	}
}