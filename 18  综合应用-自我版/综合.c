#include "reg52.h"

sfr AUXR=0x8e;

unsigned char D_h=0;
unsigned char D_f=0;
unsigned char D_s=0;
unsigned char stat_led = 0xff;

sbit s5=P3^2;
sbit s4=P3^3;

unsigned char code SMG_duanma[18]={0xc0,0xf9,
	  0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
    0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};

void SelectHC573(unsigned int channel)
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

void Delay(unsigned int t)
{
	while(t--);
	while(t--);
}

void Delay_SMG(unsigned int t)
{
	while(t--);
}

void Delay_Key(unsigned char t)
{
	while(t--);
}
void DisplaySMG_Bit(unsigned char pos,unsigned char date)
{
	P0=0xff;
	SelectHC573(6);
	P0=0x01<<pos;
	SelectHC573(7);
	P0=date;
}

void SMG_Timer()
{
	DisplaySMG_Bit(7,SMG_duanma[D_s%10]);
	Delay_SMG(250);
	DisplaySMG_Bit(6,SMG_duanma[D_s/10]);
	Delay_SMG(250);
	DisplaySMG_Bit(5,SMG_duanma[16]);
	Delay_SMG(250);
	
	DisplaySMG_Bit(4,SMG_duanma[D_f%10]);
	Delay_SMG(250);
	DisplaySMG_Bit(3,SMG_duanma[D_f/10]);
	Delay_SMG(250);
	DisplaySMG_Bit(2,SMG_duanma[16]);
	Delay_SMG(250);
	
	DisplaySMG_Bit(1,SMG_duanma[D_h%10]);
	Delay_SMG(250);
	DisplaySMG_Bit(0,SMG_duanma[D_h/10]);
	Delay_SMG(250);
}

void Init_LED()
{
	unsigned int i;
	SelectHC573(4);
	for(i=0;i<=8;i++)
	{
		stat_led=0xff<<i;
		P0=stat_led;
		Delay(65535);
	}
	for(i=0;i<=8;i++)
	{
		stat_led=~(0xff<<i);
		P0=stat_led;
		Delay(65535);
	}
	SelectHC573(0);
}

void Init_SMG()
{
	unsigned int j;
	SelectHC573(7);
	P0=0x00;
	for(j=0;j<8;j++)
	{
		SelectHC573(6);
		P0=0x01<<j;
		Delay(65535);
	}
	for(j=0;j<8;j++)
	{
		SelectHC573(6);
		P0=~(0x01)<<j;
		Delay(65535);
	}
	SelectHC573(0);
}
//====================定时器========================//
void InitTimer()
{
	TMOD=0x21;
	TH0=(65535-10000)/256;
	TL0=(65535-10000)%256;
	
	EA=1;
	ET0=1;
	TR0=1;
}

unsigned int count=0;

void ServiceTimer() interrupt 1
{
	TH0=(65535-10000)/256;
	TL0=(65535-10000)%256;
	
	count++;
	if(count==100)
	{
		D_s++;
		count=0;
	}
	if(D_s==60)
	{
		D_s=0;
		D_f++;
		if(D_f==60)
		{
			D_f=0;
			D_h++;
		}
	}
}
//=================================================//
//====================串口=========================//
void InitUart()
{
	TMOD=0x21;
	TH1=0xfd;
	TL1=0xfd;
	TR1=1;
	
	SCON=0x50;
	AUXR=0X00;
	
	EA=1;
	ES=1;
}
//=========串口接收=======//
unsigned char command=0;
void ServiceUart() interrupt 4
{
	if(RI==1)
	{
		command=SBUF;
		RI=0;
	}
}
//=========串口发送=======//
void SendBute(unsigned char dat)
{
	SBUF=dat;
	while(TI==0);
	TI=0;
}
//=================================================//
//==============上位机解释=========================//
void ExecuteCommand()
{
	if(command!=0x00)
	{
		switch(command & 0xf0)
		{
			case 0xa0:
				SelectHC573(4);
				stat_led=(stat_led|0x0f) & (~command | 0xf0);
				P0=stat_led;
				SelectHC573(0);
				command=0x00;
			break;
			
			case 0xb0:
				SendBute((D_h/10<<4)|(D_h%10));
				SendBute((D_f/10<<4)|(D_f%10));
				SendBute((D_s/10<<4)|(D_s%10));
				command=0x00;
			break;
		}
	}
}
//=================================================//
void ScanKeys()
{
	if(s5==0)
	{
		Delay_Key(10);
		if(s5==0)
		{
			while(s5==0)
			{
				SMG_Timer();
			}
			SelectHC573(4);
			stat_led=(stat_led|0x40) & (~stat_led | 0xbf);
			P0=stat_led;
			SelectHC573(0);
		}
	}
	if(s4==0)
	{
		Delay_Key(10);
		if(s4==0)
		{
			while(s4==0)
			{
				SMG_Timer();
			}
			SelectHC573(4);
			stat_led=(stat_led|0x80) & (~stat_led | 0x7f);
			P0=stat_led;
			SelectHC573(0);
		}
	}
		
}

void InitSystem()
{
	SelectHC573(5);
	P0=0x00;
	SelectHC573(4);
	P0=stat_led;
	SelectHC573(0);
}

void main()
{
	InitSystem();
	Init_LED();
	Init_SMG();
	InitTimer();
	InitUart();
	while(1)
	{
		SMG_Timer();
		ExecuteCommand();
		ScanKeys();
	}
}