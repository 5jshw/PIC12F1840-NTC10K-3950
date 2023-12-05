#include <xc.h>
#include "KF2.h"

unsigned int getADCValue(unsigned char channel);	// 单次AD转换
unsigned int getADS(unsigned char ssa);				// 多次AD转换
void __interrupt() ISR(void);						// 中断处理函数
void ADsensing(void);								// AD转换，主程序
void Tempjudgements(void);							// 温度判断

//enum PWMmode { BRIGHTNESS_MODE, CONSTANT_MODE };// PWM灯光状态转换
int t;							// 电位器设定温度值
int v;							// NTC测量温度值
int y = 0;						// PWM灯光状态转换
unsigned int r;					// 互感器
unsigned long Rt;				// NTC当前电阻值
	unsigned long VR;				// NTC电压值
	char i;							// 电阻数组指示器
	char p;							// PWM增减指示器
	char add = 0;					// 电位器的采样频率
char timerActive = 0;			// 互感器计时标志位
unsigned int timerCounter = 0;	// 互感器启动计时器
	unsigned long ad1 = 0;			// ad1 NTC
	unsigned int ad2 = 0;			// ad2 电位器
	unsigned int ad3 = 0;			// ad3 模拟互感器输入
unsigned long const TABLE[] ={	177000, 166400, 156500, 147200, 138500, 130400, 122900, 115800, 109100, 97120, 
								91660, 86540, 81720, 77220, 72980, 69000, 65260, 61760, 58460, 55340, 
								52420, 49660, 47080, 44640, 42340, 40160, 38120, 36200, 34380, 32660, 
								31040, 29500, 28060, 26680, 25400, 24180, 23020, 21920, 20880, 19900, 
								18970, 18090, 17260, 16460, 15710, 15000, 14320, 13680, 13070, 12490, 
								11940, 11420, 10920, 10450, 10000, 9574, 9166, 8778, 8408, 8058, 
								7722, 7404, 7098, 6808, 6532, 6268, 6016, 5776, 5546, 5326, 
								5118, 4918, 4726, 4544, 4368, 4202, 4042, 3888, 3742, 3602, 
								3468, 3340, 3216, 3098, 2986, 2878, 2774, 2674, 2580, 2488, 
								2400, 2316, 2234, 2158, 2082, 2012, 1942, 1876, 1813, 1751
                             };  //  -30 ~ 70 温度对应电阻值 0 ~ 99 (100)
/*
	unsigned int const TABLE[] =	{9712, 9166, 8654, 8172, 7722, 7298, 6900, 6526, 6176, 5534, 5242, 4966, 4708, 4464, 4234, 4016, 3812, 3620, 3438, 3266, 
									3104, 2950, 2806, 2668, 2540, 2418, 2302, 2192, 2088, 1990, 1897, 1809, 1726, 1646, 1571, 1500, 1432, 1368, 1307, 1249, 
									1194, 1142, 1092, 1045, 1000, 957, 916, 877, 840, 805, 772, 740, 709, 680, 653, 626, 601, 577, 554, 532, 511, 491, 472, 
									454, 436, 420, 404, 388, 374, 360, 346, 334, 321, 309, 298, 287, 277, 267, 258, 248
									};  //   -20 ~ 60 温度对应电阻值，为了优化空间，所有值缩小一位数 0 ~ 80
*/
/*
unsigned int const TABLE[] =	{9910, 9345, 8815, 8319, 7854, 7418, 7009, 6625, 6275, 5925,              //-20~-11
								5607, 5307, 5026, 4761, 4512, 4277, 4056, 3848, 3651, 3466, 			//-10~-1
								3291, 																	//0
								3127, 2971, 2824, 2685, 2554, 2430, 2313, 2202, 2097, 1998, 			//1~10
								1904, 1815, 1731, 1651, 1575, 1503, 1435, 1370, 1309, 1250,             //11~20
								1195, 1142, 1092, 1045, 1000, 957, 916, 877, 840, 804, 					//21~30
								771, 739, 708, 679, 651, 625, 600, 576, 553, 531, 						//31~40
								510, 490, 471, 452, 435, 418, 402, 387, 372, 358, 						//41~50
								345, 332, 320, 308, 297, 286, 276, 266, 256, 247                        //51~60
								};
*/

void main(void) 
{
	timerActive = 0;		// 关闭计时器
    timerCounter = 0;		// 重置计时器计数
    
    setup();				// AD转换初始化函数调用，原函数位于KF1.c
    PWMinit();				// PWM初始化函数调用，原函数位于KF1.c

    __delay_ms(1000);		// 缓冲期
	T2CONbits.TMR2ON = 1;   // 开启时钟2
	TRISAbits.TRISA5 = 0;   // 允许输出
	
    while (1)				// 主循环
    {
		ADsensing();		// 函数调用, AD转换
		Tempjudgements();	// 函数调用, 温度判断
		
		__delay_ms(100);	// 延时一定时间，减少AD采样频率
    }
}

unsigned int getADCValue(unsigned char channel)		// 单次AD转换函数
{
    ADCON0bits.CHS = channel;                               // 选择AD通道
    __delay_ms(5);                                          // 改变AD通道后，需延时稳定
    ADCON0bits.GO = 1;                                      // 开始AD转换
    while (ADCON0bits.GO);                                  // 转换完成指示
    return (unsigned int)((ADRESH << 2) | (ADRESL >> 6));   // 返回高低位合并后的AD值
}

void ADsensing(void)				// AD转换
{
	if(add == 0)					// 错开多路AD转换的频率
	{
		ad1 = getADCValue(0x00);	// AD接口感应的AD值
		ad1 = 1024 - ad1;			// 由于NTC位于测温电路下端，所以温度升高时，电压下降，电阻上升。因此，将AD值反向
		VR = ad1 * 500 / 1024;		// 转换成电压值
		Rt = (unsigned long)(500 - VR) * 10000 / VR;		// 计算NTC当前的动态电阻值
		add++;
	}
	else if(add == 1)				// 电位器设定温度值
	{
		ad2 = getADCValue(0x01);	// 选择电位器
		t = (int)(ad2 * 10 / 128);	// 1024 * 10 / 128 = 80
		t += 10;					// 实际温度对照(0 ~ 100)(-30 ~ 70) 设定温度对照(10 ~ 90)(-20 ~ 60)  
		add++;
	}
	else if(add == 2)
	{
		ad3 = getADCValue(0x02);	// 模拟互感器
		r = ad3;					// 存储转换后的值
		add = 0;					// AD识别指示器
	}
	else add = 0;					// 防止异常情况
}

void Tempjudgements(void)			// 温度判断与电路异常情况判定
{
	if(v <= t)						// 实际温度低于设定温度-2时
	{
		B1 = 1;						// 继电器输出，开始加热
		y = 1;						// 更改PWM状态，最高亮度常量模式
		if (r < 100 || r > 900)		// 互感器判断，监测电路电流状态
		{
			timerActive = 1;		// 启动计时器，消除抖动
		}
		else
		{
			timerActive = 0;		// 关闭计时器
			timerCounter = 0;		// 重置计时器计数
		}
	}
	else if(v > (t + 5))			// 实际温度高于设定温度时
	{
		B1 = 0;						// 继电器停止输出，结束加热
		y = 0;						// 更改PWM状态，呼吸灯模式
		timerActive = 0;			// 关闭计时器
		timerCounter = 0;			// 重置计时器计数
	}
}

void __interrupt() ISR(void)		// 中断处理函数
{
	if(PIR1bits.TMR2IF == 1)		// 检测时钟2是否溢出中断
	{
		PIE1bits.TMR2IE = 0;		// 禁止时钟2溢出中断
		T2CONbits.TMR2ON = 0;		// 停止计数
		PIR1bits.TMR2IF = 0;		// 时钟2溢出标志清零
		if(y == 0)
		{
			if(CCPR1L >= PR2)		// 检测脉宽是否到达极值
			{
				p = 0;
			}
			else if(CCPR1L <= 0)
			{
				p = 1;
			}
			if(p == 1)				// 判断PWM增减指示器状态
			{
				CCPR1L++;
			}
			else if(p == 0)
			{
				CCPR1L--;
			}
		}
		else if(y == 1)
		{
			CCPR1L = 0x3F;			// 占空比公式：(CCPR1L + CCP1CONbits.DC1B)/(4(PR2 + 1))
		}		
		PIE1bits.TMR2IE = 1;		// 允许时钟2溢出中断
		T2CONbits.TMR2ON = 1;		// 开启时钟2
	}
    
	if(PIR1bits.ADIF == 1)			// 检查AD是否发生中断
	{
		PIE1bits.ADIE = 0;			// 禁止中断发生
		PIR1bits.ADIF = 0;			// 清除中断标志
		for(i = 0; i < 100; i++)
		{
			if(Rt >= TABLE[i])		// 阻值对照表是按照阻值从大到小排列，温度是按照从低到高排列
			{
				v = i;				// 存储电阻对应温度值
				break;				// 存储后推出循环
			}
			else if(Rt < TABLE[99])	// 当在组织对照表中找不到对应温度后，即代表当前温度已超出表中范围
			{
				v = 200;		// 返回此数值则表示温度超过可测量范围
				break;
			}
		}
		if (timerActive)				// 计时器判断
		{
			timerCounter++;				// 每次中断，增加计时器计数
			if(timerCounter >= 5)		// 计数一定时间后(消除抖动)，电流异常，切断继电器输出，报警
			{
				B1 = 0;					// 关闭继电器输出
				T2CONbits.TMR2ON = 0;	// 关闭时钟2
				CCP1CON = 0;			// 关闭PWM
				ADCON0bits.ADON = 0;	// 关闭 AD 转换器
				PORTAbits.RA5 = 0;		// 初始化PWM灯光状态
				while(1)				// 在继电器输出时，当互感器返回值异常则进入该循环，无法退出
				{
					PORTAbits.RA5 = !PORTAbits.RA5;	// 使PWM灯光闪烁
					__delay_ms(100);
				}
			}
		}
		PIE1bits.ADIE = 1;              // 一次中断执行完毕，允许中断继续发生
	}
}