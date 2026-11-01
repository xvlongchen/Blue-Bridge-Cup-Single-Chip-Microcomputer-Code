#include "reg52.h"
#include "ds1302.h"

//无小数点
unsigned char code SMG_NoDot[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};

//ds1302读地址
unsigned char Read_addr[7]={0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};
//ds1302写地址
unsigned char Write_addr[7]={0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};
//初始化时间：2024年5月8日 23:01
unsigned char TIME[7]={0x34,0x01,0x23,0x08,0x05,0x03,0x24};

//实际时间
unsigned char hour=0;
unsigned char min=0;
unsigned char sec=0;
//===============================寄存器通道选择===========================//
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
//============================单个数码管动态显示===============//
//数码管延时函数
void DelaySMG(unsigned int t)
{
	while(t--);
}

void DisplaySMG_Bit(unsigned char pos,unsigned char value)
{
	Select_HC573(6,0x01<<pos);       //选择数码管
	Select_HC573(7,value);
	DelaySMG(500);
	Select_HC573(6,0x01<<pos);       //消影
	Select_HC573(7,0xff);
}
//=============================全部数码管静态显示=====================//
void DisplaySMG_All(unsigned char value)
{
	Select_HC573(6,0xff);    //使能所有数码管
	Select_HC573(7,value);   //数码管显示内容
}
//================================数码管显示函数=======================//
void Display_SMG()
{
	DisplaySMG_Bit(0,SMG_NoDot[hour>>4]);
	DisplaySMG_Bit(1,SMG_NoDot[hour&0x0f]);
	DisplaySMG_Bit(2,SMG_NoDot[16]);
	
	DisplaySMG_Bit(3,SMG_NoDot[min>>4]);
	DisplaySMG_Bit(4,SMG_NoDot[min&0x0f]);
	DisplaySMG_Bit(5,SMG_NoDot[16]);
	
	DisplaySMG_Bit(6,SMG_NoDot[(sec>>4)&0x07]);
	DisplaySMG_Bit(7,SMG_NoDot[sec&0x0f]);
}
//===============================初始化ds1302========================//
void Init_Ds1302()
{
	unsigned char i;
	Write_Ds1302_Byte(0x8e,0x00);       //解除写保护
	for(i=0;i<7;i++)
	{
		Write_Ds1302_Byte(Write_addr[i],TIME[i]);
	}
	Write_Ds1302_Byte(0x8e,0x80);      //使能写保护
}
//===============================读取ds1302===========================//
void Read_DS1302_Time()
{
	sec=Read_Ds1302_Byte(0x81);
	min=Read_Ds1302_Byte(0x83);
	hour=Read_Ds1302_Byte(0x85);
}
//===============================系统初始化================================//
void Init_System()
{
	Select_HC573(0,0x00);  //关闭所有锁存器
	Select_HC573(5,0x00);  //关闭蜂鸣器和继电器
	Select_HC573(4,0xff);  //关闭led
	DisplaySMG_All(0xff);
}
void main()
{
	Init_System();
	Init_Ds1302();
	while(1)
	{
		Read_DS1302_Time();
		Display_SMG();
	}
}