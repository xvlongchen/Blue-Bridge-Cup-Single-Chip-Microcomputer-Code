#include "reg52.h"

sbit S4=P3^3;

unsigned char code SMG_NoDot[18]={0xc0,0xf9,
    0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
    0x88,0x83,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};

unsigned char num=28;           //定义计数变量
unsigned char F_key=0;          //按键按下标志
unsigned char count_t=0;        //10ms单位累计

//==================寄存器通道选择=======
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
//===================数码管===============
void Delay_smg(unsigned char t)
{
	while(t--);
}

//数码管初始化
void Init_smg(unsigned char pos,unsigned char value)
{
	Select_HC573(7,0xff);
	Select_HC573(6,0x01<<pos);
	Select_HC573(7,value);
}

//数码管显示
void Display_smg()
{
	Init_smg(6,SMG_NoDot[num/10]);
	Delay_smg(100);
	Init_smg(7,SMG_NoDot[num%10]);
	Delay_smg(100);
}
//===================定时器0==============
//定时器初始化
void Init_Timer0()
{
	TMOD=0x01;
	
	TH0=(65536-10000)/256;
	TL0=(65536-10000)%256;
	
	ET0=1;
	EA=1;
	TR0=1;
}

//定时器服务函数
void Service_Timer0() interrupt 1
{
	TH0=(65536-10000)/256;
	TL0=(65536-10000)%256;
	
	if(F_key==1)      //在按键按下期间进行计数
	{
		count_t++;
	}
}
//====================扫描按键============
void Delay_Key(unsigned int t)
{
	while(t--);
}

void Scan_Key3()
{
	if(S4==0)
	{
		Delay_Key(300);
		if(S4==0)
		{
			count_t=0;
			F_key=1;
			while(S4==0)
			{
				Display_smg();
			}
			F_key=0;           //标志按键松开状态
			
			if(count_t>100)
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
//===================系统初始化===========
void Init_System()
{
	Select_HC573(5,0x00);
	Select_HC573(4,0xff);
	Select_HC573(6,0xff);
	Select_HC573(7,0xff);
}
//===================主函数============
void main()
{
	Init_Timer0();
	Init_System();
	while(1)
	{
		Display_smg();
		Scan_Key3();
	}
}