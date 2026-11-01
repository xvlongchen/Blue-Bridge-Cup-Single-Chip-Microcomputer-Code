#include "reg52.h"

sbit s4=P3^3;
sbit s5=P3^2;

unsigned char t_f=0;
unsigned char t_s=0;
unsigned char t_005s=0;

unsigned char code SMG_duanma[18]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xbf,0x7f};

void SelectHC573(unsigned char t)
{
	switch(t)
	{
		case 4:P2=(P2&0x1f)|0x80;break;
		case 5:P2=(P2&0x1f)|0xa0;break;
		case 6:P2=(P2&0x1f)|0xc0;break;
		case 7:P2=(P2&0x1f)|0xe0;break;
	}
}

void DisplaySMG_Bit(unsigned char value,unsigned char pos)
{
	SelectHC573(6);
	P0=0x01<<pos;
	SelectHC573(7);
	P0=value;
}

void Delay_SMG(unsigned int j)
{
	while(j--);
}

void DisplayTime()
{
	DisplaySMG_Bit(SMG_duanma[t_005s%10],7);
	Delay_SMG(500);
	DisplaySMG_Bit(SMG_duanma[t_005s/10],6);
	Delay_SMG(500);
	DisplaySMG_Bit(SMG_duanma[16],5);
	Delay_SMG(500);
	
	DisplaySMG_Bit(SMG_duanma[t_s%10],4);
	Delay_SMG(500);
	DisplaySMG_Bit(SMG_duanma[t_s/10],3);
	Delay_SMG(500);
	DisplaySMG_Bit(SMG_duanma[16],2);
	Delay_SMG(500);
	
	DisplaySMG_Bit(SMG_duanma[t_f%10],1);
	Delay_SMG(500);
	DisplaySMG_Bit(SMG_duanma[t_f/10],0);
	Delay_SMG(500);
}

//*************定时器*******//

void InitTimer0()
{
	TMOD=0x01;
	TH0=(65535-50000)/256;
	TL0=(65535-50000)%256;
	
	EA=1;
	ET0=1;
	TR0=1;
}

void ServiceTimer0() interrupt 1
{
	TH0=(65535-50000)/256;
	TL0=(65535-50000)%256;
	
	t_005s++;
	if(t_005s==20)
	{
		t_s++;
		t_005s=0;
		if(t_s==60)
		{
			t_f++;
			t_s=0;
		}
		if(t_f==99)
		{
			t_f=0;
		}
	}
}
//**************************//

void Delay_Key(unsigned char k)
{
	while(k--);
}

void ScanKeys()          //秒表启动与暂停
{
	if(s4==0)
	{
		Delay_Key(100);
		if(s4==0)
		{
			TR0=~TR0;
			while(s4==0)
			{
				DisplayTime();
			}
		}
	}
	if(s5==0)               //秒表清零
	{
		Delay_Key(100);
		if(s5==0)
		{
			t_f=0;
			t_s=0;
			t_005s=0;
			while(s5==0)
			{
				DisplayTime();
			}
		}
	}
}

void main()
{
	InitTimer0();
	while(1)
	{
		DisplayTime();
		ScanKeys();
	}
}