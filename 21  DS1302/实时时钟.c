#include "reg52.h"
#include "ds1302.h"
//写寄存器地址
unsigned char Write_DS1302_adrr[7]={0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};
//读寄存器地址
unsigned char Read_DS1302_adrr[7]={0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};
//事实是间具体数值2024/3/16/ 13:28 43秒 周六 （秒，分，时，日，月，星期，年）
unsigned char Timer[7]={0x43,0x28,0x13,0x16,0x03,0x06,0x24};
unsigned char code SMG_duanma[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};

void HC573(unsigned char channel)
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

void Delay(unsigned char t)
{
	while(t--);
	while(t--);
}

void DS1302_config()  //初始化日历时钟
{
	char i;
	Write_Ds1302_Byte(0x8e,0x00);
	for(i=0;i<7;i++)
	{
		Write_Ds1302_Byte( Write_DS1302_adrr[i],Timer[i]);
	}
	Write_Ds1302_Byte(0x8e,0x80);
}

void Read_DS1302_Timer()  //读时间
{
	char j;
	for(j=0;j<7;j++)
	{
		Timer[j]=Read_Ds1302_Byte(Read_DS1302_adrr[j]);
	}
}
void Init_SMG(unsigned char pos,unsigned char value)
{
	P0=0xff;
	HC573(6);
	P0=0x01<<pos;
	HC573(7);
	P0=value;
}

void Display_SMG()
{
	//时
	Init_SMG(0,SMG_duanma[Timer[2]/16]);
	Delay(100);
	Init_SMG(1,SMG_duanma[Timer[2]%16]);
	Delay(100);
	
	//-
	Init_SMG(2,SMG_duanma[16]);
	Delay(100);
	
	//分
	Init_SMG(3,SMG_duanma[Timer[1]/16]);
	Delay(100);
	Init_SMG(4,SMG_duanma[Timer[1]%16]);
	Delay(100);
	
	//-
	Init_SMG(5,SMG_duanma[16]);
	Delay(100);
	
	//秒
	Init_SMG(6,SMG_duanma[Timer[0]/16]);
	Delay(100);
	Init_SMG(7,SMG_duanma[Timer[0]%16]);
	Delay(100);
}

void Init_System()
{
	HC573(5);
	P0=0x00;
	HC573(0);
}
void main()
{
	Init_System();
	DS1302_config();
	while(1)
	{
		Read_DS1302_Timer();
		Display_SMG();
	}
}