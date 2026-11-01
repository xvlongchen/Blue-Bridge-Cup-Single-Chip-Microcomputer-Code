#include "reg52.h"
#include "intrins.h"

unsigned char code SMG_duanma[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};

sfr AUXR=0x8e; 
sbit TX=P1^0;    //发射引脚
sbit RX=P1^1;    //接收引脚

unsigned int distance=0;
void Read_chaoshengb();

void Delay12us()		//@12.000MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}


void Delay_SMG(unsigned int t)
{
	while(t--);
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
	
	DisplaySMG_bit(SMG_duanma[distance/100],5);
	Delay_SMG(500);
	DisplaySMG_bit(SMG_duanma[(distance%100)/10],6);
	Delay_SMG(500);
	DisplaySMG_bit(SMG_duanma[distance%10],7);
	Delay_SMG(500);
	
	SelectHC573(6);
	P0=0xff;
	SelectHC573(7);
	P0=0xff;
}
//=============发出方波信号==============//
void Seng_maichong()
{
	char i;
	for(i=0;i<=7;i++)
	{
		TX=1;
		Delay12us();
		TX=0;
		Delay12us();
	}
}

//=============定时器计数功能============//
void Timer1Init()		//100微秒@12.000MHz
{
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0x00;		//设置定时初值
	TH1 = 0x00;		//设置定时初值
	TF1 = 0;		//清除TF1标志，计数溢出标志位
	TR1 = 0;		//定时器1开始计时
	
	EA=1;
	ET1=0;       //不用跳到定时服务函数中执行，所以为0关掉
}

//=================计算时间==============//
void Read_chaoshengb()    //计算时间并返回
{
	unsigned int num=0;
	Timer1Init();
	Seng_maichong();
	TR1=1;
	
	while((RX==1)&&(TF1==0));
	
	TR1 = 0;
	if(TF1==0)
	{
		num=TH1;
		num=(num<<8)|TL1;
		distance = ((num / 10) * 17) / 100 + 3;
	}
	else if(TF1==1)
	{
		distance=999;
		TF1=0;
	}
}

void Delay(unsigned char g)
{
	while(g--)
	{
		Display_Dynamic();
	}
}
void main()
{
	while(1)
	{
    Read_chaoshengb();
		Delay(10);
	}
}