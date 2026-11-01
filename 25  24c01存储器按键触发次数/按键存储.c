#include "reg52.h"
#include "iic.h"

//=============定义按键==========//
sbit S4=P3^3;
sbit S5=P3^2;
sbit S6=P3^1;
//=============数码管断码======//
unsigned char code SMG_duanma[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};
//=============定义变量========//
unsigned char dat1=0;
unsigned char dat2=0;
unsigned char dat3=0;
//====================延时函数===========//
void Delay_SMG(unsigned char t)
{
	while(t--);
	while(t--);
}

void Delay_Key(unsigned char k)
{
	while(k--);
}
//==============HC573寄存器============//
void Select_HC573(unsigned char channel)
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
//==================数码管初始化=================//
void Init_SMG(unsigned char pos,unsigned char value)
{
	P0=0xff;
	Select_HC573(6);
	P0=0x01<<pos;
	Select_HC573(7);
	P0=value;
}
//====================数码管显示==================//
void Display_SMG()
{
	Init_SMG(0,SMG_duanma[dat1/10]);
	Delay_SMG(100);
	Init_SMG(1,SMG_duanma[dat1%10]);
	Delay_SMG(100);
	
	Init_SMG(2,SMG_duanma[16]);
	Delay_SMG(100);
	
	Init_SMG(3,SMG_duanma[dat2/10]);
	Delay_SMG(100);
	Init_SMG(4,SMG_duanma[dat2%10]);
	Delay_SMG(100);
	
	Init_SMG(5,SMG_duanma[16]);
	Delay_SMG(100);
	
	Init_SMG(6,SMG_duanma[dat3/10]);
	Delay_SMG(100);
	Init_SMG(7,SMG_duanma[dat3%10]);
	Delay_SMG(100);
	
	Select_HC573(6);
	P0=0xff;
	Select_HC573(7);
	P0=0xff;
}
//====================24c02单字节写入=============//
void Weite_24c02(unsigned char addr,unsigned char dat)
{
	IIC_Start();              //开始
	IIC_SendByte(0xa0);           //写设备地址
	IIC_WaitAck();            //等待应答
	IIC_SendByte(addr);       //内存地址单元
	IIC_WaitAck();            //等待应答
	IIC_SendByte(dat);        //内存写入数据
	IIC_WaitAck();            //等待应答
	IIC_Stop();               //停止
}
//====================24c01单字节读取============//
unsigned char Read_24c02(unsigned char addr)
{
	unsigned char tmp;
	
	IIC_Start();              //开始
	IIC_SendByte(0xa0);           //写设备地址
	IIC_WaitAck();            //等待应答
	IIC_SendByte(addr);       //内存地址单元
	IIC_WaitAck();            //等待应答
	
	IIC_Start();              //开始
	IIC_SendByte(0xa1);           //读设备地址
	IIC_WaitAck();            //等待应答
	tmp=IIC_RecByte();        //读取设备地址
	IIC_SendAck(1);           //产生非应答信号
	IIC_Stop();               //停止
	
	return tmp;
}
//===================按键扫描=================//
void Scan_Key()
{
	if(S4==0)
	{
		Delay_Key(100);
		if(S4==0)
		{
			dat1++;
			if(dat1>13)
			{
				dat1=0;
			}
			Weite_24c02(0x00,dat1);
			while(S4==0)
			{
				Display_SMG();
			}
		}
	}
	
	if(S5==0)
	{
		Delay_Key(100);
		if(S5==0)
		{
			dat2++;
			if(dat2>13)
			{
				dat2=0;
			}
			Weite_24c02(0x01,dat2);
			while(S5==0)
			{
				Display_SMG();
			}
		}
	}
	
	if(S6==0)
	{
		Delay_Key(100);
		if(S6==0)
		{
			dat3++;
			if(dat3>13)
			{
				dat3=0;
			}
			Weite_24c02(0x02,dat3);
			while(S6==0)
			{
				Display_SMG();
			}
		}
	}
	dat1=Read_24c02(0x00);
	dat2=Read_24c02(0x01);
	dat3=Read_24c02(0x02);
}

//==================系统初始化================//
void Init_System()
{
	Select_HC573(5);
	P0=0x00;
	Select_HC573(4);
	P0=0xff;
	
//	dat1=Read_24c02(0x00);
//	dat2=Read_24c02(0x01);
//	dat3=Read_24c02(0x02);
}
void main()
{
	Init_System();
	while(1)
	{
		Scan_Key();
		Display_SMG();
	}
}