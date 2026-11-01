#include "reg52.h"
#include "iic.h"

//=========================无小数点断码=========================//
unsigned char code SMG_NoDot[18]={0xc0,0xf9,
    0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
    0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};
//==========================带小数断码==========================//
unsigned char code SMG_Dot[10]={0x40,0x79,
    0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};
//============================定义变量=========================//
unsigned char adc1_value=0;        //AIN1的采样数据
float adc1_volt=0;                 //AIN1的换算电压
unsigned int smg1_volt=0;          //AIN的显示电压
		
unsigned char adc3_value=0;        //AIN3的采样数据
float adc3_volt=0;                 //AIN3的换算电压
unsigned int smg3_volt=0;          //AIN3的显示电压
//==============================延时函数=======================//
void Delay_SMG(unsigned char t)
{
	while(t--);
	while(t--);
}
//========================初始化HC573寄存器====================//
void Selcet_HC573(unsigned char channel)
{
	switch(channel)
	{
		case 4:P2=(P2&0x1f)|0x80;break;
		case 5:P2=(P2&0x1f)|0xa0;break;
		case 6:P2=(P2&0x1f)|0xc0;break;
		case 7:P2=(P2&0x1f)|0xe0;break;
		case 0:P2=(P2&0x1f)|0x80;break;
	}
}
//=============================数码管初始化=====================//
void Init_SMG(unsigned char pos,unsigned char value)
{
	P0=0xff;
	Selcet_HC573(6);
	P0=0x01<<pos;
	Selcet_HC573(7);
	P0=value;
}
//==============================数码管显示======================//
void Display_SMG_ADC()
{
	Init_SMG(0,SMG_Dot[smg3_volt/100]);
	Delay_SMG(100);
	Init_SMG(1,SMG_NoDot[(smg3_volt/10)%10]);
	Delay_SMG(100);
	Init_SMG(2,SMG_NoDot[smg3_volt%10]);
	Delay_SMG(100);
	
	Init_SMG(5,SMG_Dot[smg1_volt/100]);
	Delay_SMG(100);
	Init_SMG(6,SMG_NoDot[(smg1_volt/10)%10]);
	Delay_SMG(100);
	Init_SMG(7,SMG_NoDot[smg1_volt%10]);
	Delay_SMG(100);
	
	Selcet_HC573(6);
	P0=0xff;
	Selcet_HC573(7);
	P0=0xff;
}
//======================PCF8591电压采样处理函数=================//
unsigned char Read_PCF8591_ADC(unsigned char ain)
{
	unsigned char tmp;
	
	IIC_Start();                //开始
	IIC_SendByte(0x90);         //PCF8591的写设备地址
	IIC_WaitAck();              //等待应答
	
	if(ain==1)
	{
		IIC_SendByte(0x01);       //通道1，光敏电阻电压
	}
	if(ain==3)
	{
		IIC_SendByte(0x03);       //通道3，可调电压电阻
	}
	IIC_WaitAck();              //等待应答
	IIC_Stop();                 //停止
	
	Display_SMG_ADC();          //等待电压转换完成
	
	IIC_Start();
	IIC_SendByte(0x91);         //PCF8591读设备地址
	IIC_WaitAck();
	tmp=IIC_RecByte();          //读出AD采样数据
	IIC_SendAck(1);             //产生非应答信号
	IIC_Stop();
	
	return tmp;
}
//=====================采样光敏电阻和可调电压的电阻=============//
void Read_AIN1_AIN3()
{
	adc1_value=Read_PCF8591_ADC(1);
	adc1_volt=adc1_value*(5.0/255);
	smg1_volt=adc1_volt*100;
	
	adc3_value=Read_PCF8591_ADC(3);
	adc3_volt=adc3_value*(5.0/255);
	smg3_volt=adc3_volt*100;
}

//============================系统初始化========================//
void Init_System()
{
	Selcet_HC573(5);
	P0=0x00;
	Selcet_HC573(4);
	P0=0xff;
}
//=============================主函数===========================//
void main()
{
	Init_System();
	while(1)
	{
		Display_SMG_ADC();
		Read_AIN1_AIN3();
	}
}