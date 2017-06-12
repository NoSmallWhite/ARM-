#include "stm32f10x.h"
#include <math.h>
void delay(u32 time);
void delay_us(u32 time);
extern u32 sysTime;
extern float VCC;
/*******************************************************************************
BMP085 ��ѹ��������غ���
*******************************************************************************/
#define BMP085_ADDR 0xEE 
//У�������ṹ��
struct
{
  s16 AC1;
  s16 AC2;
  s16 AC3;
  u16 AC4;
  u16 AC5;
  u16 AC6;
  s16 B1;
  s16 B2;
  s16 MB;
  s16 MC;
  s16 MD;
} BMP085_CaliCoeff;
s32 B5;
//��ȡ�Ĵ�������
void BMP085_BufferRead(u8 *pBuffer,u8 ReadAddr,u8 NumByteToRead)
{
  
}
//��ȡУ��ϵ��
void BMP085_ReadCalCoeff(void)
{
  u8 buffer[22];
  
 
}
//��ȡ�¶�,����ֵ��ȷ��0.1��
u16 BMP085_ReadTemp(void)
{
  u8 buffer[2];
  s32 X1,X2,T;
  
  
  return (s16)T;
}
//��ȡ��ѹ
u32 BMP085_ReadPressure(void)
{
  u8 buffer[3];
  s32 UP,B3,B6,X1,X2,X3,P;
  u32 B4,B7;
  u8 oss=3;
  
  
  
  return P;
}

/*******************************************************************************
SHT1x ��ѹ��������غ���
*******************************************************************************/
//����Data�ܽ�Ϊ���
void SHT1x_DataOut(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
}
//����Data�ܽ�Ϊ����
void SHT1x_DataIn(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
}
#define SHT1x_DATA1 GPIO_SetBits(GPIOA,GPIO_Pin_9)
#define SHT1x_DATA0 GPIO_ResetBits(GPIOA,GPIO_Pin_9)
#define SHT1x_SCK1 GPIO_SetBits(GPIOC,GPIO_Pin_14)
#define SHT1x_SCK0 GPIO_ResetBits(GPIOC,GPIO_Pin_14)
#define SHT1x_DATA_IN GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_9)
//�ֽ�д��,ԭ�����SHT1x�����ֲ��3�½�
u8 SHT1x_WriteByte(u8 value)
{
  u8 i, error=0;
  
  
  
  return error;
}
//�ֽڶ���,ԭ�����SHT1x�����ֲ��3�½�
u8 SHT1x_ReadByte(u8 ack)
{
  u8 i,val=0;
  
   
  return val;
}
//��ʼ�ź�,ԭ�����SHT1x�����ֲ��3.2�½�
void SHT1x_TransStart(void)
{
 
}
//���Ӹ�λ�ź�,ԭ�����SHT1x�����ֲ��3.4�½�
void SHT1x_ConnectionReset(void)
{
  u8 i;
  
 
}
//��״̬�Ĵ�����,ԭ�����SHT1x�����ֲ��3.6�½�
u8 SHT1x_ReadReg(u8 *pvalue)
{
  u8 error=0,crc;
  
  
  return error;
}

//д״̬�Ĵ���,ԭ�����SHT1x�����ֲ��3.6�½�
u8 SHT1x_WriteReg(u8 value)
{
  u8 error=0;
  
  
  return error;
}
//������ʪ��,ԭ�����SHT1x�����ֲ��3.6�½�
u8 SHT1x_Measure(u16 *pvalue, u8 mode)
{
  u8 error=0,crc;
  
  u32 delaytime;
  
  
  
  return error;
}

//��ȡ��ʪ�ȣ�ԭ�����SHT1x�����ֲ��4�½�
u8 SHT1x_ReadTempHumi(float *ptemp,float *phumi)
{
  u8 error = 0;
  u16 result;

  
  return error;
}


/*******************************************************************************
GL5528 ������������غ���
*******************************************************************************/
//ADC��ȡ����
u16 Read_ADC(u8 channel)
{
  ADC_RegularChannelConfig(ADC1,channel,1,ADC_SampleTime_239Cycles5);
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)==RESET);
  return (ADC_GetConversionValue(ADC1));
}

//GL5528���������ȼ���
float GL5528_ReadLight(void)
{
  u16 value;
  float R,V;
  

}
/*******************************************************************************
������������غ���,����ٷֱ�
*******************************************************************************/
u16 Rain_Read(void)
{
  u16 value;
  float R,V;
  
}