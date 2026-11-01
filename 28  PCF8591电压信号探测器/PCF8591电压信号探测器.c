#include "reg52.h"
#include "iic.h"

sbit S4=P3^3;
sbit S5=P3^2;
sbit S6=P3^1;

unsigned char code SMG_NoDot[18]={0xc0,0xf9,
    0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
    0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};

unsigned char code SMG_Dot[10]={0x40,0x79,
    0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};

unsigned int adc_value=0;        //AIN3的采样数据
float adc_volt=0;                //AIN3的换算电压
unsigned int smg_volt=0;         //AIN3的显示电压
unsigned int param=300;          //电压参数,3.00v
unsigned char stat_smg=1;        //当前显示界面标志
unsigned char num=0;             //报警计数变量
unsigned int count_t=0;          //50ms单位累计变量
unsigned char stat_led=0xff;     //LED灯控制状态
unsigned char F_shan=0;          //秒闪标志
unsigned char F_time=0;          //计时标志
//========================延时函数===============
void Delay_SMG(unsigned char t)
{
	while(t--);
	while(t--);
}
//=======================配置通道================
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
//=======================初始化数码管=============
void Init_SMG(unsigned char pos,unsigned char value)
{
	P0=0xff;
	Select_HC573(6);
	P0=0x01<<pos;
	Select_HC573(7);
	P0=value;
}
//========================显示数码管==============
void Display_SMG()
{
	switch(stat_smg)
	{
		//电压数据界面
		case 1:
			Init_SMG(0,0xc1);
			Delay_SMG(100);
		
		  Init_SMG(5,SMG_Dot[smg_volt/100]);
			Delay_SMG(100);
			Init_SMG(6,SMG_NoDot[(smg_volt%100)/10]);
			Delay_SMG(100);
			Init_SMG(7,SMG_NoDot[smg_volt%10]);
			Delay_SMG(100);
		break;
		
		//参数设置界面
		case 2:
			Init_SMG(0,0x8c);
			Delay_SMG(100);
		
		  Init_SMG(5,SMG_Dot[param/100]);
			Delay_SMG(100);
			Init_SMG(6,SMG_NoDot[(param%100)/10]);
			Delay_SMG(100);
			Init_SMG(7,SMG_NoDot[param%10]);
			Delay_SMG(100);
		break;
		
		//报警计数界面
		case 3:
			Init_SMG(0,0x92);
			Delay_SMG(100);
		
			Init_SMG(7,SMG_NoDot[num%10]);
			Delay_SMG(100);
			if(num/10!=0)
			{
				Init_SMG(6,SMG_NoDot[num/10]);
				Delay_SMG(100);
			}
		break;
	}
	Select_HC573(6);
	P0=0xff;
	Select_HC573(7);
	P0=0xff;
}
//=======================定时器T0初始化===========
void Init_Timer0()
{
	//定时器计数初值设置：50ms
	TH0=(65536-50000)/256;
	TL0=(65536-50000)%256;
	
	TMOD=0x01;         //IO模式1；16位不可重装模式
	ET0=1;            //使能定时器0的中断
	EA=1;              //打开总中断
	TR0=1;             //启动定时器0
}
//=======================定时器T0的中断服务函数===
void Service_Timer0() interrupt 1
{
	TH0=(65536-50000)/256;
	TL0=(65536-50000)%256;
	
	if(F_time==1)                      //启动报警计时
	{
		count_t++;
		if(count_t%20==0)
		{
			if(num!=99)
			{
				num++;                       //报警计数累计
			} 
			if(F_shan==1)                  //启动秒闪功能
			{
				if((stat_led&0x80)==0x80)
				{
					stat_led&=~0x80;           //点亮L8灯
				}
				else
				{
					stat_led|=0x80;            //熄灭L8灯
				}
			}
		}
		
		if(count_t>120)                 //超过6秒
		{
			stat_led&=~0x07;              //点亮L1，L2，L3
			F_shan=1;
		}
		else if(count_t>80)             //超过4秒
		{
			stat_led&=~0x03;              //点亮L1，L2
		}
		else if(count_t>40)             //超过2秒
		{
			stat_led&=~0x01;              //点亮L1
		}
		else
		{
			stat_led=0xff;
		}
	}
}
//=======================PCF8591电压采样处理函数===
unsigned char Read_PCF8591_AIN3()
{
	unsigned char tmp;
	
	IIC_Start();
	IIC_SendByte(0x90);           //PCF8591写设备地址
	IIC_WaitAck();
	
	IIC_SendByte(0x03);           //AIN3通道，可调电阻电压
	IIC_WaitAck();
	IIC_Stop();
	
	IIC_Start();
	IIC_SendByte(0x91);           //PCF8591读设备地址
	IIC_WaitAck();
	tmp=IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
	
	return tmp;
}
//======================电压数据应用分析处理======
void App_Volt()
{
	unsigned char i;
	//采样通道3的电压，并做简单的均值滤波处理
	adc_value=0;
	for(i=0;i<3;i++)
	{
		adc_value+=Read_PCF8591_AIN3();
	}
	adc_value=adc_value/3;
	//将ADC采样到的数据换算成对应的电压值
	adc_volt=adc_value*(5.0/255);
	smg_volt=adc_volt*100;
	//当前电压与电压参数的比较处理
	if(smg_volt<param)
	{
		if(F_time==0)
		{
			F_time=1;
		}
	}
	else
	{
		F_time=0;
		F_shan=0;
		count_t=0;
		num=0;
		stat_led|=0xff;
	}
	Display_SMG();
	Select_HC573(4);
	P0=stat_led;
}
//====================按键扫描处理函数============
void Scan_Keys()
{
	if(S4==0)                        //切换界面
	{
		Delay_SMG(500);
		if(S4==0)
		{
			if(stat_smg==1)
			{
				stat_smg=2;                //切换至：参数设置
			}
			else if(stat_smg==2)
			{
				stat_smg=3;                //切换至：报警计数
			}
			else if(stat_smg==3)
			{
				stat_smg=1;                //切换至：电压数据
			}
			while(S4==0)                 //松手检测
			{
				App_Volt();               //保持电压采样与显示
			}
		}
	}
	
	if(S5==0)
	{
		Delay_SMG(500);
		if(S5==0)
		{
			if(stat_smg==2)
			{
				if(param!=0)
				{
					param=param-50;
				}
				else
				{
					param=500;
				}
			}
			while(S5==0)
			{
				App_Volt(); 
			}
		}
	}
	
	if(S6==0)
	{
		Delay_SMG(500);
		if(S6==0)
		{
			if(stat_smg==2)
			{
				if(param!=500)
				{
					param=param+50;
				}
				else
				{
					param=0;
				}
			}
			while(S6==0)
			{
				App_Volt();
			}
		}
	}
}
//=======================系统初始化===============
void Init_System()
{
	Select_HC573(5);
	P0=0x00;
	Select_HC573(4);
	P0=0xff;
}
//=========================主函数==================
void main()
{
	Init_Timer0();
	Init_System();
	while(1)
	{
		App_Volt();
		Scan_Keys();
	}
}

//先进行电压采集(App_Volt),判断状态->在中断服务函数里面进行判断->