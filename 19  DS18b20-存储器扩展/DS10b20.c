#include "reg52.h"
#include "absacc.h"
#include "onewire.h"

unsigned char SMGNoDot_CA[10]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};    //没小数点
unsigned char SMGDot_CA[10]={0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};   //有小数点
unsigned int temp=0;

void Delay_SMG(unsigned int t)
{
	while(t--);
}

void DispaySMG_Bit(unsigned char pos,unsigned char dat)
{
	XBYTE[0xE000]=0xff;
	XBYTE[0xC000]=0x01<<pos;
	XBYTE[0xE000]=dat;
}

void DisplaySMG_all(unsigned char dat)
{
	XBYTE[0xC000]=0xff;
	XBYTE[0xE000]=dat;
}
void DisplaySMG_Temp()
{
	/*
  DispaySMG_Bit(7,SMGNoDot_CA[temp%10]);
	Delay_SMG(100);
  DispaySMG_Bit(6,SMGDot_CA[(temp%100)/10]);
	Delay_SMG(100);
	DispaySMG_Bit(5,SMGNoDot_CA[temp/100]);
	Delay_SMG(100);
	*/
	DispaySMG_Bit(7,SMGNoDot_CA[temp%10]);
	Delay_SMG(100);
  DispaySMG_Bit(6,SMGNoDot_CA[temp/10]);
	Delay_SMG(100);
	DispaySMG_Bit(5,0xff);
	Delay_SMG(100);
	
  DispaySMG_Bit(4,0xff);
	Delay_SMG(100);
  DispaySMG_Bit(3,0xff);
	Delay_SMG(100);
	DispaySMG_Bit(2,0xff);
	Delay_SMG(100);
	DispaySMG_Bit(1,0xff);
	Delay_SMG(100);
  DispaySMG_Bit(0,0xff);
	Delay_SMG(100);
	
	DisplaySMG_all(0xff);
}

void Delay(unsigned int t)
{
	while(t--)
	{
		DisplaySMG_Temp();
	}
}
void Read_DS18B20_TEMP()
{
	unsigned char LSB;
	unsigned char HSB;
	
	Init_DS18B20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	Delay(1000);
	
	Init_DS18B20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	LSB=Read_DS18B20();
	HSB=Read_DS18B20();
	
	temp=HSB;
	temp=(temp<<8)|LSB;
	temp>>=4;                            //无小数
/*	if((temp&0xf800)==0x00000)        //有小数
	{
		temp>>=4;
		temp=temp*10;
		temp=temp+(LSB & 0x0f)*0.0625;
	}
*/
}
void main()
{
	while(1)
	{
		Read_DS18B20_TEMP();
		DisplaySMG_Temp();
	}
}