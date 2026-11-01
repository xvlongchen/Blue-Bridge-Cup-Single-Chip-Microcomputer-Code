#include "reg52.h"
#include "ds1302.h"

unsigned char hour=0;
unsigned char min=0;
unsigned char score=0;

unsigned char code SMG_NOdat[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};

//ds1302读地址
unsigned char Read_addr[7]={0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};
//ds1302写地址
unsigned char Write_addr[7]={0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};
//当前时间
unsigned char TIME[7]={0x34,0x52,0x23,0x22,0x05,0x03,0x24};
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
//=================显示当前温度=============//
void Display_TIME()
{
	DisplaySMG_bit(0,SMG_NOdat[hour>>4]);
	DisplaySMG_bit(1,SMG_NOdat[hour&0x0f]);
	DisplaySMG_bit(2,SMG_NOdat[16]);
	
	DisplaySMG_bit(3,SMG_NOdat[min>>4]);
	DisplaySMG_bit(4,SMG_NOdat[min&0x0f]);
	DisplaySMG_bit(5,SMG_NOdat[16]);
	
	DisplaySMG_bit(6,SMG_NOdat[(score>>4)&0x07]);
	DisplaySMG_bit(7,SMG_NOdat[score&0x0f]);
}
//==================初始化DS1302=============//
void Init_DS1302()
{
	unsigned char i;
	Write_Ds1302_Byte(0x8e,0x00);
	for(i=0;i<7;i++)
	{
		Write_Ds1302_Byte(Write_addr[i],TIME[i]);
	}
	Write_Ds1302_Byte(0x8e,0x80);
}
//==================读取时间==================
void Read_DS1302()
{
	hour=Read_Ds1302_Byte(0x85);
	min=Read_Ds1302_Byte(0x83);
	score=Read_Ds1302_Byte(0x81);
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
	Init_DS1302();
	while(1)
	{
		Read_DS1302();
		Display_TIME();
	}
}