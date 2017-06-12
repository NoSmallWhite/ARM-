/**
  ******************************************************************************
  * @file    USART/Printf/main.c 
  * @author  MCD Application Team
  * @version V3.3.0
  * @date    04/16/2010
  * @brief   Main program body
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32_eval.h"
#include "ili9320.h"
#include "ili9320_api.h"
#include <stdio.h>
#include "TouchScreen.h"
#include "CPUInit.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dac.h"
#include "sensor.h"
/*******************************************************************************
�жϻ���������
*******************************************************************************/
#define USART2_TX_BUFFER_SIZE 50   //����ش�С
u8 USART2_Tx_Buffer[USART2_TX_BUFFER_SIZE+1];//���建���
u8 USART2_Tx_Rd_Index=0;//����ض�ȡ�����ţ��жϷ��������
u8 USART2_Tx_Wr_Index=0;//�����д�������ţ�putchar�����ã����ڱ�ʶ��������
u8 USART2_Tx_Counter=0;//���������������δ�����ֽ���


float VCC;
u16 DAC_data=0;
u8 USART2_RX_Buf=0;
u32 sysTime = 0;//ϵͳʱ��


void USART2_Putchar(u8 c)
{
  while(USART2_Tx_Counter>(USART2_TX_BUFFER_SIZE));
  __disable_irq();//����ǰ�������ȼ��ᵽ��ߣ���ֹ�����жϴ��
  
  /*if(USART2_Tx_Counter>(USART2_TX_BUFFER_SIZE))
  {// ��������������ִ�����κβ�������
    __enable_irq();//���͵�ǰ�������ȼ��������ж�
    return;//��������
  }*/
 
  if(USART2_Tx_Counter)
  {//���������ڷ��ͣ�ֱ�ӽ�Ҫ���͵��ֽ�д�뻺����
    USART2_Tx_Buffer[USART2_Tx_Wr_Index]=c;  
  }
  else
  {//��ǰû�������ڷ��ͣ�ֱ�ӽ�����д�����ݼĴ�����ʹ��TC�ж�
    USART_ITConfig(USART2,USART_IT_TC,ENABLE);
    (void)(USART2->SR==0);
    USART2->DR=c;
  }
  //��������ų�����Χ������0
  if(++USART2_Tx_Wr_Index>USART2_TX_BUFFER_SIZE)
    USART2_Tx_Wr_Index=0;
  //��������+1
  USART2_Tx_Counter++;
  __enable_irq();
}

void USART2_TX_ISR(void)
{
  if(USART2_Tx_Counter)//������һ���ֽڣ���������-1
      USART2_Tx_Counter--;
  
  if(++USART2_Tx_Rd_Index>USART2_TX_BUFFER_SIZE)//��������ų�����Χ������0
      USART2_Tx_Rd_Index=0;
  
    if(USART2_Tx_Counter)
    {//��������Ҫ���ͣ����������ݼĴ���д�뻺�����е�����
      USART2->DR=USART2_Tx_Buffer[USART2_Tx_Rd_Index]; 
    }
    else
    {//���������������ݶ�������ɣ���TC�ж�
      USART_ITConfig(USART2,USART_IT_TC,DISABLE);
    }
}

/*******************************************************************************
�����жϽ���
*******************************************************************************/
void USART2_RX_ISR(void)
{
  USART2_RX_Buf = USART_ReceiveData(USART2);//�����յ����ַ���ֵ��������RXNE��־�ڴ˹����л��Զ�����
}
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

void LCD_Putchar(u8 c)
{//ÿ���ַ�Ϊ8X16��,320x240�ܹ�������ʾ600���ַ���15�У�ÿ��40���ַ���
  static u16 index=0;
  
  if(index>=600)
  {//һ����ʾ�����������¿�ʼ��ʾ
    index = 0;
    ili9320_Clear(Blue);
  }
  if(c=='\r')
  {//�س���
    index = 40*((index/40)+1);
  }
  else
  {
    ili9320_PutChar(8*(index%40),16*(index/40),c,Red,Blue);//�������ż���Ҫ��ʾ��λ��
    index++;
  }
  
}
void delay(u32 time)
{
  u32 next = sysTime+time-1;
  while((sysTime-next)&0x80000000);
}

void delay_us(u16 time)
{
  u16 next = TIM2->CNT+time-1;
  while((TIM2->CNT-next)&0x8000);
}

int main(void)
{
  s16 T;
  u32 P;
  float tmpr,humidity,lux,rain;
  u8 *p,checksum,i;
  u32 delaytime;
  /* ��������ʼ�� */
  CPU_Init();
  /* ��������ʼ�� */
  //ADS7843_Init();
  /* TFT LCD��ʼ�� */
  ili9320_Initializtion();
  
  ili9320_Clear(Blue);

  BMP085_ReadCalCoeff();
  SHT1x_ConnectionReset();
  
  delaytime = sysTime + 999;
  /* Main loop */
  while (1) 
  {
      if((delaytime-sysTime)&0x80000000)
      {
        delaytime = sysTime+999;
        
        T = BMP085_ReadTemp();
        P = BMP085_ReadPressure();
        ili9320_PutChar(0,0,'B',Red,Blue);
        ili9320_PutChar(8,0,'M',Red,Blue);
        ili9320_PutChar(16,0,'P',Red,Blue);
        ili9320_PutChar(24,0,'0',Red,Blue);
        ili9320_PutChar(32,0,'8',Red,Blue);
        ili9320_PutChar(40,0,'5',Red,Blue);
        ili9320_PutChar(48,0,'-',Red,Blue);
        ili9320_PutChar(56,0,'-',Red,Blue);
        ili9320_PutChar(64,0,'T',Red,Blue);
        ili9320_PutChar(72,0,':',Red,Blue);
        ili9320_PutChar(80,0,'0'+T/100,Red,Blue);
        ili9320_PutChar(88,0,'0'+(T%100)/10,Red,Blue);
        ili9320_PutChar(96,0,'.',Red,Blue);
        ili9320_PutChar(104,0,'0'+(T%10),Red,Blue);
        ili9320_PutChar(112,0,'-',Red,Blue);
        ili9320_PutChar(120,0,'-',Red,Blue);
        ili9320_PutChar(128,0,'P',Red,Blue);
        ili9320_PutChar(136,0,':',Red,Blue);
        ili9320_PutChar(144,0,'0'+P/100000,Red,Blue);
        ili9320_PutChar(152,0,'0'+(P%100000)/10000,Red,Blue);
        ili9320_PutChar(160,0,'0'+(P%10000)/1000,Red,Blue);
        ili9320_PutChar(168,0,'0'+(P%1000)/100,Red,Blue);
        ili9320_PutChar(176,0,'0'+(P%100)/10,Red,Blue);
        ili9320_PutChar(184,0,'0'+(P%10),Red,Blue);
        
        //��������ʾ��ʪ��
        if(SHT1x_ReadTempHumi(&tmpr,&humidity))
        {
          SHT1x_ConnectionReset();
          ili9320_PutChar(184,20,'-',Red,Blue);
          ili9320_PutChar(192,20,'-',Red,Blue);
          ili9320_PutChar(200,20,'E',Red,Blue);
          ili9320_PutChar(208,20,'R',Red,Blue);
          ili9320_PutChar(216,20,'R',Red,Blue);
        }
        else
        {
          ili9320_PutChar(0,20,'S',Red,Blue);
          ili9320_PutChar(8,20,'H',Red,Blue);
          ili9320_PutChar(16,20,'T',Red,Blue);
          ili9320_PutChar(24,20,'1',Red,Blue);
          ili9320_PutChar(32,20,'x',Red,Blue);
          ili9320_PutChar(40,20,'-',Red,Blue);
          ili9320_PutChar(48,20,'-',Red,Blue);
          ili9320_PutChar(56,20,'T',Red,Blue);
          ili9320_PutChar(64,20,':',Red,Blue);
          ili9320_PutChar(72,20,'0'+(u8)tmpr/10,Red,Blue);
          ili9320_PutChar(80,20,'0'+(u8)tmpr%10,Red,Blue);
          ili9320_PutChar(88,20,'.',Red,Blue);
          ili9320_PutChar(96,20,'0'+((u16)(10*tmpr))%10,Red,Blue);
          ili9320_PutChar(104,20,'-',Red,Blue);
          ili9320_PutChar(112,20,'-',Red,Blue);
          ili9320_PutChar(120,20,'H',Red,Blue);
          ili9320_PutChar(128,20,'R',Red,Blue);
          ili9320_PutChar(136,20,':',Red,Blue);
          ili9320_PutChar(144,20,'0'+(u8)humidity/10,Red,Blue);
          ili9320_PutChar(152,20,'0'+(u8)humidity%10,Red,Blue);
          ili9320_PutChar(160,20,'.',Red,Blue);
          ili9320_PutChar(168,20,'0'+((u16)(10*humidity))%10,Red,Blue);
          ili9320_PutChar(176,20,'%',Red,Blue);
        }
        //��������ʾ����
        lux = GL5528_ReadLight();
        ili9320_PutChar(0,40,'L',Red,Blue);
        ili9320_PutChar(8,40,'i',Red,Blue);
        ili9320_PutChar(16,40,'g',Red,Blue);
        ili9320_PutChar(24,40,'h',Red,Blue);
        ili9320_PutChar(32,40,'t',Red,Blue);
        ili9320_PutChar(40,40,'-',Red,Blue);
        ili9320_PutChar(48,40,'-',Red,Blue);
        ili9320_PutChar(56,40,'0'+(u16)lux/100,Red,Blue);
        ili9320_PutChar(64,40,'0'+((u16)lux%100)/10,Red,Blue);
        ili9320_PutChar(72,40,'0'+(u16)lux%10,Red,Blue);
        ili9320_PutChar(80,40,'.',Red,Blue);
        ili9320_PutChar(88,40,'0'+((u16)(10*lux))%10,Red,Blue);
        ili9320_PutChar(96,40,'l',Red,Blue);
        ili9320_PutChar(104,40,'u',Red,Blue);
        ili9320_PutChar(112,40,'x',Red,Blue);
        
        //��������ʾ����
        rain = Rain_Read();
        ili9320_PutChar(0,60,'R',Red,Blue);
        ili9320_PutChar(8,60,'a',Red,Blue);
        ili9320_PutChar(16,60,'i',Red,Blue);
        ili9320_PutChar(24,60,'n',Red,Blue);
        ili9320_PutChar(32,60,'-',Red,Blue);
        ili9320_PutChar(40,60,'-',Red,Blue);
        ili9320_PutChar(48,60,'0'+(u8)rain/10,Red,Blue);
        ili9320_PutChar(56,60,'0'+(u8)rain%10,Red,Blue);
        ili9320_PutChar(64,60,'.',Red,Blue);
        ili9320_PutChar(72,60,'0'+((u16)(10*rain))%10,Red,Blue);
        ili9320_PutChar(80,60,'%',Red,Blue);
        
        
        USART2_Putchar(0xB5);
        USART2_Putchar(0x62);
        USART2_Putchar(22);
        checksum = 22;
        
        p = (u8 *)(&T);
        for(i=0;i<2;i++)
        {
          USART2_Putchar(*(p+i));
          checksum ^= *(p+i);
        }
        
        p = (u8 *)(&P);
        for(i=0;i<4;i++)
        {
          USART2_Putchar(*(p+i));
          checksum ^= *(p+i);
        }
        
        p = (u8 *)(&tmpr);
        for(i=0;i<4;i++)
        {
          USART2_Putchar(*(p+i));
          checksum ^= *(p+i);
        }

        p = (u8 *)(&humidity);
        for(i=0;i<4;i++)
        {
          USART2_Putchar(*(p+i));
          checksum ^= *(p+i);
        }
        
        p = (u8 *)(&lux);
        for(i=0;i<4;i++)
        {
          USART2_Putchar(*(p+i));
          checksum ^= *(p+i);
        }
        
        p = (u8 *)(&rain);
        for(i=0;i<4;i++)
        {
          USART2_Putchar(*(p+i));
          checksum ^= *(p+i);
        }
        USART2_Putchar(checksum);
        
      }
  }
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
