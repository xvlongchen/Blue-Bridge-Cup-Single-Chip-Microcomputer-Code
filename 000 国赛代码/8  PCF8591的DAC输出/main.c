#include "reg52.h"
#include "iic.h"

sbit S4=P3^3;

unsigned char adc_value=0;      //AIN3的采样数据
float adc_volt=0;               //AIN3的换算电压
unsigned int smg_volt;        //AIN3显示电压

unsigned char mode=1;           //模式

//无小数点断码
unsigned char code SMG_NODOT[18]={0xc0,0xf9,
    0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
    0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};
//===================寄存器通道选择=============//
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
//==================单个数码管显示============//
//数码管延时函数
void Delay_SMG(unsigned int t)
{
	while(t--);
}
void DisplaySMG_Bit(unsigned char pos,unsigned char value)
{
	Select_HC573(6,0x01<<pos);
	Select_HC573(7,value);
	Delay_SMG(500);
	Select_HC573(6,0x01<<pos);
	Select_HC573(7,0xff);
}
//==================关闭数码管================//
void OffSMG_All(unsigned char value)
{
	Select_HC573(6,0xff);
	Select_HC573(7,value);
}
//==================显示数码管函数============//
void Display_SMG()
{
	DisplaySMG_Bit(0,SMG_NODOT[16]);
	DisplaySMG_Bit(1,SMG_NODOT[mode]);
	DisplaySMG_Bit(2,SMG_NODOT[16]);
	
	DisplaySMG_Bit(5,SMG_NODOT[smg_volt/100]-0x80);
	DisplaySMG_Bit(6,SMG_NODOT[(smg_volt%100)/10]);
	DisplaySMG_Bit(7,SMG_NODOT[smg_volt%10]);
}
//================PCF8591电压采集函数=========//
void Read_PCF8591_ADC_AIN3()
{
	I2CStart();
	I2CSendByte(0x90);     //PCF8591写设备地址
	I2CWaitAck();
	I2CSendByte(0x03);     
	I2CWaitAck();
	I2CStop();
	
	I2CStart();
	I2CSendByte(0x91);      //读设备地址
	I2CWaitAck();
	adc_value=I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	
	adc_volt=adc_value*(5.0/255);
	smg_volt=adc_volt*100;
}
//=================PCF8591电压输出设备函数====//
void Set_PCF8591_DAC(unsigned char dat)
{
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x43);    //设置DAC，转换AIN3
	I2CWaitAck();
	I2CSendByte(dat);
	I2CWaitAck();
	I2CStop();
}
//=====================按键扫描==============//
//按键扫描函数
void Delay_Key(unsigned char t)
{
	while(t--);
}

void Scan_Keys()
{
	if(S4==0)
	{
		Delay_Key(100);
		if(S4==0)
		{
			if(mode==1)
			{
				mode=2;
				Set_PCF8591_DAC(204);    //输出固定电压
				smg_volt=400;
			}
			else if(mode==2)
			{
				mode=3;
			}
			else if(mode==3)
			{
				mode=1;
				Set_PCF8591_DAC(102);
				smg_volt=200;
			}
			while(S4==0)
			{
				Display_SMG();
			}
		}
	}
}
//=================系统初始化=================//
void Init_System()
{
	Select_HC573(0,0x00);
	Select_HC573(5,0x00);
	Select_HC573(4,0xff);
	OffSMG_All(0xff);
}
//================主函数=====================//
void main()
{
	Init_System();
	while(1)
	{
		if(mode==3)
		{
			Read_PCF8591_ADC_AIN3();
			Set_PCF8591_DAC(adc_value);
		}
		Scan_Keys();
		Display_SMG();
	}
}