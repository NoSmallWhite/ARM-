#include "stm32f10x.h"
#include <math.h>
void delay(u32 time);
void delay_us(u32 time);
extern u32 sysTime;
extern float VCC;

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
while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)); // �ȴ�I2C����
/* ���� START �ź� */
I2C_GenerateSTART(I2C1, ENABLE);
/* �ȴ�������ģʽ */
while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
/* ����BMP085���豸��ַ */
I2C_Send7bitAddress(I2C1, BMP085_ADDR, I2C_Direction_Transmitter);
/* �ȴ�������������ģʽ */
while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
/* ���� BMP085 ����ַ */
I2C_SendData(I2C1, ReadAddr);
/* �ȴ����ݴ������ */
while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
/* ����START */
I2C_GenerateSTART(I2C1, ENABLE);
/* �ȴ�������ģʽ */
while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
/* ����EEPROM���豸�غ�i������ */
I2C_Send7bitAddress(I2C1, BMP085_ADDR, I2C_Direction_Receiver);
/* �ȴ��������豸����ģʽ */
while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
/* While there is data to be read */
while(NumByteToRead)
{
if(NumByteToRead == 1)
{
/* NACK�ź� */
I2C_AcknowledgeConfig(I2C1, DISABLE);
/* ����STOPֹͣ�ź� */
I2C_GenerateSTOP(I2C1, ENABLE);
}
/* ���յ����豸���������� */
if(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))
{
/* �������� */
*pBuffer = I2C_ReceiveData(I2C1);
/* ά��ָ�� */
pBuffer++;
/* Ҫ��ȡ����1 */
NumByteToRead--;
}
}
/* ʹ��ACK */
I2C_AcknowledgeConfig(I2C1, ENABLE);
}
//��ȡУ��ϵ��
void BMP085_ReadCalCoeff(void)
{
u8 buffer[22];
BMP085_BufferRead(buffer,0xAA,22);
BMP085_CaliCoeff.AC1 = (s16)((buffer[0]<<8)|buffer[1]);
BMP085_CaliCoeff.AC2 = (s16)((buffer[2]<<8)|buffer[3]);
BMP085_CaliCoeff.AC3 = (s16)((buffer[4]<<8)|buffer[5]);
BMP085_CaliCoeff.AC4 = ((buffer[6]<<8)|buffer[7]);
BMP085_CaliCoeff.AC5 = ((buffer[8]<<8)|buffer[9]);
BMP085_CaliCoeff.AC6 = ((buffer[10]<<8)|buffer[11]);
BMP085_CaliCoeff.B1 = (s16)((buffer[12]<<8)|buffer[13]);
BMP085_CaliCoeff.B2 = (s16)((buffer[14]<<8)|buffer[15]);
BMP085_CaliCoeff.MB = (s16)((buffer[16]<<8)|buffer[17]);
BMP085_CaliCoeff.MC = (s16)((buffer[18]<<8)|buffer[19]);
BMP085_CaliCoeff.MD = (s16)((buffer[20]<<8)|buffer[21]);
}
//��ȡ�¶�,����ֵ��ȷ��0.1��
u16 BMP085_ReadTemp(void)
{
u8 buffer[2];
s32 X1,X2,T;
while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)); // �ȴ�I2C����
/* ���� START �ź� */
I2C_GenerateSTART(I2C1, ENABLE);
/* �ȴ�������ģʽ */
while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
/* ����BMP085���豸��ַ */
I2C_Send7bitAddress(I2C1, BMP085_ADDR, I2C_Direction_Transmitter);
/* �ȴ�������������ģʽ */
while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
/* ���� BMP085 ����ַ */
I2C_SendData(I2C1, 0xF4);
/* �ȴ����ݴ������ */
while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
/* ���� BMP085 ����ַ */
I2C_SendData(I2C1, 0x2E);
/* �ȴ����ݴ������ */
while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
/* ����START */
I2C_GenerateSTOP(I2C1, ENABLE);
//�ȴ�5ms
delay(5);
//��ȡת�����
BMP085_BufferRead(buffer,0xF6,2);
X1 = ((((buffer[0]<<8)|buffer[1])-BMP085_CaliCoeff.AC6)*BMP085_CaliCoeff.AC5)>>15;
X2 = (BMP085_CaliCoeff.MC<<11)/(X1+BMP085_CaliCoeff.MD);
B5 = X1+X2;
T = (B5+8)>>4;
return (s16)T;
}
//��ȡ��ѹ
u32 BMP085_ReadPressure(void)
{
u8 buffer[3];
s32 UP,B3,B6,X1,X2,X3,P;
u32 B4,B7;
u8 oss=3;
while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)); // �ȴ�I2C����
/* ���� START �ź� */
I2C_GenerateSTART(I2C1, ENABLE);
/* �ȴ�������ģʽ */
while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
/* ����BMP085���豸��ַ */
I2C_Send7bitAddress(I2C1, BMP085_ADDR, I2C_Direction_Transmitter);
/* �ȴ�������������ģʽ */
while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
/* ���� BMP085 ����ַ */
I2C_SendData(I2C1, 0xF4);
/* �ȴ����ݴ������ */
while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
/* ���� BMP085 ����ַ */
I2C_SendData(I2C1, 0xF4);
/* �ȴ����ݴ������ */
while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
/* ����START */
I2C_GenerateSTOP(I2C1, ENABLE);
//�ȴ�26ms
delay(26);
//��ȡת�����
BMP085_BufferRead(buffer,0xF6,3);
UP = (((buffer[0]<<16)|(buffer[1]<<8))|buffer[2])>>(8-oss);
B6 = B5-4000;
X1 = (BMP085_CaliCoeff.B2*((B6*B6)>>12))>>11;
X2 = (BMP085_CaliCoeff.AC2*B6)>>11;
X3 = X1+X2;
B3 = (((BMP085_CaliCoeff.AC1*4+X3)<<oss)+2)/4;
X1 = (BMP085_CaliCoeff.AC3*B6)>>13;
X2 = (BMP085_CaliCoeff.B1*((B6*B6)>>12))>>16;
X3 = (X1+X2+2)/4;
B4 = (BMP085_CaliCoeff.AC4*(X3+32768))>>15;
B7 = (UP-B3)*(50000>>oss);
if(B7<0x80000000)
P = 2*B7/B4;
else
P = (B7/B4)*2;
X1 = (P>>8)*(P>>8);
X1 = (X1*3038)>>16;
X2 = (-7357*P)>>16;
P = P+((X1+X2+3791)>>4);
return P;
}

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
SHT1x_DataOut();
for(i=0x80;i>0;i/=2)
{
if(i&value)
SHT1x_DATA1;
else
SHT1x_DATA0;
delay_us(2);
SHT1x_SCK1;
delay_us(5);
SHT1x_SCK0;
}
SHT1x_DATA0;
SHT1x_DataIn();
SHT1x_SCK1;
delay_us(2);
error = SHT1x_DATA_IN;
delay_us(2);
SHT1x_SCK0;
return error;
}
//�ֽڶ���,ԭ�����SHT1x�����ֲ��3�½�
u8 SHT1x_ReadByte(u8 ack)
{
u8 i,val=0;
SHT1x_DataIn();
for(i=0x80;i>0;i/=2)
{
SHT1x_SCK1;
delay_us(2);
if(SHT1x_DATA_IN)
val = (val|i);
SHT1x_SCK0;
delay_us(2);
}
SHT1x_DataOut();
if(ack)
SHT1x_DATA0;
else
SHT1x_DATA1;
SHT1x_SCK1;
delay_us(5);
SHT1x_SCK0;
SHT1x_DataIn();
return val;
}
//��ʼ�ź�,ԭ�����SHT1x�����ֲ��3.2�½�
void SHT1x_TransStart(void)
{
SHT1x_DataOut();
SHT1x_DATA1;
SHT1x_SCK0;
delay_us(2);
SHT1x_SCK1;
delay_us(2);
SHT1x_DATA0;
delay_us(2);
SHT1x_SCK0;
delay_us(5);
SHT1x_SCK1;
delay_us(2);
SHT1x_DATA1;
delay_us(2);
SHT1x_SCK0;
SHT1x_DataIn();
}
//���Ӹ�λ�ź�,ԭ�����SHT1x�����ֲ��3.4�½�
void SHT1x_ConnectionReset(void)
{
u8 i;
SHT1x_DataOut();
SHT1x_DATA1;
SHT1x_SCK0;
for(i=0;i<9;i++)
{
SHT1x_SCK1;
delay_us(1);
SHT1x_SCK0;
delay_us(1);
}
SHT1x_TransStart();
SHT1x_DataIn();
}
//��״̬�Ĵ�����,ԭ�����SHT1x�����ֲ��3.6�½�
u8 SHT1x_ReadReg(u8 *pvalue)
{
u8 error=0,crc;
SHT1x_TransStart();
error = SHT1x_WriteByte(0x7);
*pvalue = SHT1x_ReadByte(1);
crc = SHT1x_ReadByte(0);
return error;
}
//д״̬�Ĵ���,ԭ�����SHT1x�����ֲ��3.6�½�
u8 SHT1x_WriteReg(u8 value)
{
u8 error=0;
SHT1x_TransStart();
error += SHT1x_WriteByte(0x6);
error += SHT1x_WriteByte(value);
return error;
}
//������ʪ��,ԭ�����SHT1x�����ֲ��3.6�½�
u8 SHT1x_Measure(u16 *pvalue, u8 mode)
{
u8 error=0,crc;
u32 delaytime;
SHT1x_TransStart();
switch(mode)
{
case 0: error += SHT1x_WriteByte(0x3);break;//Ϊʲô��0x3?�μ������ֲ��4;
case 1: error += SHT1x_WriteByte(0x5);break;
default:break;
}
SHT1x_DataIn();
delaytime = sysTime + 2000;
while(1)
{
if(SHT1x_DATA_IN==0) break;
if((delaytime-sysTime)&0x80000000) break;
}
if(SHT1x_DATA_IN) error +=1;
if(error)
return error;
*pvalue = (SHT1x_ReadByte(1)<<8)|SHT1x_ReadByte(1);
crc = SHT1x_ReadByte(0);
return error;
}
//��ȡ��ʪ�ȣ�ԭ�����SHT1x�����ֲ��4�½�
u8 SHT1x_ReadTempHumi(float *ptemp,float *phumi)
{
u8 error = 0;
u16 result;
//�¶Ȳ���
error += SHT1x_Measure(&result,0);
*ptemp = 0.01*result-39.66;
//ʪ�Ȳ���
error += SHT1x_Measure(&result,1);
*phumi = -2.0468 + 0.0367*result - 0.0000015955*result*result;
*phumi += (*ptemp-25)*(0.01+0.00008*result);
return error;
}

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
value = Read_ADC(ADC_Channel_8);
V = VCC*value/4095.0;
R = 10*VCC/V - 10;
return (10*pow(10.0,log10(15/R)/0.6));
}

u16 Rain_Read(void)
{
u16 value;
float R,V;
return (Read_ADC(ADC_Channel_4)/40.95);
}