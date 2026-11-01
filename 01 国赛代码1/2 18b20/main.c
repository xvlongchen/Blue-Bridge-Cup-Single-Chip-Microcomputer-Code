#include "reg52.h"
#include "onewire.h"

sbit S4=P3^3;
unsigned char code SMG_NODOt[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};
float temp_ds18b20=0;     //实际温度
unsigned int smg_ds18b20=0;   //数码管显示温度
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
void DisplaySMG_temp()
{
	switch(mode)
	{
		//显示一位小数
		case 1:
			smg_ds18b20=temp_ds18b20*10;
			DisplaySMG_bit(7,SMG_NODOt[smg_ds18b20%10]);
			DisplaySMG_bit(6,SMG_NODOt[(smg_ds18b20/10)%10]-0x80);
			if(smg_ds18b20/100!=0)
			{
				DisplaySMG_bit(5,SMG_NODOt[smg_ds18b20/100]);
			}
		break;
		//显示2位小数
		case 2:
			smg_ds18b20=temp_ds18b20*100;
			DisplaySMG_bit(7,SMG_NODOt[smg_ds18b20%10]);
			DisplaySMG_bit(6,SMG_NODOt[(smg_ds18b20/10)%10]);
			DisplaySMG_bit(5,SMG_NODOt[(smg_ds18b20/100)%10]-0x80);
			if(smg_ds18b20/1000!=0)
			{
				DisplaySMG_bit(4,SMG_NODOt[smg_ds18b20/1000]);
			}
		break;
			//显示整数
		case 3:
			smg_ds18b20=temp_ds18b20;
			DisplaySMG_bit(7,SMG_NODOt[smg_ds18b20%10]);
			if(smg_ds18b20/10!=0)
			{
				DisplaySMG_bit(6,SMG_NODOt[smg_ds18b20/10]);
			}
		break;	
	}
}
//===================读取温度================//
//读取温度
void Read_DS18b20_temp()
{
	unsigned int LSB;
	unsigned int HSB;
	unsigned int temp=0;
	
	init_ds18b20();
	Write_DS18B20(0xcc);    //忽略ROM操作
	Write_DS18B20(0x44);    //启动温度转换
	DisplaySMG_temp();
	
	init_ds18b20();
	Write_DS18B20(0xcc);    //忽略ROM操作
	Write_DS18B20(0xbe);    //温度读取
	LSB=Read_DS18B20();
	HSB=Read_DS18B20();
	DisplaySMG_temp();
	
	temp=HSB;
	temp=(temp<<8)|LSB;
	if((temp&0xf800)==0x0000)
	{
		temp_ds18b20=temp*0.0625;
	}
}
//====================按键扫描===============//
void Key_scan()
{
	if(S4==0)
	{
		Delay_SMG(500);
		if(S4==0)
		{
			while(S4==0)
			{
				DisplaySMG_temp();
			}
			if(mode==1)
			{
				mode=2;
			}
			else if(mode==2)
			{
				mode=3;
			}
			else if(mode==3)
			{
				mode=1;
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
	while(1)
	{
		DisplaySMG_temp();
		Read_DS18b20_temp();
		Key_scan();
	}
}