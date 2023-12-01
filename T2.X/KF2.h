#ifndef KF2_H
#define	KF2_H

// CONFIG1
#pragma config FOSC = INTOSC    // ָ��ʹ���ڲ����� (INTOSC) ��Ϊʱ��Դ���������ⲿ����
#pragma config WDTE = OFF       // ���ÿ��Ź���ʱ�������ⲻ��Ҫ�ĸ�λ��
#pragma config PWRTE = OFF      // �����ϵ綨ʱ������ȷ�����ϵ�ʱ�����ж�����ӳ١�
#pragma config MCLRE = ON       // �� MCLR/VPP ��������Ϊ MCLR ���ܣ�������λ���š�
#pragma config CP = OFF         // ���ô����ڴ汣���������ڳ�������ʱ�޸ĳ���洢����
#pragma config CPD = OFF        // ���������ڴ汣���������ڳ�������ʱ�޸����ݴ洢����
#pragma config BOREN = ON       // ���õ��縴λ���ܣ��������Դ����������ĸ�λ��
#pragma config CLKOUTEN = OFF   // ����ʱ��������ܣ�ȷ��ʱ��������Ų���������Ŀ�ġ�
#pragma config IESO = OFF       // �����ڲ�/�ⲿ�л����ܣ�����������ʱ���ڲ�ʱ���л����ⲿʱ�ӡ�
#pragma config FCMEN = ON       // ʧЧ��ȫʱ�Ӽ��ʹ�ܣ�ʧЧ��ȫʱ�Ӽ�������ã�

// CONFIG2
#pragma config WRT = OFF        // ���� Flash �洢����д�����������ڳ�������ʱ�� Flash �洢������д�롣
#pragma config PLLEN = ON      // ���� 4x PLL����ʱ��Ƶ����ߵ� 4 ����
#pragma config STVREN = ON      // ���ö�ջ���/���縴λ���ܣ�����ջ���������ʱ�����¸�λ��
#pragma config BORV = LO        // ���õ͵�ѹ�����㣬����̽���Դ�ĵ͵�ѹ�����
#pragma config DEBUG = OFF      // �������ߵ���ģʽ��ICSPCLK �� ICSPDAT ���Ž���Ϊͨ�� I/O ���š�
#pragma config LVP = OFF        // ���õ͵�ѹ��̣���֧�ֵ͵�ѹ���ģʽ��

#define _XTAL_FREQ 32000000

#define B1      PORTAbits.RA4       //������
#define T_B1    TRISAbits.TRISA4

void setup(void);
void PWMinit(void);
#endif	/* KF1_H */

