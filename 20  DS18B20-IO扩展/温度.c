#include "reg52.h"
#include "onewire.h"

unsigned char SMGNoDot_duanma[10]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};
unsigned char SMGDot_duanma[10]={0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};
unsigned int temp=0;

void SelectHC573(unsigned int channel)
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

void Delay_SMG(unsigned int t)
{
	while(t--);
}

void Display_SMg_Bit(unsigned char pos,unsigned char dat)
{
//	SelectHC573(7);
	P0=0xff;
	SelectHC573(6);
	P0=0x01<<pos;
	SelectHC573(7);
	P0=dat;
}

void Display_SMg_Temp()
{
	Display_SMg_Bit(7,SMGNoDot_duanma[temp%10]);
	Delay_SMG(100);
	Display_SMg_Bit(6,SMGDot_duanma[(temp%100)/10]);
	Delay_SMG(100);
	Display_SMg_Bit(5,SMGNoDot_duanma[temp/100]);
	Delay_SMG(100);
	
//	Display_SMg_Bit(4,0xff);
//	Delay_SMG(100);
//	Display_SMg_Bit(3,0xff);
//	Delay_SMG(100);
//	Display_SMg_Bit(2,0xff);
//	Delay_SMG(100);
//	Display_SMg_Bit(1,0xff);
//	Delay_SMG(100);
//	Display_SMg_Bit(0,0xff);
//	Delay_SMG(100);
}

void Delay_temp(unsigned int t)
{
	while(t--)
	{
		Display_SMg_Temp();
	}
}

void Read_DS18B20_Temp()
{
	unsigned int LSB;
	unsigned int MSB;
	
	Init_DS18B20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	
	Delay_temp(1000);
	
	Init_DS18B20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	LSB=Read_DS18B20();
	MSB=Read_DS18B20();
	Init_DS18B20();
	
	temp=MSB;
	temp=(MSB<<8)|LSB;
	if((temp&0xf800)==0x0000)
	{
		temp>>=4;
		temp=temp*10;
		temp=temp+(LSB&0x0f)*0.625;
	}
}

void InitSystem()
{
	SelectHC573(4);
	P0=0xff;
}

void main()
{
	InitSystem();
	while(1)
	{
		Read_DS18B20_Temp();
		Display_SMg_Temp();
	}
}