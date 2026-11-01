#include "reg52.h"

unsigned char code SMG_NoDot[18]={0xc0,0xf9,
    0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
    0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};

unsigned char stat_led=0xff;
unsigned char num1=0,num2=0;

//==================寄存器通道选择============
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
//==================关闭全部数码管============
void DisplaySMg_All(unsigned char value)
{
	Select_HC573(6,0xff);
	Select_HC573(7,value);
}
//==================数码管====================
void Delay_smg(unsigned char t)
{
	while(t--);
}

//初始化数码管
void Init_SMG(unsigned char pos,unsigned char value)
{
	Select_HC573(7,0xff);
	Select_HC573(6,0x01<<pos);
	Select_HC573(7,value);
}

//数码管显示
void Display_smg()
{
	Init_SMG(0,SMG_NoDot[num2]);
	Delay_smg(100);
	
	Init_SMG(6,SMG_NoDot[num1/10]);
	Delay_smg(100);
	Init_SMG(7,SMG_NoDot[num1%10]);
	Delay_smg(100);
}
//===================灯光控制=================
void Delay(unsigned int t)
{
	while(t--)
	{
		Display_smg();
	}
}
void LED_Control()
{
  stat_led &= ~0x80;         
  Select_HC573(4, stat_led);    //L8灯点亮
  Delay(1000);                //延时
  stat_led |= 0x80;         
  Select_HC573(4, stat_led);    //L8灯熄灭
  Delay(1000);                //延时
  
  num1++;                    //L8闪烁累加
  if(num1 == 100)            //达到最大值时恢复0
  {
    num1 = 0;
  }
  //L1灯和L2灯同时翻转当前的工作状态
  if((stat_led & 0x03) == 0x03)
  {
    stat_led &= ~0x03;
  }
  else
  {
    stat_led |= 0x03;
  }
  Select_HC573(4, stat_led);
  
  num2++;                    //L1和L2翻转次数累加
  if(num2 == 10)             //达到最大值时恢复0
  {
    num2 = 0;
  }
}
//==================初始化系统================
void Init_System()
{
	Select_HC573(5,0x00);
	Select_HC573(4,0xff);
	DisplaySMg_All(0xff);
}
//===================主函数===================
void main()
{
	Init_System();
	while(1)
	{
		LED_Control();
		Display_smg();
	}
}