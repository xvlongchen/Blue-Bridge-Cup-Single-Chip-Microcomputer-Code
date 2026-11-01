#include "reg52.h"
#include "iic.h"

unsigned char code smg_NO_duanma[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,
		0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};
unsigned char code smg_duanma[10]={0x40,0x79,
    0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};

unsigned int count_f=0;
unsigned int dat_f=0;
unsigned int count_t=0;
		
unsigned char mode=1;    //1：频率显示界面  2：周期显示界面   3:电压显示界面
unsigned int zhouqi;

unsigned char adc1_value=0;
float adc1_volt=0;
unsigned int smg1_volt=0;

unsigned char adc3_value=0;
float adc3_volt=0;
unsigned int smg3_volt=0;

unsigned char smg_volt;
unsigned char channel_num=1;
		
unsigned char dat1=0;
unsigned char dat2=0;
sbit S4=P3^3;
sbit S5=P3^2;
sbit S6=P3^1;
sbit S7=P3^0;		

void Delay_Key(unsigned char k);
//==================锁存器通道选择=============
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
//===================数码管====================
void Delay_smg(unsigned int t)
{
	while(t--);
}
void Init_smg(unsigned char pos,unsigned char value)
{
	P0=0xff;
	Select_HC573(6);
	P0=0x01<<pos;
	Select_HC573(7);
	P0=value;
}

void Display_smg()
{
	switch(mode)
	{
		case 1:
		
			Init_smg(0,0x8e);
			Delay_smg(100);
			if(dat_f>9999)
			{
				Init_smg(3,smg_NO_duanma[(dat_f%100000)/10000]);
				Delay_smg(100);
			}
			if(dat_f>999)
			{
				Init_smg(4,smg_NO_duanma[(dat_f%10000)/1000]);
				Delay_smg(100);
			}
			if(dat_f>99)
			{
				Init_smg(5,smg_NO_duanma[(dat_f%1000)/100]);
				Delay_smg(100);
			}
			if(dat_f>9)
			{
				Init_smg(6,smg_NO_duanma[(dat_f%100)/10]);
				Delay_smg(100);
			}
			Init_smg(7,smg_NO_duanma[dat_f%10]);
			Delay_smg(100);
		break;
			
		case 2:
			Init_smg(0,0xc8);
			Delay_smg(100);
			if(zhouqi>9999)
			{
				Init_smg(3,smg_NO_duanma[zhouqi/10000]);
				Delay_smg(100);
			}
			if(zhouqi>999)
			{
				Init_smg(4,smg_NO_duanma[(zhouqi%10000)/1000]);
				Delay_smg(100);
			}
			if(zhouqi>99)
			{
				Init_smg(5,smg_NO_duanma[(zhouqi%1000)/100]);
				Delay_smg(100);
			}
			if(zhouqi>9)
			{
				Init_smg(6,smg_NO_duanma[(zhouqi%100)/10]);
				Delay_smg(100);
			}
			Init_smg(7,smg_NO_duanma[zhouqi%10]);
			Delay_smg(100);
		break;
			
		case 3:
			if(channel_num==1)
			{
				Init_smg(0,0xc1);
			  Delay_smg(100);
				Init_smg(1,0xbf);
				Delay_smg(100);
				Init_smg(2,smg_NO_duanma[channel_num]);
				Delay_smg(100);
				Init_smg(5,smg_duanma[smg3_volt/100]);
				Delay_smg(100);
				Init_smg(6,smg_NO_duanma[(smg3_volt%100)/10]);
				Delay_smg(100);
				Init_smg(7,smg_NO_duanma[smg3_volt%10]);
				Delay_smg(100);
			}
			if(channel_num==3)
			{
				Init_smg(0,0xc1);
				Delay_smg(100);
				Init_smg(1,0xbf);
				Delay_smg(100);
				Init_smg(2,smg_NO_duanma[channel_num]);
				Delay_smg(100);
				Init_smg(5,smg_duanma[smg1_volt/100]);
				Delay_smg(100);
				Init_smg(6,smg_NO_duanma[(smg1_volt%100)/10]);
				Delay_smg(100);
				Init_smg(7,smg_NO_duanma[smg1_volt%10]);
				Delay_smg(100);
			}
		break;
	}
	Select_HC573(6);
	P0=0xff;
	Select_HC573(7);
	P0=0xff;

//			Init_smg(0,smg_duanma[dat2/100]);
//			Delay_smg(100);
//			Init_smg(1,smg_NO_duanma[(dat2%100)/10]);
//			Delay_smg(100);
//			Init_smg(2,smg_NO_duanma[dat2%10]);
//			Delay_smg(100);
//			
//				Init_smg(5,smg_duanma[dat_f/100]);
//				Delay_smg(100);
//				Init_smg(6,smg_NO_duanma[(dat_f%100)/10]);
//				Delay_smg(100);
//				Init_smg(7,smg_NO_duanma[dat_f%10]);
//				Delay_smg(100);
}
//==================NE555======================
void Read_Ne555_Timer()
{
	TH0=0xff;
	TL0=0xff;
	
	TH1=(65536-50000)/256;
	TL1=(65536-50000)%256;
	
	TMOD=0x16;
	
	ET0=1;
	ET1=1;
	EA=1;
	
	TR0=1;
	TR1=1;
}

void ServerT0() interrupt 1
{
	count_f++;
}

void ServerT1() interrupt 3
{
	TH1=(65536-50000)/256;
	TL1=(65536-50000)%256;
	
	count_t++;
	if(count_t==20)
	{
		dat_f=count_f;
		count_t=0;
		count_f=0;
	}
}
//====================PCF8591AD采集============
unsigned char Read_PCF8591_Tmp(unsigned char ain)
{
	unsigned char tmp;
	
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	
	if(ain==1)
	{
		IIC_SendByte(0x01);
	}
	if(ain==3)
	{
		IIC_SendByte(0x03);
	}
	IIC_WaitAck();
	IIC_Stop();
	
	Display_smg();
	
	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();
	tmp=IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
	
	return tmp;
}

void Read_AIN1_AIN3()
{
	adc1_value=Read_PCF8591_Tmp(1);
	adc1_volt=adc1_value*(5.0/255);
	smg1_volt=adc1_volt*100;
	
	adc3_value=Read_PCF8591_Tmp(3);
	adc3_volt=adc3_value*(5.0/255);
	smg3_volt=adc3_volt*100;
	
	
}
//=================24c02单字节写入=============
void Wrete_24c02(unsigned char addr,unsigned char dat)
{
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();
}

unsigned char Read_24c02(unsigned char addr)
{
	unsigned char tmp;
	
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
  IIC_Stop();
	
	IIC_Start();
	IIC_SendByte(0xa1);
	IIC_WaitAck();
	tmp=IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
	
	return tmp;
}
//====================按键扫描=================
void Delay_Key(unsigned char k)
{
	while(k--);
}
void Scan_Key()
{
	if(S4==0)
	{
		Delay_Key(100);
		if(S4==0)
		{
			if(mode==1)
			{
				mode=2;
				zhouqi=1000000/dat_f;
			}
			else if(mode==2)
			{
				mode=3;
			}
			else if(mode==3)
			{
				mode=1;
			}
			while(S4==0)
			{
				Display_smg();
			}
		}
	}
	
	if(mode==3)
		{
			if(S5==0)
			{
				Delay_Key(100);
				if(channel_num==1)
				{
					channel_num=3;
				}
				else if(channel_num==3)
				{
					channel_num=1;
				}
			}
			while(S5==0)
			{
				Display_smg();
			}
		}
	if(S6==0)
	{
		Delay_Key(100);
		if(S6==0)
		{
			Wrete_24c02(0x01,smg1_volt);
		}
	}
	if(S7==0)
	{
		Delay_Key(100);
		if(S7==0)
		{
			Wrete_24c02(0x02,dat_f);
		}
	}
	
	dat2=Read_24c02(0x02);
	dat1=Read_24c02(0x01);
	if(dat1<smg1_volt)
	{
		Select_HC573(4);
		P0=~(0x01);
	}
	if(dat2<dat_f)
	{
		Select_HC573(4);
		P0=~(0x02);
	}
	
}
//===================系统初始化================
void Init_System()
{
	Select_HC573(5);
	P0=0x00;
	Select_HC573(4);
	P0=0xff;
}
//====================主函数===================
void main()
{
	Read_Ne555_Timer();
	Init_System();
	while(1)
	{
		Display_smg();
		Scan_Key();
		Read_AIN1_AIN3();
	}
}