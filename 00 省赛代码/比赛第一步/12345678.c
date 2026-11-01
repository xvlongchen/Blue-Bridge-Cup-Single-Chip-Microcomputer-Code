#include "reg52.h"

//=============延时函数===========
void Delay_smg(unsigned int t)
{
	while(t--);
}
//=============寄存器通道选择==========
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
//=======================数码管========================
void Display_smg(unsigned char pos,unsigned char value)
{
	Select_HC573(0,0x01<<pos);
	Select_HC573(7,value);
	Delay_smg(500);
	Select_HC573(6,0x01<<pos);
	Select_HC573(7,0xff);
}
//=====================关闭数码管======================
void Display_smg_all(unsigned char value)
{
	Select_HC573(6,0xff);
	Select_HC573(7,value);
}
//=======================初始化系统====================
void Init_System()
{
	Select_HC573(0,0x00);
	Select_HC573(5,0x00);
	Select_HC573(4,0xff);
	Display_smg_all(0xff);
}
//============================主函数================
void main()
{
	Init_System();
	while(1)
	{
		
	}
}