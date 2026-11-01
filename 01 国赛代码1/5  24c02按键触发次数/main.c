#include "reg52.h"
#include "iic.h"

unsigned char code SMG_NODAT[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};
//按键
sbit S4=P3^3;
sbit S5=P3^2;
sbit S6=P3^1;

unsigned char dat1=0;
unsigned char dat2=0;
unsigned char dat3=0;

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
//================数码管显示按键次数=========//
void Display_SMG()
{
	DisplaySMG_bit(7,SMG_NODAT[dat3%10]);
	DisplaySMG_bit(6,SMG_NODAT[dat3/10]);
	DisplaySMG_bit(5,SMG_NODAT[16]);
	
	DisplaySMG_bit(4,SMG_NODAT[dat2%10]);
	DisplaySMG_bit(3,SMG_NODAT[dat2/10]);
	DisplaySMG_bit(2,SMG_NODAT[16]);
	
	DisplaySMG_bit(1,SMG_NODAT[dat1%10]);
	DisplaySMG_bit(0,SMG_NODAT[dat1/10]);
}
//===================24c02单数据写入=========//
void Write_24c02(unsigned char addr,unsigned char dat)
{
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(addr);
	I2CWaitAck();
	I2CSendByte(dat);
	I2CWaitAck();
	I2CStop();
}
//====================24c02数据读取==========//
unsigned char Read_24c02(unsigned char addr)
{
	unsigned char tmp;
	//伪写
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(addr);
	I2CWaitAck();
	
	//数据读取
	I2CStart();
	I2CSendByte(0xa1);
	I2CWaitAck();
	tmp=I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	
	return tmp;
}
//====================24c02数据读取与写入====//
void Read_Write_24c02()
{
	dat1=Read_24c02(0x00);
	dat2=Read_24c02(0x01);
	dat3=Read_24c02(0x02);
	
	//按键4
	if(S4==0)
	{
		Delay_SMG(500);
		if(S4==0)
		{
			dat1++;
			if(dat1>13)
			{
				dat1=0;
			}
			Write_24c02(0x00,dat1);
			while(S4==0)
			{
				Display_SMG();
			}
		}
	}
	//按键5
	if(S5==0)
	{
		Delay_SMG(500);
		if(S5==0)
		{
			dat2++;
			if(dat2>13)
			{
				dat2=0;
			}
			Write_24c02(0x01,dat2);
			while(S5==0)
			{
				Display_SMG();
			}
		}
	}
	//按键6
	if(S6==0)
	{
		Delay_SMG(500);
		if(S6==0)
		{
			dat3++;
			if(dat3>13)
			{
				dat3=0;
			}
			Write_24c02(0x02,dat3);
			while(S6==0)
			{
				Display_SMG();
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
	Read_Write_24c02();
	while(1)
	{
		Display_SMG();
	}
}