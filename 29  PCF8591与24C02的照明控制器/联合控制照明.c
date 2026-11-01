#include "reg52.h"
#include "iic.h"

unsigned char code SMG_NoDot[18]={0xc0,0xf9,
    0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
    0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};

unsigned char code SMG_Dot[10]={0x40,0x79,
    0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};

unsigned char adc_value=0;             //采样数据
float adc_volt=0;                      //换算电压
unsigned int smg_volt=0;               //电压显示
unsigned char stat_led=0xff;           //定义LED灯当前状态
unsigned char level=0;                 //灯光控制等级
sbit s4=P3^3;
sbit s5=P3^2;
//=========================初始化通道锁存器==============
void Selsec_HC573(unsigned char channel)
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
//=========================延时函数======================
void Delay_smg(unsigned char t)
{
	while(t--);
	while(t--);
}
//=========================初始化数码管==================
void Init_smg(unsigned char pos,unsigned char value)
{
	P0=0xff;
	Selsec_HC573(6);
	P0=0x01<<pos;
	Selsec_HC573(7);
	P0=value;
}
//==========================数码管显示===================
void Display_smg()
{
	Init_smg(0,SMG_NoDot[16]);
	Delay_smg(100);
	Init_smg(1,SMG_NoDot[level]);
	Delay_smg(100);
	Init_smg(2,SMG_NoDot[16]);
	Delay_smg(100);
	
	
	Init_smg(5,SMG_Dot[smg_volt/100]);
	Delay_smg(100);
	Init_smg(6,SMG_NoDot[(smg_volt%100)/10]);
	Delay_smg(100);
	Init_smg(7,SMG_NoDot[smg_volt%10]);
	Delay_smg(100);
	
	Selsec_HC573(6);
	P0=0xff;
	Selsec_HC573(7);
	P0=0xff;
}
//==========================24c02单字节写入==============
void Write_24c02(unsigned char addr,unsigned char dat)
{
	IIC_Start();
	IIC_SendByte(0xa0);                   //EEPROM写设备地址
	IIC_WaitAck();
	IIC_SendByte(addr);                   //内存单元地址
	IIC_WaitAck();
	IIC_SendByte(dat);                    //内存写入数据
	IIC_WaitAck();
	IIC_Stop();
}
//==========================24c02单字节读取==============
unsigned char Read_24c02(unsigned char addr)
{
	unsigned char tmp;
	
	IIC_Start();
	IIC_SendByte(0xa0);                   //EEPROM写设备地址
	IIC_WaitAck();
	IIC_SendByte(addr);                   //内存单元地址
	IIC_WaitAck();
	
	IIC_Start();
	IIC_SendByte(0xa1);                   //EEPROM读设备地址
	IIC_WaitAck();
	tmp=IIC_RecByte();                    //读取内存中数据
	IIC_SendAck(1);
	IIC_Stop();
	
	return tmp;
}
//=========================PCF8591电压采样处理函数=======
void Read_PCF8591_AIN1()
{
	IIC_Start();
	IIC_SendByte(0x90);                    //PCF8591的写设备地址
	IIC_WaitAck();
	IIC_SendByte(0x01);                    //通道1，光敏电阻电压
	IIC_WaitAck();
	IIC_Stop();
	
	IIC_Start();
	IIC_SendByte(0x91);                    //PCF8591的读设备地址
	IIC_WaitAck();
	adc_value=IIC_RecByte();               //读出AD采样数据
	IIC_SendAck(1);
	IIC_Stop();
	
	adc_volt=adc_value*(5.0/255);
	smg_volt=adc_volt*100;
	Display_smg();
}
//===============================照明控制======================
void LED_Control()
{
	Read_PCF8591_AIN1();
	if(adc_volt>4.5)
	{
		stat_led=0xff;
		level=0;
	}
	else if(adc_volt>4.0)
	{
		stat_led=0xfe;
		level=1;
	}
	else if(adc_volt>3.5)
	{
		stat_led=0xfc;
		level=2;
	}
	else if(adc_volt>3.0)
	{
		stat_led=0xf8;
		level=3;
	}
	else if(adc_volt>2.5)
	{
		stat_led=0xf0;
		level=4;
	}
	else if(adc_volt>2.0)
	{
		stat_led=0xe0;
		level=5;
	}
	else if(adc_volt>1.5)
	{
		stat_led=0xc0;
		level=6;
	}
	else if(adc_volt>1.0)
	{
		stat_led=0x80;
		level=7;
	}
	else
	{
		stat_led=0x00;
		level=8;
	}
	Selsec_HC573(4);
	P0=stat_led;
}
//==========================保存当前数据=================
void Save_to_24c02()
{
	Write_24c02(0x01,level);
	Display_smg();
	Write_24c02(0x02,adc_value);
	Display_smg();
}
//===========================读取历史数据================
void Read_from_24c02()
{
	level=Read_24c02(0x01);
	adc_value=Read_24c02(0x02);
	adc_volt=adc_value*(5.0/255);
	smg_volt=adc_volt*100;
}
//==========================按键扫描函数=================
void Scan_Key()
{
	if(s4==0);
	{
		Delay_smg(100);
		if(s4==0)
		{
			Save_to_24c02();
			while(s4==0)
			{
				LED_Control();
			}
		}
	}
	
	if(s5==0)
	{
		Delay_smg(100);
		if(s5==0)
		{
			while(s5==0)
			{
				Selsec_HC573(4);
				P0=0xff;
				Read_from_24c02();
				Display_smg();
			}
			
//			while(s5==0)
//			{
//				
//			}
		}
	}
}
//==========================系统初始化===================
void Init_System()
{
	Selsec_HC573(5);
	P0=0x00;
	Selsec_HC573(4);
	P0=0xff;
}
//============================主函数=====================
void main()
{
	Init_System();
	while(1)
	{

		Scan_Key();
		LED_Control();
	}
}