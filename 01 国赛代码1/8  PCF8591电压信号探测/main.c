#include "reg52.h"
#include "iic.h"

sbit S4=P3^3;
sbit S5=P3^2;
sbit S6=P3^1;

unsigned char code SMG_NODAT[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};

unsigned char adc3_value=0;   //通道3采集数据
float adc3_volt=0;            //转换数据
unsigned int smg_volt=0;      //数码管显示数据
unsigned int parma=300;       //电压参数
unsigned char stat_smg=1;     //当前界面模式
unsigned char num=0;          //报警计数变量
unsigned int count_t=0;       //50ms单位累计变量
unsigned char stat_led=0xff;  //LED灯控制状态
unsigned char F_shan=0;       //秒闪标志
unsigned char F_time=0;       //及时标志
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
//===================数码管显示界面==========//
void Display_SMG()
{
	switch(stat_smg)
	{
		//电压数据=采样电压
		case 1:
			DisplaySMG_bit(7,SMG_NODAT[smg_volt%10]);
			DisplaySMG_bit(6,SMG_NODAT[(smg_volt/10)%10]);
			DisplaySMG_bit(5,SMG_NODAT[smg_volt/100]-0x80);
			DisplaySMG_bit(0,0xc1);
		break;
		//电压参数界面
		case 2:
			DisplaySMG_bit(7,SMG_NODAT[parma%10]);
			DisplaySMG_bit(6,SMG_NODAT[(parma/10)%10]);
			DisplaySMG_bit(5,SMG_NODAT[parma/100]-0x80);
			DisplaySMG_bit(0,0x8c);
		break;
		//报警计数
		case 3:
			DisplaySMG_bit(7,SMG_NODAT[num%10]);
			if(num/10!=0)
			{
				DisplaySMG_bit(6,SMG_NODAT[num/10]);
			}
			DisplaySMG_bit(0,0x92);
		break;
	}
}
//==================PCF8591电压采集==========//
unsigned char Read_PCF8591_AIN3()
{
	unsigned char tmp;
	
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x03);
	I2CWaitAck();
	I2CStop();
	
	Display_SMG();
	
	I2CStart();
	I2CSendByte(0x91);
	I2CWaitAck();
	tmp=I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	
	return tmp;
}
//=================电压数据处理函数==========//
void App_Volt()
{
	unsigned char i;
	adc3_value=0;
	for(i=0;i<3;i++)
	{
		adc3_value+=Read_PCF8591_AIN3();
	}
	adc3_value=adc3_value/3;
	adc3_volt=adc3_value*(5.0/255);
	smg_volt=adc3_volt*100;
	
	if(smg_volt<parma)
	{
		
	}
}
//====================定时器0初始化==========//
void Init_Timer0()
{
	TH0=(0-50000)/256;
	TL0=(0-50000)%256;
	
	
}
//=====================按键扫描==============//
void Key_Scan()
{
	if(S4==0)
	{
		Delay_SMG(500);
		if(S4==0)
		{
			if(stat_smg==1)
			{
				stat_smg=2;
			}
			else if(stat_smg==2)
			{
				stat_smg=3;
			}
			else if(stat_smg==3)
			{
				stat_smg=1;
			}
			while(S4==0)
			{
				App_Volt();
			}
		}
	}
	
	if(S5==0)
	{
		Delay_SMG(500);
		if(S5==0)
		{
			if(stat_smg==2)
			{
				if(parma!=0)
				{
					parma=parma-50;
				}
				else
				{
					parma=500;
				}
			}
			while(S5==0)
			{
				App_Volt();
			}
		}
	}
	
	if(S6==0)
	{
		Delay_SMG(500);
		if(S6==0)
		{
			if(stat_smg==2)
			{
				if(parma!=500)
				{
					parma=parma+50;
				}
				else
				{
					parma=0;
				}
			}
			while(S6==0)
			{
				App_Volt();
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
		App_Volt();
		Key_Scan();
		Display_SMG();
	}
}