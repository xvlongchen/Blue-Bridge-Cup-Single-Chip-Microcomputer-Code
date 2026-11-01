#include "reg52.h"
#include "iic.h"

unsigned char code SMG_NODat[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};

unsigned char adc1_value=0;        //AIN1采集电压
float adc1_volt=0;                 //AIN1换算电压
unsigned int smg1_volt=0;          //AIN1显示电压

unsigned char adc3_value=0;        
float adc3_volt=0;
unsigned int smg3_volt=0;
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
//============数码管显示光敏电阻和可调电压===//
void Display_SMG_ADC()
{
	DisplaySMG_bit(7,SMG_NODat[smg3_volt%10]);
	DisplaySMG_bit(6,SMG_NODat[(smg3_volt/10)%10]);
	DisplaySMG_bit(5,SMG_NODat[smg3_volt/100]-0x80);
	
	DisplaySMG_bit(2,SMG_NODat[smg1_volt%10]);
	DisplaySMG_bit(1,SMG_NODat[(smg1_volt/10)%10]);
	DisplaySMG_bit(0,SMG_NODat[smg1_volt/100]-0x80);
}
//==================PCF8591电压采样处理函数======//
unsigned char Read_PCF8591_ADC(unsigned char ain)
{
	unsigned char tmp;
	
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	if(ain==1)
	{
		I2CSendByte(0x01);
	}
	else if(ain==3)
	{
		I2CSendByte(0x03);
	}
	I2CWaitAck();
	I2CStop();
	
	Display_SMG_ADC();
	
	I2CStart();
	I2CSendByte(0x91);
	I2CWaitAck();
	tmp=I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	
	return tmp;
}
//==========采样光敏电阻和可调电阻的电压=====//
void Read_AIN1_AIN3()
{
	adc1_value=Read_PCF8591_ADC(1);
	adc1_volt=adc1_value*(5.0/255);
	smg1_volt=adc1_volt*100;
	
	adc3_value=Read_PCF8591_ADC(3);
	adc3_volt=adc3_value*(5.0/255);
	smg3_volt=adc3_volt*100;
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
		Display_SMG_ADC();
		Read_AIN1_AIN3();
	}
}