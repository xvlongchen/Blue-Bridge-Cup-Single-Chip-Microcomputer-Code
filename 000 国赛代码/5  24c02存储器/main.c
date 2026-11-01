#include "reg52.h"
#include "iic.h"

//无小数点断码
unsigned char code SMG_NODOT[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};

unsigned char dat1=0,dat2=0,dat3=0;
//=======================寄存器选择==============//
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
//======================数码管单个显示============//
//延时函数
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
//=====================数码管全部关闭=============//
void OffSMG_All(unsigned char value)
{
	Select_HC573(6,0xff);
	Select_HC573(7,value);
}
//=====================数码管显示24c02的数据=====//
void DisplaySMG_24c02()
{
	DisplaySMG_Bit(0,SMG_NODOT[dat1/10]);
	DisplaySMG_Bit(1,SMG_NODOT[dat1%10]);
	DisplaySMG_Bit(2,SMG_NODOT[16]);
	
	DisplaySMG_Bit(3,SMG_NODOT[dat2/10]);
	DisplaySMG_Bit(4,SMG_NODOT[dat2%10]);
	DisplaySMG_Bit(5,SMG_NODOT[16]);
	
	DisplaySMG_Bit(6,SMG_NODOT[dat3/10]);
	DisplaySMG_Bit(7,SMG_NODOT[dat3%10]);
}
//=====================24c02单字节写入===========//
void Write_24c02(unsigned char addr,unsigned char dat)
{
	I2CStart();
	I2CSendByte(0xa0);     //iic写设备地址
	I2CWaitAck();
	I2CSendByte(addr);     //内存单元地址
	I2CWaitAck();
	I2CSendByte(dat);       //内存写入数据
	I2CWaitAck();
	I2CStop();
}
//=====================24c02单字节读取===========//
unsigned char Read_24c02(unsigned char addr)
{
	unsigned char tmp;
	//先进行一个伪写操作
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(addr);
	I2CWaitAck();
	
	//在进行读操作
	I2CStart();
	I2CSendByte(0xa1);
	I2CWaitAck();
	tmp=I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	
	return tmp;
}
//====================24c02数据的读取与写入======//
void Read_Write_24c02()
{
	dat1=Read_24c02(0x01);
	dat2=Read_24c02(0x03);
	dat3=Read_24c02(0x05);
	
	dat1=dat1+1;
	dat2=dat2+1;
	dat3=dat3+1;
	
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
//=====================系统初始化================//
void Init_System()
{
	Select_HC573(0,0x00);
	Select_HC573(5,0x00);
	Select_HC573(4,0xff);
	OffSMG_All(0xff);
}
//====================主函数=====================//
void main()
{
	Init_System();
	Read_Write_24c02();
	while(1)
	{
		DisplaySMG_24c02();
	}
}