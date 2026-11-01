#include "reg52.h"
#include "onewire.h"
#include "iic.h"

unsigned char SMGNoDot_duanma[10]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};
unsigned char SMGDot_duanma[10]={0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};

unsigned char mode=1;   //1:温度显示界面,2:参数设置界面,3:DAC输出界面

unsigned int temp=0;   //温度
unsigned char Key_num;   //按键
unsigned char canshu=25;  //参数界面设置
unsigned char dianya;     //输出电压
unsigned char dianya_mode=1;   //输出电压模式

sfr P4=0xC0;

sbit R1=P3^0;
sbit R2=P3^1;
sbit R3=P3^2;
sbit R4=P3^3;

sbit C4=P3^4;
sbit C3=P3^3;
sbit C2=P4^2;
sbit C1=P4^4;
void Scan_Key();
//=======================通道选择==================
void Select_HC573(unsigned char channel)
{
	switch(channel)
	{
		case 4:P2=(P2&0x1f)|0x80;break;
		case 5:P2=(P2&0x1f)|0xa0;break;
		case 6:P2=(P2&0x1f)|0xc0;break;
		case 7:P2=(P2&0x1f)|0xe0;break;
	}
}
//=========================延时函数================
void Delay_smg(unsigned char t)
{
	while(t--);
	while(t--);
}
//=======================数码管初始化==============
void Init_smg(unsigned char pos,unsigned char value)
{
	P0=0xff;
	Select_HC573(6);
	P0=0x01<<pos;
	Select_HC573(7);
	P0=value;
}
//=======================数码管显示================
void Display_smg()
{
	switch(mode)
	{
		case 1:
			Init_smg(0,0xc6);
			Delay_smg(100);
			Init_smg(7,SMGNoDot_duanma[temp%10]);
			Delay_smg(100);
			Init_smg(6,SMGNoDot_duanma[(temp%100)/10]);
			Delay_smg(100);
			Init_smg(5,SMGDot_duanma[(temp%1000)/100]);
			Delay_smg(100);
			Init_smg(4,SMGNoDot_duanma[temp/1000]);
			Delay_smg(100);
		break;
		
		case 2:
			Init_smg(0,0x8c);
			Delay_smg(100);
			Init_smg(7,SMGNoDot_duanma[canshu%10]);
			Delay_smg(100);
			Init_smg(6,SMGNoDot_duanma[canshu/10]);
			Delay_smg(100);
		break;
		
		case 3:
			Init_smg(0,0x88);
			Delay_smg(100);
			Init_smg(7,SMGNoDot_duanma[dianya%10]);
			Delay_smg(100);
			Init_smg(6,SMGNoDot_duanma[(dianya%100)/10]);
			Delay_smg(100);
			Init_smg(5,SMGDot_duanma[dianya/100]);
			Delay_smg(100);
		break;
	}
	
	Select_HC573(6);
	P0=0xff;
	Select_HC573(7);
	P0=0xff;
}
//=======================DS18b20测量温度===========
void Delay_temp(unsigned int t)
{
	while(t--)
	{
		Display_smg();
	}
}

void Read_DS18b20_Temp()
{
	unsigned int LSB;
	unsigned int HSB;
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	
	Delay_temp(10);
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	LSB=Read_DS18B20();
	HSB=Read_DS18B20();

//一位小数
//	temp=HSB;
//	temp=(temp<<8)|LSB;
//	if((temp&0xf800)==0x0000)
//	{
//		temp>>=4;
//		temp=temp*10;
//		temp=temp+(LSB&0x0f)*6.25;
//	}

//两位小数
	temp=HSB;
	temp=(temp<<8)|LSB;
	temp=temp*25;
	temp=temp>>2;
}
//=======================DA输出====================
void Read_PCF8591_AIN3(unsigned char dat)
{
	IIC_Start();
	IIC_SendByte(0x90);      //PCF8591写设备地址
	IIC_WaitAck();
	
	IIC_SendByte(0x40);          //输出DAC
	IIC_WaitAck();
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}
//=======================按键扫描==================
void Delay_key(unsigned char k)
{
	while(k--);
}

void Scan_Key()
{
	C1=0;
	C2=C3=C4=1;
	R1=R2=R3=R4=1;
	if(R4==0)
	{
		Delay_key(100);
		if(R4==0)
		{
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
			while(R4==0)
			{
				Display_smg();
			}
  	}
	}
	if(mode==2)
	{
		C2=0;
		C1=C3=C4=1;
		R1=R2=R3=R4=1;
		if(R4==0)
		{
			Delay_key(100);
			canshu=canshu-1;
			while(R4==0)
			{
				Display_smg();
			}
		}
		else if(R3==0)
		{
			Delay_key(100);
			canshu=canshu+1;
			while(R3==0)
			{
				Display_smg();
			}
		}
  }
}
//=======================系统初始化================
void Init_System()
{
	Select_HC573(5);
	P0=0x00;
	Select_HC573(4);
	P0=0xff;
}

void main()
{
	Init_System();
	while(1)
	{
		if(mode==3)
		{
			C1=0;
			C2=C3=C4=1;
			R1=R2=R3=R4=1;
			if(R3==0)
			{
				Delay_key(10);
				if(R3==0)
				{
					if(dianya_mode==1)
					{
						dianya_mode=2;
						if(temp<canshu)
						{
								Read_PCF8591_AIN3((dianya/10)*51);
								dianya=0.00;
						}
						else
						{
							Read_PCF8591_AIN3((dianya/10)*51);
							dianya=500;
						}
					}
					else if(dianya_mode==2)
						{
							dianya_mode=1;
							if(temp>40)
							{
								Read_PCF8591_AIN3((dianya/10)*51);
								dianya=400;
							}
							else if(temp<20)
							{
								Read_PCF8591_AIN3((dianya/10)*51);
								dianya=100;
							}
							else if(temp>=20&&temp<=40)
							{
								Read_PCF8591_AIN3((dianya/10)*51);
								dianya=(15*temp)-200;
							}
						}
					while(R3==0)
					{
						Display_smg();
					}
				}
			}
		}
		Read_DS18b20_Temp();
		Display_smg();
		Scan_Key();
	}
}