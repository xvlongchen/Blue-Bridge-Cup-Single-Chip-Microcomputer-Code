#include <reg52.h>
#include "ds1302.h"
#include "iic.h"
#include "intrins.h"

sfr P4=0xC0;
sbit h1 = P3^1;
sbit h2 = P3^2;
sbit h3 = P3^3;
sbit s1 = P4^4;
sbit s2 = P4^2;
sbit s3 = P3^5;
sbit s4 = P3^4;
unsigned char stat_my4 = 0;//防止stat_4一直为四，影响Led4
unsigned char stat_4 = 0;//四位数据输入完成标志
unsigned char stat_data = 0;//得到第一个旧数据标志
unsigned int old_value = 0;//旧的数据
unsigned char stat_led = 0xff;
unsigned int num_value = 0;//四位数值大小
unsigned char jilu_h = 0;
unsigned char jilu_m = 0;
unsigned char count_4 = 0;
unsigned char value_4[4] = {0};//键盘获取的四位数值分别存入数组中
unsigned char UI = 0;//0-时间界面 1-输入界面 2—记录界面
unsigned char t_h = 0;
unsigned char t_m = 0;
unsigned char t_s = 0;
unsigned char stat_jilu = 0;
unsigned char code SMG_NoDot[18]={0xc0,0xf9,
    0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
    0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};
unsigned char code read_addre[7] = {0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};
unsigned char code write_addre[7] = {0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};
unsigned char Intimer[7] = {0x59,0x09,0x23,0x03,0x03,0x07,0x24};
void Delay5ms()		//@12.000MHz
{
	unsigned char i, j;
 
	i = 59;
	j = 90;
	do
	{
		while (--j);
	} while (--i);
}
 
void Delay20ms()		//@12.000MHz
{
	unsigned char i, j, k;
 
	_nop_();
	_nop_();
	i = 1;
	j = 234;
	k = 113;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}
 
void Init_mytimer()
{
	unsigned char i = 0;
	Write_Ds1302_Byte(0x8e,0x00);
	for(i = 0;i < 7;i++)
	{
		Write_Ds1302_Byte(write_addre[i],Intimer[i]);
	}
	Write_Ds1302_Byte(0x8e,0x80);
}
void read_mytimer()
{
	t_h = Read_Ds1302_Byte(0x85);
	t_m = Read_Ds1302_Byte(0x83);
	t_s = Read_Ds1302_Byte(0x81);
}
unsigned char read_eeprom(unsigned char addre)
{
	unsigned char epr_value = 0;
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addre);
	IIC_WaitAck();
//	I2CStop();
	
	IIC_Start();
	IIC_SendByte(0xa1);
	IIC_WaitAck();
	epr_value = IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
	return epr_value;
}
void write_eeprom(unsigned char addre,unsigned char w_value)
{
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addre);
	IIC_WaitAck();
	IIC_SendByte(w_value);
	IIC_WaitAck();
	IIC_Stop();
}
void SelectHC573(unsigned char channel,unsigned char dat)
{
	P2 = (P2 & 0x1f) | 0x00;
	P0 = dat;
	switch(channel)
	{
		case 4:
			P2 = (P2 & 0x1f) | 0x80;
		break;
		case 5:
			P2 = (P2 & 0x1f) | 0xa0;
		break;
		case 6:
			P2 = (P2 & 0x1f) | 0xc0;
		break;
		case 7:
			P2 = (P2 & 0x1f) | 0xe0;
		break;
		case 0:
			P2 = (P2 & 0x1f) | 0x00;
		break;
	}
	P2 = (P2 & 0x1f) | 0x00;
}
void DelaySMG(unsigned int t)
{
	while(t--);
}
void DisplaySMG_Bit(unsigned char pos,unsigned char value)
{
	SelectHC573(6,0x01 << pos);
	SelectHC573(7,value);
	DelaySMG(500);
	SelectHC573(6,0x01 << pos);
	SelectHC573(7,0xff);
}
void DisplaySMG_All(unsigned char value)
{
	SelectHC573(6,0xff);
	SelectHC573(7,value);
}
void DisplaySMG_Info()
{
	switch(UI)
	{
		case 0:
			DisplaySMG_Bit(0,SMG_NoDot[t_h >> 4]);
			DisplaySMG_Bit(1,SMG_NoDot[t_h & 0x0f]);
			DisplaySMG_Bit(2,SMG_NoDot[16]);
			DisplaySMG_Bit(3,SMG_NoDot[t_m >> 4]);
			DisplaySMG_Bit(4,SMG_NoDot[t_m & 0x0f]);
			DisplaySMG_Bit(5,SMG_NoDot[16]);
			DisplaySMG_Bit(6,SMG_NoDot[(t_s >> 4) & 0x07]);
			DisplaySMG_Bit(7,SMG_NoDot[t_s & 0x0f]);
		break;
		case 1:
			DisplaySMG_Bit(0,0xc6);
			if(count_4 == 1)
			{
				DisplaySMG_Bit(7,SMG_NoDot[value_4[0]]);
			}
			else if(count_4 == 2)
			{
				DisplaySMG_Bit(6,SMG_NoDot[value_4[0]]);
				DisplaySMG_Bit(7,SMG_NoDot[value_4[1]]);
			}
			else if(count_4 == 3)
			{
				DisplaySMG_Bit(5,SMG_NoDot[value_4[0]]);
				DisplaySMG_Bit(6,SMG_NoDot[value_4[1]]);
				DisplaySMG_Bit(7,SMG_NoDot[value_4[2]]);
			}
			else if(count_4 == 4)
			{
				DisplaySMG_Bit(4,SMG_NoDot[value_4[0]]);
				DisplaySMG_Bit(5,SMG_NoDot[value_4[1]]);
				DisplaySMG_Bit(6,SMG_NoDot[value_4[2]]);
				DisplaySMG_Bit(7,SMG_NoDot[value_4[3]]);
			}
			
		break;
		case 2:
			DisplaySMG_Bit(0,0x86);
			DisplaySMG_Bit(3,SMG_NoDot[jilu_h >> 4]);
			DisplaySMG_Bit(4,SMG_NoDot[jilu_h & 0x0f]);
			DisplaySMG_Bit(5,SMG_NoDot[16]);
			DisplaySMG_Bit(6,SMG_NoDot[jilu_m >> 4]);
			DisplaySMG_Bit(7,SMG_NoDot[jilu_m & 0x0f]);
		break;
	}
}
void scan_key()
{
	h1 = 0;
	h2 = h3 = s1 = s2 = s3 = s4 = 1;
	if(s1 == 0)//6->0
	{
		Delay20ms();
		if(s1 == 0)
		{
			if(count_4 < 4)
			{
				value_4[count_4] = 0;
				count_4++;
			}
			while(s1 == 0)
			{
				DisplaySMG_Info();
			}
		}
	}
	else if(s2 == 0 && UI == 1)//10->1
	{
		Delay20ms();
		if(s2 == 0)
		{
			if(count_4 < 4)
			{
				value_4[count_4] = 1;
				count_4++;
			}
			while(s2 == 0)
			{
				DisplaySMG_Info();
			}
		}
	}
	else if(s3 == 0 && UI == 1)//14->2
	{
		Delay20ms();
		if(s3 == 0)
		{
			if(count_4 < 4)
			{
				value_4[count_4] = 2;
				count_4++;
			}
			while(s3 == 0)
			{
				DisplaySMG_Info();
			}
		}
	}
	else if(s4 == 0 && UI == 1)//18->3
	{
		Delay20ms();
		if(s4 == 0)
		{
			if(count_4 < 4)
			{
				value_4[count_4] = 3;
				count_4++;
			}
			while(s4 == 0)
			{
				DisplaySMG_Info();
			}
		}
	}
	
	h2 = 0;
	h1 = h3 = s1 = s2 = s3 = s4 = 1;
	if(s1 == 0)//5
	{
		Delay20ms();
		if(s1 == 0)
		{
			if(UI == 1)
			{
				count_4 = 0;
				value_4[0] = value_4[1] = value_4[2] = value_4[3] = 0;
				stat_jilu = 0;
				stat_my4 = 0;
			}
			while(s1 == 0)
			{
				DisplaySMG_Info();
			}
		}
	}
	else if(s2 == 0 && UI == 1)//9->4
	{
		Delay20ms();
		if(s2 == 0)
		{
			if(count_4 < 4)
			{
				value_4[count_4] = 4;
				count_4++;
			}
			while(s2 == 0)
			{
				DisplaySMG_Info();
			}
		}
	}
	else if(s3 == 0 && UI == 1)//13->5
	{
		Delay20ms();
		if(s3 == 0)
		{
			if(count_4 < 4)
			{
				value_4[count_4] = 5;
				count_4++;
			}
			while(s3 == 0)
			{
				DisplaySMG_Info();
			}
		}
	}
	else if(s4 == 0 && UI == 1)//17->6
	{
		Delay20ms();
		if(s4 == 0)
		{
			if(count_4 < 4)
			{
				value_4[count_4] = 6;
				count_4++;
			}
			while(s4 == 0)
			{
				DisplaySMG_Info();
			}
		}
	}
	
	h3 = 0;
	h2 = h1 = s1 = s2 = s3 = s4 = 1;
	if(s1 == 0)//4
	{
		Delay20ms();
		if(s1 == 0)
		{
			if(UI == 0)
			{
				UI = 1;
				count_4 = 0;
				stat_jilu = 0;
				stat_my4 = 0;
			}
			else if(UI == 1)
			{
				UI = 2;
				write_eeprom(0x00,(jilu_h/16)*10 + jilu_h%16);
				Delay5ms();
				write_eeprom(0x01,(jilu_m/16)*10 + jilu_m%16);
				Delay5ms();
				write_eeprom(0x02,num_value >> 8);
				Delay5ms();
				write_eeprom(0x03,num_value & 0x00ff);
				Delay5ms();
			}
			else
			{
				UI = 0;
			}
			while(s1 == 0)
			{
				DisplaySMG_Info();
			}
		}
	}
	else if(s2 == 0 && UI == 1)//8->7
	{
		Delay20ms();
		if(s2 == 0)
		{
			if(count_4 < 4)
			{
				value_4[count_4] = 7;
				count_4++;
			}
			while(s2 == 0)
			{
				DisplaySMG_Info();
			}
		}
	}
	else if(s3 == 0 && UI == 1)//12->8
	{
		Delay20ms();
		if(s3 == 0)
		{
			if(count_4 < 4)
			{
				value_4[count_4] = 8;
				count_4++;
			}
			while(s3 == 0)
			{
				DisplaySMG_Info();
			}
		}
	}
	else if(s4 == 0 && UI == 1)//16->9
	{
		Delay20ms();
		if(s4 == 0)
		{
			if(count_4 < 4)
			{
				value_4[count_4] = 9;
				count_4++;
			}
			while(s4 == 0)
			{
				DisplaySMG_Info();
			}
		}
	}
	if(count_4 == 1)
	{
		if(stat_jilu == 0)
		{
			jilu_h = Read_Ds1302_Byte(0x85);
			jilu_m = Read_Ds1302_Byte(0x83);
			stat_jilu = 1;
		}
	}
	if(count_4 == 4)
	{
		num_value = (value_4[0] * 1000) + (value_4[1] * 100) + (value_4[2] * 10) + value_4[3];
		if(stat_my4 == 0)
		{
			stat_4 = 1;
			stat_my4 = 1;
		}
		
	}
}
 
void led_control()
{
	if(UI == 0)
	{
		stat_led = stat_led | 0x04;
		stat_led = stat_led & 0xfe;
		SelectHC573(4,stat_led);
	}
	else if(UI == 1)
	{
		stat_led = stat_led | 0x01;
		stat_led = stat_led & 0xfd;
		SelectHC573(4,stat_led);
	}
	else if(UI == 2)
	{
		stat_led = stat_led | 0x02;
		stat_led = stat_led & 0xfb;
		SelectHC573(4,stat_led);
	}
	if(stat_data == 0 && stat_4 == 1)
	{
		old_value = num_value;
		stat_data = 1;
		stat_4 = 0;
	}
	if(stat_4 == 1)
	{
	if(old_value < num_value)
	{
		stat_led = stat_led & 0xf7;
		SelectHC573(4,stat_led);
	}
	else
	{
		stat_led = stat_led | 0x08;
		SelectHC573(4,stat_led);
	}
	stat_4 = 0;
	old_value = num_value;
}
}
void main()
{
	Init_mytimer();
	DisplaySMG_All(0xff);
	SelectHC573(4,0xff);
	SelectHC573(5,0x00);
	while(1)
	{
		scan_key();
		read_mytimer();
		DisplaySMG_Info();
		led_control();
	}
}