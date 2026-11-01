#include "reg52.h"
#include "iic.h"

unsigned char code SMG_duanma[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};
unsigned char dat1=0,dat2=0,dat3=0;

void HC537(unsigned char channel)
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
//=================延时函数==============================//
void Delay_SMG(unsigned int t)
{
	while(t--);
}
//=================数码管=================================//
void SMG_Init(unsigned char pos,unsigned char value)
{
	P0=0xff;
	HC537(6);
	P0=0x01<<pos;
	HC537(7);
	P0=value;
}

void SMG_Display()
{
	SMG_Init(0,SMG_duanma[dat1/10]);
	Delay_SMG(100);
	SMG_Init(1,SMG_duanma[dat1%10]);
	Delay_SMG(100);
	
	SMG_Init(2,SMG_duanma[16]);
	Delay_SMG(100);
	
	SMG_Init(3,SMG_duanma[dat2/10]);
	Delay_SMG(100);
	SMG_Init(4,SMG_duanma[dat2%10]);
	Delay_SMG(100);
	
	SMG_Init(5,SMG_duanma[16]);
	Delay_SMG(100);
	
	SMG_Init(6,SMG_duanma[dat3/10]);
	Delay_SMG(100);
	SMG_Init(7,SMG_duanma[dat3%10]);
	Delay_SMG(100);
}
//=====================24c02单字节写入=======================//
void Write_24c02(unsigned char addr,unsigned char dat)
{
	IIC_Start();            //开始
	IIC_SendByte(0xa0);         //iic写设备地址
	IIC_WaitAck();          //等待应答
	IIC_SendByte(addr);     //内存单元地址
	IIC_WaitAck();          //等待应答
	IIC_SendByte(dat);     //内存写入数据
	IIC_WaitAck();          //等待应答
	IIC_Stop();             //停止信号
}
//=======================24c02单字节读取====================//
unsigned char Read_24c02(unsigned char addr)
{
	unsigned char tmp;
	//先进行一个伪写操作
	IIC_Start();             //开始
	IIC_SendByte(0xa0);         //iic写设备地址
	IIC_WaitAck();          //等待应答
	IIC_SendByte(addr);     //内存单元地址
	IIC_WaitAck();          //等待应答
	
	//然后，进行读操作
	IIC_Start();             //开始
	IIC_SendByte(0xa1);         //iic读设备地址
	IIC_WaitAck();          //等待应答
	tmp=IIC_RecByte();     //读取内存中的数据
	IIC_SendAck(1);         //产生非应答信号
	IIC_Stop();             //停止信号
	return tmp;
}
//======================24c02数据的读取与写入==========//
void Read_Write_24c02()
{
	dat1=Read_24c02(0x01);
	dat2=Read_24c02(0x03);
	dat3=Read_24c02(0x05);
	
	dat1=dat1+1;
	dat2=dat2+2;
	dat3=dat3+3;
	
	
	if(dat1>10)
	{
		dat1=0;
	}
	if(dat2>20)
	{
		dat2=0;
	}
	if(dat3>30)
	{
		dat3=0;
	}
	
	Write_24c02(0x01,dat1);
	Delay_SMG(10000);
	Write_24c02(0x03,dat2);
	Delay_SMG(10000);
	Write_24c02(0x05,dat3);
	Delay_SMG(10000);
}

//======================系统初始化=====================//
void Init_System()
{
	HC537(5);
	P0=0x00;
	HC537(4);
	P0=0xff;
}
//========================主函数==========================//
void main()
{
	Read_Write_24c02();
	Init_System();
	while(1)
	{
		SMG_Display();
	}
}