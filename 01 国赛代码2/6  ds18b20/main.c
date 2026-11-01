#include "reg52.h"
#include "onewire.h"

unsigned char code SMG_NODAT[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e};

float temp_ds18b20=0;  //实际温度
unsigned int smg_ds18b20;   //数码管显示温度
unsigned char mode=1;
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
//===================数码管显示温度==========//
void Display_temp()
{
	switch(mode)
	{
		//整数
		case 1:
			smg_ds18b20=temp_ds18b20;
			DisplaySMG_bit(7,SMG_NODAT[smg_ds18b20%10]);
			if(smg_ds18b20/10!=0)
			{
				DisplaySMG_bit(6,SMG_NODAT[smg_ds18b20/10]);
			}
	}
}
//==================ds18b20温度读取与显示====//
void Read_Display_ds18b20()
{
	unsigned char LSB,MSB;
	unsigned int tmp=0;
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	
	Display_temp();
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	Display_temp();
	
	LSB=Read_DS18B20();
	MSB=Read_DS18B20();
	
	tmp=MSB;
	tmp=(tmp<<8)|LSB;
	if((tmp&0xf800)==0x0000)
	{
		temp_ds18b20=tmp*0.0625;
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
	while(1)
	{
		Read_Display_ds18b20();
		Display_temp();
	}
}