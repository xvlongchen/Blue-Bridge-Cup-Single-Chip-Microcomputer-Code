#include "reg52.h"
#include "ds1302.h"
#include "iic.h"
#include "onewire.h"

//无小数点
unsigned char code SMGNoDot_duanma[10]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};
//有小数点
unsigned char code SMGDot_duanma[10]={0x40,0x79,0x24,0x30,0x19,0x12,0x02,0x78,0x00,0x10};
//DS1302写地址
unsigned char code Write_ds1302_addr[10]={0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};
//DS1302读地址
unsigned char code Read_ds1302_addr[10]={0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};
//开始时间
unsigned char Timer[7]={0x05,0x03,0x13,0x12,0x04,0x05,0x24};

//实时时间
unsigned char hour=0;   //时
unsigned char min=0;    //分
unsigned char sec=0;    //秒

//触发时间
unsigned char hour_trigger=0;
unsigned char min_trigger=0;
//状态标志
unsigned char stat_smg=1;      //数码管当前界面标志
unsigned char stat_led=0xff;   //led灯控制状态
unsigned char stat_trigger=0;  //光照电压触发状态
unsigned char stat_smg_old=1; //数码管原状态
//温度数据：保留1位小数，放大10倍
unsigned int temp_average=0;   //平均温度
unsigned int temp_smg=0;       //当前采集温度
unsigned int temp_max=0;       //最大温度
unsigned int temp_param=300;   //温度参数，默认30
//湿度数据：保留1位小数，放大10倍
unsigned int humi_smg=0;       //当前采集湿度
unsigned int humi_average=0;   //平均湿度
unsigned int humi_max=0;       //最大湿度
//温度和湿度采集缓存
unsigned int temp[10]={0};  
unsigned int humi[10]={0};

unsigned char trigger=0;     //触发次数
unsigned int freq=0;         //频率变量
unsigned int count_t1=0;     //10ms单位累计
unsigned int count_t2=0;     //10ms单位累计
unsigned int count_t3=0;     //10ms单位累计
unsigned int count_key=0;    //10ms单位累计

unsigned char adc_value0=0;        //上一次光照电压
unsigned char adc_value1=0;        //本次光照电压

//S9按键按下标志
bit f_key=0;

sfr P4=0xC0;
//2X2矩阵键盘的行列定义
//S4,S5,S8,S9
sbit R1=P3^3;
sbit R2=P3^2;
sbit C1=P4^4;
sbit C2=P4^4;
//=================寄存器通道选择========
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
//===============初始化数码管============
//数码管延时函数
void Delay_smg(unsigned int t)
{
	while(t--);
}

//初始化数码管
void Init_smg(unsigned char pos,unsigned char value)
{
	Select_HC573(6,0x01<<pos);
	Select_HC573(7,value);
	Delay_smg(500);
	Select_HC573(6,0x01<<pos);
	Select_HC573(7,0xff);
}
//===========全部数码关静态显示=====
void Display_smg_all(unsigned char value)
{
	Select_HC573(6,0xff);
	Select_HC573(7,value);
}
//数码管显示
void Display_smg()
{
	stat_led|=0x07;
	switch(stat_smg)
	{
		//时间界面
		case 1:
			stat_led&=~0x01;					//设置L1,L2,L3熄灭状态
			//时
			Init_smg(0,SMGNoDot_duanma[hour>>4]);
			Init_smg(1,SMGNoDot_duanma[hour&0x0f]);
			Init_smg(2,0xbf);
			//分
			Init_smg(3,SMGNoDot_duanma[min>>4]);
			Init_smg(4,SMGNoDot_duanma[min&0x0f]);
			Init_smg(5,0xbf);
			//秒
			Init_smg(6,SMGNoDot_duanma[(sec>>4)&0x07]);
			Init_smg(7,SMGNoDot_duanma[sec&0x0f]);
		break;
		//温度回显界面
		case 2:
			stat_led&=~0x02;            //设置点亮L2
			//符号C：0xc6
			Init_smg(0,0xc6);
			if(trigger!=0)
			{
				//最大温度
				Init_smg(2,SMGNoDot_duanma[temp_max/100]);
				Init_smg(3,SMGNoDot_duanma[(temp_max/10)%10]);
				//平均温度
				Init_smg(5,SMGNoDot_duanma[temp_average/100]);
				Init_smg(6,SMGDot_duanma[(temp_average/10)%10]);
				Init_smg(7,SMGNoDot_duanma[temp_average%10]);
			}
		break;
			//湿度回显界面
		case 3:
			stat_led&=~0x02;          //设置点亮L2
			//符号H：0x89
			Init_smg(0,0x89);
			if(trigger!=0)
			{
				//最大湿度
				Init_smg(2,SMGNoDot_duanma[humi_max/100]);
				Init_smg(3,SMGNoDot_duanma[(humi_max/10)%10]);
				//平均湿度
				Init_smg(5,SMGNoDot_duanma[humi_average/100]);
				Init_smg(6,SMGDot_duanma[(humi_average/10)%10]);
				Init_smg(7,SMGNoDot_duanma[humi_average%10]);
			}
		break;
		//时间回显界面
		case 4:
			stat_led&=~0x02;          //设置点亮L2
			//符号F：0x8e
			Init_smg(0,0x8e);
			//触发次数
			Init_smg(1,SMGNoDot_duanma[trigger/10]);
			Init_smg(2,SMGNoDot_duanma[trigger%10]);
			if(trigger!=0)
			{
				//触发时间
				Init_smg(3,SMGNoDot_duanma[hour_trigger>>4]);
				Init_smg(4,SMGNoDot_duanma[hour_trigger%0x0f]);
				Init_smg(5,0xbf);
				Init_smg(6,SMGNoDot_duanma[min_trigger>>4]);
				Init_smg(7,SMGNoDot_duanma[min_trigger%0x0f]);
			}
		break;
		//参数界面
		case 5:
			//符号P:0x8c
			Init_smg(0,0x8c);
			Init_smg(6,SMGNoDot_duanma[temp_param/100]);
			Init_smg(7,SMGNoDot_duanma[(temp_param/10)%10]);
		break;
		//温湿度界面
		case 6:
			stat_led&=~0x04;					//设置点亮L3
			//符号E:0x86
			Init_smg(0,0x86);
			//温度
			Init_smg(3,SMGNoDot_duanma[temp_smg/100]);
			Init_smg(4,SMGNoDot_duanma[(temp_smg/10)%10]);
			Init_smg(5,0xbf);
			if(humi_smg==99)             //无效数据，显示AA
			{
				Init_smg(6,SMGNoDot_duanma[10]);
				Init_smg(7,SMGNoDot_duanma[10]);
			}
			else
			{
				Init_smg(6,SMGNoDot_duanma[humi_smg/100]);
				Init_smg(7,SMGNoDot_duanma[(humi_smg/10)%10]);
			}
		break;	
	}
	Select_HC573(4,stat_led);
}
//================定时器初始化===========
void Init_Timer0_Timer1()
{
	//定时器1计数初值设置：10ms
	TH1=(65536-10000)/256;
	TL1=(65536-10000)%256;
	//T0设置为计数功能，初始值为0x0000
	TH0=0;
	TL0=0;
	//T0计数，T1定时，都使用16位不可重装模式
	TMOD=0x15;
	
	ET1=1;         //使能T1中断
	EA=1;           //打开总中断
	TR0=1;
	TR1=1;
}
//================定时器T1的中断服务函数==
void Service_Timer() interrupt 3
{
	TH1=(65536-10000)/256;
	TL1=(65536-10000)%256;
	count_t1++;
	count_t2++;
	//0.1秒定时，当采集温度>温度参数时。L4闪烁
	if(count_t2==10)
	{
		count_t2=0;
		if((stat_led&0x08)==0x00)
		{
			stat_led|=0x08;
			Select_HC573(4,stat_led);
		}
		else if(temp_smg>temp_param)
		{
			stat_led&=~0x08;
			Select_HC573(4,stat_led);
		}
	}
	//1秒定时，读取计数器0的脉冲计数值，得到NE555的频率
	if(count_t1==100)
	{
		count_t1=0;
		//1秒钟的脉冲计数，即为信号的频率
		freq=(TH0<<8)|TL0;
		TH0=0;
		TL0=0;
	}
	//发生触发时，延时3秒后，切换回原有的显示界面
	if(stat_trigger==1)
	{
		count_t3++;
		if(count_t3==300)
		{
			stat_smg=stat_smg_old;
			count_t3=0;
			stat_trigger=0;
		}
	}
	//当在时间回显界面，S9按下，计算按下时间
	if(f_key==1)
	{
		count_key++;
	}
}
//=================DS1302================
//初始化DS1302的参数
void Init_DS1302()
{
	unsigned char i;
	Write_Ds1302_Byte(0x8e,0x00);
	for(i=0;i<7;i++)
	{
		Write_Ds1302_Byte(Write_ds1302_addr[i],Timer[i]);
	}
	Write_Ds1302_Byte(0x8e,0x80);
}

//读取ds1302时间
void Read_ds1302_Timer()
{
	sec=Read_Ds1302_Byte(0x81);
	min=Read_Ds1302_Byte(0x83);
	hour=Read_Ds1302_Byte(0x85);
}
//=========设置PCF8591电压转换通道=======
void Set_PCF8591_ADC(unsigned char ain)
{
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(ain);
	I2CWaitAck();
	I2CStop();
}
//============读取上次的转换结果并启动本次转换====
unsigned char Read_PCF8591_ADC()
{
	unsigned char tmp;
	I2CStart();
	I2CSendByte(0x91);
	I2CWaitAck();
	tmp=I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	
	return tmp;
}
//======获取当前光照时间并判断是否触发=========
void GetADC_and_Check_Tigger()
{
	//只有在未触发时，才进行采样
	if(stat_trigger==0)
	{
		adc_value1=Read_PCF8591_ADC();
		if(adc_value1<=50&&adc_value0>50)
		{
			stat_trigger=1;
		}
		adc_value0=adc_value1;
	}
}
//=============DS18b20温度读出与显示=====
void Begin_18B20()
{
	char i;
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	//等待第一次温度数据采集完成，不然显示默认值85.00
	for(i=0;i<8;i++)
	{
		Delay_smg(60000);
	}
}
//按照要求，对采集到的当前温度处理
void Handle_Temp()
{
	unsigned int sum=0;
	unsigned char i;
	//将当前温度放入缓存
	temp[(trigger-1)%10]=temp_smg;
	//获取最大温度
	if(temp_smg>temp_max)
	{
		temp_max=temp_smg;
	}
	//获取平均温度
	if(trigger<10)
	{
		for(i=0;i<trigger;i++)
		{
			sum+=temp[i];
		}
		temp_average=sum/trigger;
	}
	else
	{
		for(i=0;i<10;i++)
		{
			sum+=temp[i];
		}
		temp_average=sum/10;
	}
}
//采集DS18b20最新温度
void Read_Display_DS18b20()
{
	unsigned char LSB,MSB;
	unsigned int dat=0;
	float temp=0;
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	Display_smg();
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	LSB=Read_DS18B20();       //第0字节，温度低8位
	MSB=Read_DS18B20();       //第1字节，温度高8位
	
	dat=MSB;
	dat=(dat<<8)|LSB;
	//判断符号，处理正温度
	if((dat&0xf800)==0x0000)
	{
		temp=dat*0.0625;         //计算实际温度值
	}
	temp_smg=temp*10;
	Display_smg();             //动态刷新数码管
	if(humi_smg!=5555)         //如果湿度有效，处理当前温度
	{
		Handle_Temp();
	}
}
//=================频率至湿度转换函数====
void Handle_Humi()
{
	unsigned int sum=0;
	unsigned char i;
	//将当前湿度放入缓存
	humi[(trigger-1)%10]=humi_smg;
	//获取最大湿度
	if(humi_smg>humi_max)
	{
		humi_max=humi_smg;
	}
	//获取平均湿度
	if(trigger<10)
	{
		for(i=0;i<trigger;i++)
		{
			sum+=humi[i];
		}
		humi_average=sum/trigger;
	}
	else
	{
		for(i=0;i<10;i++)
		{
			sum+=humi[i];
		}
		humi_average=sum/10;
	}
}
//将频率转换对应湿度
void Freq_to_Humi()
{
	float humi=0;
	if(freq>=200&&freq<=200)
	{
		humi=(80.0/1800.0)*(freq-200)+10;
		humi_smg=humi*10;                       //有效数据
		stat_led|=0x10;
		Select_HC573(4,stat_led);               //L5熄灭
		
		trigger++;                  				//触发次数加1
		Read_ds1302_Timer();                 //读取当前时间
		hour_trigger=hour;                   //记录触发时间
		min_trigger=min;
		
		Handle_Humi();							//处理湿度数据
	}
	else
	{
		humi_smg=5555;     					//无效数据
		stat_led&=~0x10;						
		Select_HC573(4,stat_led);               //L5点亮
	}
}
//=================比较处理采集的温湿度=======
void Compare_temp_and_humi()
{
	unsigned char n;
	if(trigger>=2) 										//触发次数>=2
	{
		n=(trigger-1)%10;								//获取缓存数据的下标
		if(n==0)												//下标为0需要特殊处理
		{
			if(temp[0]>temp[9]&&humi[0]>humi[9])
			{
				stat_led&=~0x20;
				Select_HC573(4,stat_led);               //L6点亮
			}
			else
			{
				stat_led|=0x20;
				Select_HC573(4,stat_led);               //L6熄灭
			}
		}
		else
		{
			if(temp[n]>temp[n-1]&&humi[n]>humi[n-1])
			{
				stat_led&=~0x20;
				Select_HC573(4,stat_led);               //L6点亮
			}
			else
			{
				stat_led|=0x20;
				Select_HC573(4,stat_led);               //L6熄灭
			}
		}
	}
}
//=================按键的功能实现函数=====
//S4按键的功能
void S4_Funtion()
{
	if(stat_smg==1)
	{
		stat_smg=2;
	}
	else if(stat_smg==2|stat_smg==3|stat_smg==4)
	{
		stat_smg=5;
	}
	else if(stat_smg==5)
	{
		stat_smg=1;
	}
}
//S5按键的功能
void S5_Funtion()
{
	if(stat_smg==2)
	{
		stat_smg=3;
	}
	else if(stat_smg==3)
	{
		stat_smg=4;
	}
	else if(stat_smg==4)
	{
		stat_smg=2;
	}
}
//S8按键功能
void S8_Funtion()
{
	if(stat_smg==5)									//在参数界面下，温度参数加1
	{
		if(temp_param<990)            //最大为99
		{
			temp_param=temp_param+10;
		}
	}
}
//S9按键的功能
void S9_Funtion()
{
	if(stat_smg==5)
	{
		if(temp_param>0)
		{
			temp_param=temp_param-10;
		}
	}
	else if(stat_smg==4)								//在时间回显界面下
	{
		count_key=0;
		f_key=1;
		while(C2==0)
		{
			Display_smg();
		}
		f_key=0;
		if(count_key>200)									//长按超过2秒，清楚所有记录数据
		{
			trigger=0;
			temp_max=0;
			temp_average=0;
			humi_max=0;
			humi_average=0;
		}
	}
}
//==============矩阵键盘扫描处理=========
void Scan_Keys_2x2()
{
	R1=0;
	R2=1;
	//扫描S4减
	if(C1==0)
	{
		Delay_smg(1000);
		if(C1==0)
		{
			S4_Funtion();
			while(C1==0)
			{
				Display_smg();
			}
		}
	}
	
	//扫描S8减
	if(C2==0)
	{
		Delay_smg(1000);
		if(C2==0)
		{
			S8_Funtion();
			while(C2==0)
			{
				Display_smg();
			}
		}
	}
	
	R1=1;
	R2=0;
	//扫描S5减
	if(C1==0)
	{
		Delay_smg(1000);
		if(C1==0)
		{
			S5_Funtion();
			while(C1==0)
			{
				Display_smg();
			}
		}
	}
	
	//扫描S9减
	if(C2==0)
	{
		Delay_smg(1000);
		if(C2==0)
		{
			S9_Funtion();
			while(C1==0)
			{
				Display_smg();
			}
		}
	}
}
	
//================系统初始化=============
void Init_System()
{
	Select_HC573(0,0x00);
	Select_HC573(5,0x00);
	Select_HC573(4,0xff);
	Display_smg_all(0xff);
}
//=================主函数================
void main()
{
	Init_DS1302();
	Init_System();
	Begin_18B20();
	Set_PCF8591_ADC(0x01);
	Read_PCF8591_ADC();
	Init_Timer0_Timer1();
	while(1)
	{
		Read_ds1302_Timer();
		GetADC_and_Check_Tigger();
		if(stat_trigger==1)
		{
			Freq_to_Humi();
			Read_Display_DS18b20();
			Compare_temp_and_humi();
			stat_smg_old=stat_smg;
			stat_smg=6;
			while(stat_trigger==1)
			{
				Display_smg();
			}
		}
		Scan_Keys_2x2();
		Display_smg();
	}
}