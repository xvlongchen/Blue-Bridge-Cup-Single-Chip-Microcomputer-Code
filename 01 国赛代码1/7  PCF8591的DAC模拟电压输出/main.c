#include "reg52.h"
#include "iic.h"

unsigned char code SMG_NODAT[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};

sbit S4=P3^3;
unsigned char adc_value=0;   //AIN3采样数据
float adc_volt=0;            //AIN3换算电压
unsigned int smg_volt=0;     //数码管显示电压；
unsigned char mode=1;   //DAC输出模式


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
//=============数码管显示几种模式的电压======//
void DisplaySMG_DAC()
{
	DisplaySMG_bit(7,SMG_NODAT[smg_volt%10]);
	DisplaySMG_bit(6,SMG_NODAT[(smg_volt/10)%10]);
	DisplaySMG_bit(5,SMG_NODAT[smg_volt/100]-0x80);
	
	DisplaySMG_bit(2,SMG_NODAT[16]);
	DisplaySMG_bit(1,SMG_NODAT[mode]);
	DisplaySMG_bit(0,SMG_NODAT[16]);
}
//===================PCF8591采样处理函数=====//
void Read_PCF8591_AIN3()
{
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x43);    //DAC转换
	I2CWaitAck();
	I2CStop();
	
	DisplaySMG_DAC();
	
	I2CStart();
	I2CSendByte(0x91);
	I2CWaitAck();
	adc_value=I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	
	adc_volt=adc_value*(5.0/255);
	smg_volt=adc_volt*100;
}
//===============PCF8591电压输出函数=========//
void Set_PCF8591_DAC(unsigned char dat)
{
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x43);
	I2CWaitAck();
	I2CSendByte(dat);
	I2CWaitAck();
	I2CStop();
}
//====================按键扫描===============//
void Key_scan()
{
	if(S4==0)
	{
		Delay_SMG(500);
		if(S4==0)
		{
			if(mode==1)
			{
				mode=2;
				smg_volt=400;
				Set_PCF8591_DAC(204);
			}
			else if(mode==2)
			{
				mode=3;
				Read_PCF8591_AIN3();
				Set_PCF8591_DAC(adc_value);
			}
			else if(mode==3)
			{
				mode=1;
				smg_volt=200;
				Set_PCF8591_DAC(102);
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
	smg_volt=200;
}
//==================主函数===================//
void main()
{
	Init_System();
	while(1)
	{
		void DisplaySMG_DAC();
		Key_scan();
	}
}