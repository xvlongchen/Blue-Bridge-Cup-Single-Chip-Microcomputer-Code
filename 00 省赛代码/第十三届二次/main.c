#include "reg52.h"
#include "iic.h"
#include "intrins.h"

unsigned char code smg_NODOT[18]={0xc0,0xf9,
    0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
    0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};
unsigned char code smg_DOT[10]={0x40,0x79,
    0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};

unsigned int adc3_value=0;
float adc3_volt=0;
unsigned int smg3_volt=0;

unsigned char mode=1;
		
unsigned char ceju_mode=1;  //1为启动测距界面，2为未启动测距界面
	
sfr AUXR=0x8e;		
sbit TX=P1^0;
sbit RX=P1^1;

unsigned int distance=0;
		
unsigned int shang_canshu=45;
unsigned int xia_canshu1=15;
unsigned char xuanze_canshu=1;       //1为上参数，2为下参数		

sbit S4=P3^3;
sbit S5=P3^2;
sbit S6=P3^1;
sbit S7=P3^0;
void Read_chaoshengb();

//===================延时函数用于产生方波======
void Delay12us()		//@12.000MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 33;
	while (--i);
}

//==================寄存器通道选择========
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
//================数码管初始化============
void Init_smg(unsigned char pos,unsigned char value)
{
	P0=0xff;
	Select_HC573(6);
	P0=0x01<<pos;
	Select_HC573(7);
	P0=value;
}
//================数码管显示==============
void Delay_smg(unsigned int t)
{
	while(t--);
}

void Display_smg()
{
	switch(mode)
	{
		case 1:
			Init_smg(0,0xc1);
			Delay_smg(100);
		
			Init_smg(5,smg_DOT[smg3_volt/100]);
			Delay_smg(100);
			Init_smg(6,smg_NODOT[(smg3_volt%100)/10]);
			Delay_smg(100);
			Init_smg(7,smg_NODOT[smg3_volt%10]);
			Delay_smg(100);
		break;
		
		case 2:
			if(ceju_mode==1)
			{
				Init_smg(0,0xc3);
			  Delay_smg(500);
				
				if(distance>99)
				{
					Init_smg(5,smg_NODOT[distance/100]);
					Delay_smg(500);
				}
				if(distance>9)
				{
					Init_smg(6,smg_NODOT[(distance%100)/10]);
					Delay_smg(500);
				}
				Init_smg(7,smg_NODOT[distance%10]);
				Delay_smg(500);
			}
			if(ceju_mode==2)
			{
				Init_smg(5,0x88);
				Delay_smg(100);
				Init_smg(6,0x88);
				Delay_smg(100);
				Init_smg(7,0x88);
				Delay_smg(100);
			}
		break;
			
		case 3:
			Init_smg(0,0x8c);
			Delay_smg(100);
			Init_smg(3,smg_DOT[shang_canshu/10]);
			Delay_smg(100);
			Init_smg(4,smg_NODOT[shang_canshu%10]);
			Delay_smg(100);
		
			Init_smg(6,smg_DOT[xia_canshu1/10]);
			Delay_smg(100);
			Init_smg(7,smg_NODOT[xia_canshu1%10]);
			Delay_smg(100);
			break;
	}
		Select_HC573(6);
		P0=0xff;
		Select_HC573(7);
		P0=0xff;
}
//================PCF8591采集函数=========
unsigned char Read_PCF8591_ADC()
{
	unsigned char tmp;
	
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(0x03);
	IIC_WaitAck();
	IIC_Stop();
	
	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();
	tmp=IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
	
	return tmp;
}
//================采集电压数据============
void Read_AIN3()
{
	adc3_value=Read_PCF8591_ADC();
	adc3_volt=adc3_value*(5.0/255);
	smg3_volt=adc3_volt*100;
}
//=================超声波测距=============
//发送方波信号
void Send_maichong()
{
	char i;
	for(i=0;i<=7;i++)
	{
		TX=1;
		Delay12us();
		TX=0;
		Delay12us();
	}
}

//定时器计数功能
void Timer1Init()		//100微秒@12.000MHz
{
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0x00;		//设置定时初值
	TH1 = 0x00;		//设置定时初值
	TF1 = 0;		//清除TF1标志,计算溢出标志位
	TR1 = 0;		//定时器1开始计时
	
	EA=1;
	ET1=0;      //不跳到定时器服务函数中执行，所以为0关闭
}

//计算时间
void Read_chaoshengb()
{
	unsigned int num=0;
	
	Timer1Init();
	Send_maichong();
	TR1=1;
	
	while((RX==1)&&(TF1==0));
	
	TR1=0;
	if(TF1==0)
	{
		num=TH1;
		num=(num<<8)|TL1;
		distance=((num/10)*17)/100+3;
	}
	else if(TF1==1)
	{
		distance=999;
		TF1=0;
	}
}
//=================DA输出=================
void Read_PCF8591_AIN3(unsigned char dat)
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	
	IIC_SendByte(0x43);
	IIC_WaitAck();
	IIC_SendByte(dat);
	IIC_WaitAck();
	
	IIC_Stop();
}
//=================按键扫描===============
void Delay_Key(unsigned char t)
{
	while(t--);
}

unsigned int csdc;
void Scan_Key()
{
	if(S4==0)
	{
		Delay_Key(10);
		if(S4==0)
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
			while(S4==0)
			{
				Display_smg();
			}
		}
	}
	
	if(S5==0)
	{
		Delay_Key(10);
		if(S5==0)
		{
			if(xuanze_canshu==1)
			{
				xuanze_canshu=2;
			}
			else if(xuanze_canshu==2)
			{
				xuanze_canshu=1;
			}
			while(S5==0)
			{
				Display_smg();
			}
		}
	}
	if(mode==3)
	{
		if(xuanze_canshu==1)
		{
			if(S6==0)
			{
				Delay_Key(10);
				if(S6==0)
				{
					shang_canshu=shang_canshu+5;
					if(shang_canshu>50)
					{
						shang_canshu=5;
					}
				}
				while(S6==0)
				{
					Display_smg();
				}
			}
		}
		
		if(xuanze_canshu==2)
		{
			if(S7==0)
			{
				Delay_Key(10);
				if(S7==0)
				{
					xia_canshu1=xia_canshu1-5;
					if(xia_canshu1<5)
					{
						xia_canshu1=50;
					}
				}
				while(S7==0)
				{
					Display_smg();
				}
			}
		}
	}
}
//=================系统初始化=============
void Init_System()
{
	Select_HC573(5);
	P0=0x00;
	Select_HC573(4);
	P0=0xff;
}
//===================主函数===============
void Delay_chaoshengb(unsigned char g)
{
	while(g--)
	{
		Display_smg();
	}
}

void main()
{
	Init_System();
	while(1)
	{
		Read_AIN3();
		Display_smg();
		Read_chaoshengb();
		Delay_chaoshengb(10);
		Scan_Key();
		if(((shang_canshu/10)>adc3_volt)&&((xia_canshu1/10)<adc3_volt))
		{
			ceju_mode=1;
		}
		else
		{
			ceju_mode=2;
		}
		
		if(mode==2&&ceju_mode==1)
		{
			if(distance>80)
			{
				Read_PCF8591_AIN3(255);
			}
			if(distance<20)
			{
				Read_PCF8591_AIN3(51);
			}
			if(distance>=20&&distance<=40)
			{
				Read_PCF8591_AIN3(6.7*distance-36);
			}
		}
	}
}