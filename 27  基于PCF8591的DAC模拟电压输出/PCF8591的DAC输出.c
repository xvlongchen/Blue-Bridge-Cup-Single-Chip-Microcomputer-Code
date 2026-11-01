#include <reg52.h>
#include <iic.h>

sbit S4=P3^3;

unsigned char adc_value=0;        //AIN3采样数据
float adc_volt=0;                 //AIN3换算电压
unsigned int smg_volt=0;          //AIN3的显示电压
unsigned char stat_smg=1;         //当前显示界面标志
unsigned char mode=1;             //模式
//========================数码管段码=================
unsigned char code SMG_NoDot[18]={0xc0,0xf9,
    0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,
    0x88,0x80,0xc6,0xc0,0x86,0x8e,0xbf,0x7f};

unsigned char code SMG_Dot[10]={0x40,0x79,
    0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};
//==========================定义选项锁存器==============
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
//===========================延时函数=================
void Delay_SMG(unsigned char t)
{
	while(t--);
	while(t--);
}

void Delay_Key(unsigned char k)
{
	while(k--);
}
//==========================初始化数码管==============
void Init_SMG(unsigned char pos,unsigned char value)
{
	P0=0xff;
	Select_HC573(6);
	P0=0x01<<pos;
	Select_HC573(7);
	P0=value;
}
//===========================显示数码管===============
void Display_SMG()
{
	Init_SMG(0,SMG_NoDot[16]);
	Delay_SMG(100);
	Init_SMG(1,SMG_NoDot[mode]);
	Delay_SMG(100);
	Init_SMG(2,SMG_NoDot[16]);
	Delay_SMG(100);
	
	Init_SMG(5,SMG_Dot[smg_volt/100]);
	Delay_SMG(100);
	Init_SMG(6,SMG_NoDot[(smg_volt%100)/10]);
	Delay_SMG(100);
	Init_SMG(7,SMG_NoDot[smg_volt%10]);
	Delay_SMG(100);
	
	Select_HC573(6);
	P0=0xff;
	Select_HC573(7);
	P0=0xff;
}
//====================PCF8591电压采样处理函数=========
void Read_PCF8591_AIN3()
{
	IIC_Start();               //开始
	IIC_SendByte(0x90);        //PCF8591写设备地址
	IIC_WaitAck();             //等待回应
	IIC_SendByte(0x43);        //AIN3的控制字节
	IIC_WaitAck();
	IIC_Stop();
	
	IIC_Start();
	IIC_SendByte(0x91);        //PCF8591读设备地址
	IIC_WaitAck();
	adc_value=IIC_RecByte();   //读出AD采样数据
	IIC_SendAck(1);            //产生非应答信号
	IIC_Stop();
	
	adc_volt=adc_value*(5.0/255);
	smg_volt=adc_volt*100;
}
//========================PCF8591电压输出设备函数=====
void Set_PCF8591_DAC(unsigned char dat)
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	IIC_SendByte(0x43);            //输出DAC，转换AIN3
	IIC_WaitAck();
	IIC_SendByte(dat);             //设置DAC输出参数
	IIC_WaitAck();
	IIC_Stop();
}


//===========================按键扫描=================
void Scan_keys()
{
	if(S4==0)
	{
		Delay_Key(100);
		if(S4==0)
		{
			if(mode==1)
			{
				mode=2;
				Set_PCF8591_DAC(204);          //输出固定电压
				smg_volt=400;
			}
			else if(mode==2)
			{
				mode=3;
			}
			else if(mode==3)
			{
				mode=1;
				Set_PCF8591_DAC(102);
				smg_volt=200;
			}
			while(S4==0)
			{
				Display_SMG();
			}
		}
	}
}
//===========================初始化系统===============
void Init_System()
{
	Select_HC573(5);
	P0=0x00;
	Select_HC573(4);
	P0=0xff;
}
//==============================主函数================ 
void main()
{
	Init_System();
	while(1)
	{
		if(mode==3)
		{
			Read_PCF8591_AIN3();
			Set_PCF8591_DAC(adc_value);
		}
		Scan_keys();
		Display_SMG();
	}
}