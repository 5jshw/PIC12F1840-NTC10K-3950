#include <xc.h>
//#include <pic12f1840.h>
#include "KF2.h"

unsigned int getADCValue(unsigned char channel);        //����ADת��
unsigned int getADS(unsigned char ssa);                 //���ADת��
void __interrupt() ISR(void);                           //�жϴ�����
int t, v, y = 0;
unsigned int r;
unsigned long Rt;                                       //VR ��ǰ��ѹֵ
unsigned int timerCounter = 0;  // ��ʱ��������
char timerActive = 0;  // ��־λ����ʾ��ʱ���Ƿ񼤻�

unsigned int const TABLE[] = {9712, 9166, 8654, 8172, 7722, 7298, 6900, 6526, 6176, 5534, 5242, 4966, 4708, 4464, 4234, 4016, 3812, 3620, 3438, 3266, 
                              3104, 2950, 2806, 2668, 2540, 2418, 2302, 2192, 2088, 1990, 1897, 1809, 1726, 1646, 1571, 1500, 1432, 1368, 1307, 1249, 
                              1194, 1142, 1092, 1045, 1000, 957, 916, 877, 840, 805, 772, 740, 709, 680, 653, 626, 601, 577, 554, 532, 511, 491, 472, 
                              454, 436, 420, 404, 388, 374, 360, 346, 334, 321, 309, 298, 287, 277, 267, 258, 248
                             };  //   -20 ~ 60 �¶ȶ�Ӧ����ֵ��Ϊ���Ż��ռ䣬����ֵ��Сһλ�� 0 ~ 80

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
unsigned int ad2 = 0, ad3 = 0;      //ad2  ��λ��, ad3 ģ�⻥��������

void main(void) 
{
	timerActive = 0;		// �رռ�ʱ��
    timerCounter = 0;		// ���ü�ʱ������
    char add = 1;			//��λ���Ĳ���Ƶ��
    setup();				//ADת����ʼ���������ã�ԭ����λ��KF1.c
    PWMinit();				//PWM��ʼ���������ã�ԭ����λ��KF1.c
	CCPR1L = 0;
    __delay_ms(1200);		//������
	TRISAbits.TRISA5 = 0;   //�������
	T2CONbits.TMR2ON = 1;   //����ʱ��2
	
    while (1)				//��ѭ��
    {
        if(add == 1)						//����·ADת����Ƶ��
        {
            unsigned long VR;
            ad1 = getADCValue(0x00);		//AD�ӿڸ�Ӧ��ADֵ
            ad1 = 1024 - ad1;				//����NTCλ�ڲ��µ�·�¶ˣ������¶�����ʱ����ѹ�½���������������ˣ���ADֵ������
            VR = ad1 * 500 / 1024;			//ת���ɵ�ѹֵ
            Rt = (unsigned long)(500 - VR) * 1000 / VR;		//����NTC��ǰ�Ķ�̬����ֵ
            add++;
        }
        else if(add == 2)					//��λ���趨�¶�ֵ
        {
            ad2 = getADCValue(0x01);		//0000 1001
            t = (int)(ad2 * 10 / 128);		//1024 * 10 / 128 = 80
            add++;
        }
        else
        {
            ad3 = getADCValue(0x02);		//ģ�⻥����
            r = ad3;
            add = 1;						//ADʶ��ָʾ��
        }
		
		if(v <= (t - 2))					//�¶��ж�
		{
			B1 = 1;
			y = 1;
			if (r < 100 || r > 900)			//�������ж�
			{
				timerActive = 1;			// ������ʱ��
			}
			else
			{
				timerActive = 0;			// �رռ�ʱ��
				timerCounter = 0;			// ���ü�ʱ������
			}
		}
		else if(v > t)
		{
			B1 = 0;
			y = 0;
			timerActive = 0;				// �رռ�ʱ��
			timerCounter = 0;				// ���ü�ʱ������
		}
		__delay_ms(200);
    }
}

//����ADת������
unsigned int getADCValue(unsigned char channel)
{
    ADCON0bits.CHS = channel;                               //ѡ��ADͨ��
    __delay_ms(5);                                          //�ı�ADͨ��������ʱ�ȶ�
    ADCON0bits.GO = 1;                                      //��ʼADת��
    while (ADCON0bits.GO);                                  //ת�����ָʾ
    return (unsigned int)((ADRESH << 2) | (ADRESL >> 6));   //���ظߵ�λ�ϲ����ADֵ
}

void __interrupt() ISR(void)            //�жϴ�����
{
	if(PIR1bits.TMR2IF == 1)			//���ʱ��2�Ƿ�����ж�
	{
		char p;							//p PWM����ָʾ��
		PIE1bits.TMR2IE = 0;			//��ֹʱ��2����ж�
		T2CONbits.TMR2ON = 0;			//ֹͣ����
		PIR1bits.TMR2IF = 0; 			//ʱ��2�����־����
		if(y == 0)
		{
			if(CCPR1L >= PR2)				//��������Ƿ񳬹��������ֵ
			{
				p = 0;
			}
			else if(CCPR1L <= 0)
			{
				p = 1;
			}
			if(p == 1)                      //�ж�PWM����ָʾ��״̬
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
		PIE1bits.TMR2IE = 1;			//����ʱ��2����ж�
		T2CONbits.TMR2ON = 1;           //����ʱ��2
	}
    
	if(PIR1bits.ADIF == 1)              //���AD�Ƿ����ж�
	{
		PIE1bits.ADIE = 0;              //��ֹ�жϷ���
		PIR1bits.ADIF = 0;              //����жϱ�־
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
					timerActive = 1;	// ������ʱ��
				}
			}
			else
			{	
				timerActive = 0;		// �رռ�ʱ��
				timerCounter = 0;		// ���ü�ʱ������
			}
		}
		else if(v > t)
		{
			B1 = 0;
			PIE1bits.TMR2IE = 1;
		}
*/
		if (timerActive)								// ��ʱ���ж�
		{
            if((r < 100 || r > 900) && (v <= (t - 2)))
			{
                timerCounter++;							// ÿ���жϣ����Ӽ�ʱ������
            }else
			{
                timerCounter = 0;						// ���ü�ʱ������
				timerActive = 0;
            }

            if(timerCounter >= 5)						// ����һ��ʱ��		
			{
				B1 = 0;								// ��B1��Ϊ0
				T2CONbits.TMR2ON = 0;				// �ر�ʱ��2
				PIE1bits.TMR2IE = 0;
				CCPR1L = 0;
				CCP1CON = 0;						// �ر�PWM
				TRISAbits.TRISA5 = 0;
				ADCON0bits.ADON = 0;				// �ر� AD ת����
				while(1)
				{
					PORTAbits.RA5 = 0;
					__delay_ms(100);
					PORTAbits.RA5 = 1;
					__delay_ms(100);
				}
            }
        }
		PIE1bits.ADIE = 1;              //һ���ж�ִ����ϣ������жϼ�������
	}
}