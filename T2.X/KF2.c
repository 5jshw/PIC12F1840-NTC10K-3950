#include <xc.h>
#include "KF2.h"

void setup(void) 
{
	OSCCON = 0b01110000;  // ��������Ϊ8MHz

	// ��������ź�����
	B1 = 0;    // ���B1
	T_B1 = 0;  // ����B1Ϊ�������
	
	// ���� ADC
	ANSELAbits.ANSA0 = 1;   // ����ģ���������� A0
	ANSELAbits.ANSA1 = 1;   // ����ģ���������� A1
	ANSELAbits.ANSA2 = 1;   // ����ģ���������� A2
	ADCON1 = 0x50;          // �������룬FOSC/16���ο���ѹΪVDD
	PIE1bits.ADIE = 1;      // ���� ADC �ж�
	PIR1bits.ADIF = 0;      // ���� ADC �жϱ�־
	ADCON0bits.GO = 0;      // ��ʼ�� AD ת��
	ADCON0bits.ADON = 1;    // ���� AD ת����
}

void PWMinit(void)
{	
	TRISAbits.TRISA5 = 1;       //��ֹ���
	APFCONbits.CCP1SEL = 1;		//����RA5���ŵĹ���
	PR2 = 254;					//�趨����
	CCP1CONbits.CCP1M = 0x0F;	//ECCP1 ģʽѡ��ΪPWM ģʽ
	CCP1CONbits.P1M = 0;		//��ǿ�� PWM �������λ
	INTCONbits.GIE = 1;         //�����ж�
	INTCONbits.PEIE = 1;        //�������ж�
	PIE1bits.TMR2IE = 1;        //����Timer2����ж�
	PIR1bits.TMR2IF = 0;        //Timer2�����־����
	T2CONbits.T2CKPS = 3;       //ʱ��2Ԥ��ƵֵΪ4
    
    
}