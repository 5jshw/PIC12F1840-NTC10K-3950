#include <xc.h>
#include "KF2.h"

void setup(void) 
{
	OSCCON = 0b01110000;  // 设置振荡器为8MHz

	// 配置输出信号引脚
	B1 = 0;    // 清空B1
	T_B1 = 0;  // 设置B1为输出引脚
	
	// 配置 ADC
	ANSELAbits.ANSA0 = 1;   // 设置模拟输入引脚 A0
	ANSELAbits.ANSA1 = 1;   // 设置模拟输入引脚 A1
	ANSELAbits.ANSA2 = 1;   // 设置模拟输入引脚 A2
	ADCON1 = 0x50;          // 结果左对齐，FOSC/16，参考电压为VDD
	PIE1bits.ADIE = 1;      // 允许 ADC 中断
	PIR1bits.ADIF = 0;      // 清零 ADC 中断标志
	ADCON0bits.GO = 0;      // 初始化 AD 转换
	ADCON0bits.ADON = 1;    // 开启 AD 转换器
}

void PWMinit(void)
{	
	TRISAbits.TRISA5 = 1;       //禁止输出
	APFCONbits.CCP1SEL = 1;		//设置RA5引脚的功能
	PR2 = 254;					//设定周期
	CCP1CONbits.CCP1M = 0x0F;	//ECCP1 模式选择为PWM 模式
	CCP1CONbits.P1M = 0;		//增强型 PWM 输出配置位
	INTCONbits.GIE = 1;         //开总中断
	INTCONbits.PEIE = 1;        //开外设中断
	PIE1bits.TMR2IE = 1;        //允许Timer2溢出中断
	PIR1bits.TMR2IF = 0;        //Timer2溢出标志清零
	T2CONbits.T2CKPS = 3;       //时钟2预分频值为4
    
    
}