#include "reg52.h"
#include "onewire.h"

//数码管断码，无小数点
unsigned char code SMG_NODot[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};

unsigned int temp=0;      //温度

//=================================寄存器通道选择==================//
void Select_HC573(unsigned char channel,unsigned char value)
{
	P0=value;
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
//================================单个数码管显示==================//
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
//================================关闭所有数码管==================//
void offSMG_All(unsigned char value)
{
	Select_HC573(6,0xff);
	Select_HC573(7,value);
}
//============================数码管显示温度==================//
void Display_temp()
{
	DisplaySMG_bit(5,SMG_NODot[temp/100]);
	Delay_SMG(500);
	DisplaySMG_bit(6,SMG_NODot[(temp%100)/10]-0x80);
	Delay_SMG(500);
	DisplaySMG_bit(7,SMG_NODot[temp%10]);
	Delay_SMG(500);
}
//=============================读取温度=========================//
void Delay(unsigned int t)
{
	while(t--)
	{
		Display_temp();
	}
}
void Read_ds18b20_Temp()
{
	unsigned int LSB;
	unsigned int MSB;
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	
	Delay(100);
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	LSB=Read_DS18B20();
	MSB=Read_DS18B20();
	
	init_ds18b20();
	
	temp=MSB;
	temp=(temp<<8)|LSB;
	if((temp&0xf800)==0x0000)
	{
		temp=temp*0.625;
	}
}
//==============================初始化系统=======================//
void Init_System()
{
	Select_HC573(0,0x00);
	Select_HC573(5,0x00);
	Select_HC573(4,0xff);
	offSMG_All(0xff);
}
void main()
{
	Init_System();
	while(1)
	{
		Read_ds18b20_Temp();
		Display_temp();
	}
}