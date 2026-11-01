#include "reg52.h"
#include "iic.h"

sbit P4=P3^3;
sbit P5=P3^2;
sbit P6=P3^1;

unsigned char code SMG_NoDot[18]={0xc0,0xf9,
    0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
    0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};

unsigned char adc_value=0;        //AIN3的采样数据
float adc_volt=0;                 //AIN3的换算电压
unsigned int smg_volt=0;          //AIN3的显示电压
//======================寄存器通道选择===============//
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
//======================关闭所有数码管===============//
void OffSMG_All(unsigned char value)
{
	Select_HC573(6,0xff);
	Select_HC573(7,value);
}
//======================单个数码管显示==============//
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
//======================PCF8591电压采样处理函数=====///
unsigned char Read_PCF8591_AIN3()
{
	unsigned char ferq;
	
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x03);
	I2CWaitAck();
	I2CStop();
	
	I2CStart();
	I2CSendByte(0x91);
	I2CWaitAck();
	ferq=I2CReceiveByte();
	I2CWaitAck();
	I2CSendAck(1);
	I2CStop();
	
	return ferq;
}
//======================电压处理函数================//
void App_Volt()
{
	unsigned char i;
	for(i=0;i<3;i++)
	{
		adc_value+=Read_PCF8591_AIN3();
	}
	adc_value=adc_value/3;
	adc_volt=adc_value*(5.0/255);
	smg_volt=adc_volt*100;
}
//=======================系统初始化=================//
void Init_System()
{
	Select_HC573(0,0x00);
	Select_HC573(5,0x00);
	Select_HC573(4,0xff);
	OffSMG_All(0xff);
}
//=======================主函数===================//
void main()
{
	Init_System();
	while(1)
	{
		App_Volt();
	}
}