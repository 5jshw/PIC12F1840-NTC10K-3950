#include <xc.h>
//#include <pic12f1840.h>
#include "KF2.h"

unsigned int getADCValue(unsigned char channel);        //单次AD转换
unsigned int getADS(unsigned char ssa);                 //多次AD转换
void __interrupt() ISR(void);                           //中断处理函数
int t, v, y = 0;
unsigned int r;
unsigned long Rt;                                       //VR 当前电压值
unsigned int timerCounter = 0;  // 计时器计数器
char timerActive = 0;  // 标志位，表示计时器是否激活

unsigned int const TABLE[] = {9712, 9166, 8654, 8172, 7722, 7298, 6900, 6526, 6176, 5534, 5242, 4966, 4708, 4464, 4234, 4016, 3812, 3620, 3438, 3266, 
                              3104, 2950, 2806, 2668, 2540, 2418, 2302, 2192, 2088, 1990, 1897, 1809, 1726, 1646, 1571, 1500, 1432, 1368, 1307, 1249, 
                              1194, 1142, 1092, 1045, 1000, 957, 916, 877, 840, 805, 772, 740, 709, 680, 653, 626, 601, 577, 554, 532, 511, 491, 472, 
                              454, 436, 420, 404, 388, 374, 360, 346, 334, 321, 309, 298, 287, 277, 267, 258, 248
                             };  //   -20 ~ 60 温度对应电阻值，为了优化空间，所有值缩小一位数 0 ~ 80

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
unsigned long ad1 = 0;              //ad1  NTC
unsigned int ad2 = 0, ad3 = 0;      //ad2  电位器, ad3 模拟互感器输入

void main(void) 
{
	timerActive = 0;		// 关闭计时器
    timerCounter = 0;		// 重置计时器计数
    char add = 1;			//电位器的采样频率
    setup();				//AD转换初始化函数调用，原函数位于KF1.c
    PWMinit();				//PWM初始化函数调用，原函数位于KF1.c
	CCPR1L = 0;
    __delay_ms(1200);		//缓冲期
	TRISAbits.TRISA5 = 0;   //允许输出
	T2CONbits.TMR2ON = 1;   //开启时钟2
	
    while (1)				//主循环
    {
        if(add == 1)						//错开多路AD转换的频率
        {
            unsigned long VR;
            ad1 = getADCValue(0x00);		//AD接口感应的AD值
            ad1 = 1024 - ad1;				//由于NTC位于测温电路下端，所以温度升高时，电压下降，电阻上升。因此，将AD值反向处理
            VR = ad1 * 500 / 1024;			//转换成电压值
            Rt = (unsigned long)(500 - VR) * 1000 / VR;		//计算NTC当前的动态电阻值
            add++;
        }
        else if(add == 2)					//电位器设定温度值
        {
            ad2 = getADCValue(0x01);		//0000 1001
            t = (int)(ad2 * 10 / 128);		//1024 * 10 / 128 = 80
            add++;
        }
        else
        {
            ad3 = getADCValue(0x02);		//模拟互感器
            r = ad3;
            add = 1;						//AD识别指示器
        }
		
		if(v <= (t - 2))					//温度判断
		{
			B1 = 1;
			y = 1;
			if (r < 100 || r > 900)			//互感器判断
			{
				timerActive = 1;			// 启动计时器
			}
			else
			{
				timerActive = 0;			// 关闭计时器
				timerCounter = 0;			// 重置计时器计数
			}
		}
		else if(v > t)
		{
			B1 = 0;
			y = 0;
			timerActive = 0;				// 关闭计时器
			timerCounter = 0;				// 重置计时器计数
		}
		__delay_ms(200);
    }
}

//单次AD转换函数
unsigned int getADCValue(unsigned char channel)
{
    ADCON0bits.CHS = channel;                               //选择AD通道
    __delay_ms(5);                                          //改变AD通道后，需延时稳定
    ADCON0bits.GO = 1;                                      //开始AD转换
    while (ADCON0bits.GO);                                  //转换完成指示
    return (unsigned int)((ADRESH << 2) | (ADRESL >> 6));   //返回高低位合并后的AD值
}

void __interrupt() ISR(void)            //中断处理函数
{
	if(PIR1bits.TMR2IF == 1)			//检测时钟2是否溢出中断
	{
		char p;							//p PWM增减指示器
		PIE1bits.TMR2IE = 0;			//禁止时钟2溢出中断
		T2CONbits.TMR2ON = 0;			//停止计数
		PIR1bits.TMR2IF = 0; 			//时钟2溢出标志清零
		if(y == 0)
		{
			if(CCPR1L >= PR2)				//检测脉宽是否超过周期最大值
			{
				p = 0;
			}
			else if(CCPR1L <= 0)
			{
				p = 1;
			}
			if(p == 1)                      //判断PWM增减指示器状态
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
			CCPR1L = PR2 - 1;
		}
		PIE1bits.TMR2IE = 1;			//允许时钟2溢出中断
		T2CONbits.TMR2ON = 1;           //开启时钟2
	}
    
	if(PIR1bits.ADIF == 1)              //检查AD是否发生中断
	{
		PIE1bits.ADIE = 0;              //禁止中断发生
		PIR1bits.ADIF = 0;              //清除中断标志
		char i;
		for(i = 0; i < 80; i++)
		{
			if((unsigned int)Rt >= TABLE[i])
			{
				v = i;
				break;
			}
		}
/*
		if(v <= (t - 2))
		{
			B1 = 1;
			PIE1bits.TMR2IE = 0;
			CCPR1L = PR2;
			if (r < 100 || r > 900)
			{
				if (!timerActive)
				{
					timerActive = 1;	// 启动计时器
				}
			}
			else
			{	
				timerActive = 0;		// 关闭计时器
				timerCounter = 0;		// 重置计时器计数
			}
		}
		else if(v > t)
		{
			B1 = 0;
			PIE1bits.TMR2IE = 1;
		}
*/
		if (timerActive)								// 计时器判断
		{
            if((r < 100 || r > 900) && (v <= (t - 2)))
			{
                timerCounter++;							// 每次中断，增加计时器计数
            }else
			{
                timerCounter = 0;						// 重置计时器计数
				timerActive = 0;
            }

            if(timerCounter >= 5)						// 计数一定时间		
			{
				B1 = 0;								// 将B1置为0
				T2CONbits.TMR2ON = 0;				// 关闭时钟2
				PIE1bits.TMR2IE = 0;
				CCPR1L = 0;
				CCP1CON = 0;						// 关闭PWM
				TRISAbits.TRISA5 = 0;
				ADCON0bits.ADON = 0;				// 关闭 AD 转换器
				while(1)
				{
					PORTAbits.RA5 = 0;
					__delay_ms(100);
					PORTAbits.RA5 = 1;
					__delay_ms(100);
				}
            }
        }
		PIE1bits.ADIE = 1;              //一次中断执行完毕，允许中断继续发生
	}
}