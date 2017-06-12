#include "stm32f10x.h"
extern float VCC;
void RCC_Configuration(void);
void GPIO_Configuration(void);
void EXTI_Configuration(void);
void NVIC_Configuration(void);
void ADC_Configuration(void);
void DAC_Configuration(void);
void USART_Configuration(void);
void SysTick_Configuration(void);
void I2C_Configuration(void);
void TIM_Configuration(void);
void CPU_Init(void)
{
  RCC_Configuration();
  GPIO_Configuration();
  EXTI_Configuration();
  ADC_Configuration();
  USART_Configuration();
  I2C_Configuration();
  TIM_Configuration();
  NVIC_Configuration();
  SysTick_Configuration();
}

void RCC_Configuration(void)
{
  ErrorStatus HSEStartUpStatus;
  
  /*****************************************************************************
  1��������RCC����Ĵ�������ΪĬ��ֵ
  *****************************************************************************/
  RCC_DeInit();
  
  /*****************************************************************************
  2�������ⲿ���پ���
  *****************************************************************************/
  RCC_HSEConfig(RCC_HSE_ON);

  /*****************************************************************************
  3���ȴ�ֱ���ⲿ���پ����ȶ�
  *****************************************************************************/
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if (HSEStartUpStatus == SUCCESS)
  {
    /*****************************************************************************
    4������Ԥȡָ����ʹ�ܺʹ�����ʱֵ
    *****************************************************************************/
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);
 
    /*****************************************************************************
    5������AHBʱ��(HCLK)Ϊϵͳʱ��(SYSCLK)
    *****************************************************************************/
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
    /*****************************************************************************
    6������APB2ʱ��(PCLK2)Ϊϵͳʱ��(SYSCLK)
    *****************************************************************************/
    /* PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1); 
    
    /*****************************************************************************
    7������APB1ʱ��(HCLK)Ϊϵͳʱ��/2(SYSCLK/2)
    *****************************************************************************/
    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div2);
    
    /*****************************************************************************
    ����ADCʱ��(ADCLK)Ϊϵͳʱ��/6(SYSCLK/6)=12MHz
    *****************************************************************************/
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); 
    
    /*****************************************************************************
    8������PLL2��Ƶϵ����PLL2CLK = (HSE / 5) * 8 = 40 MHz
    *****************************************************************************/
    RCC_PREDIV2Config(RCC_PREDIV2_Div5);//HSE/5
    RCC_PLL2Config(RCC_PLL2Mul_8);//(HSE/5)*8

    /*****************************************************************************
    9��ʹ��PLL2���ȴ�PLL2����
    *****************************************************************************/
    RCC_PLL2Cmd(ENABLE);//ʹ��PLL2
    while (RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET);//�ȴ�PLL2����

    /*****************************************************************************
    10������PLLʱ��Դ�뱶Ƶϵ����PLLCLK = (PLL2CLK / 5) * 9 = 72 MHz
    *****************************************************************************/
    RCC_PREDIV1Config(RCC_PREDIV1_Source_PLL2, RCC_PREDIV1_Div5);//PLL2CLK/5
    RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_9);//(PLL2CLK / 5) * 9


    /*****************************************************************************
    11��ʹ��PLL���ȴ�PLL����
    *****************************************************************************/
    RCC_PLLCmd(ENABLE);//ʹ��PLL
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);//�ȴ�PLL����

    /*****************************************************************************
    12������PLLΪϵͳʱ��Դ���ȴ������
    *****************************************************************************/
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);//����PLLΪʱ��Դ
    while(RCC_GetSYSCLKSource() != 0x08);//�ȴ�ϵͳʱ�Ӿ���
  }
  else
  { //HSE����ʧ�ܣ�������ѭ��
    while (1);
  }
  
  /*****************************************************************************
    13����������ʱ��
  *****************************************************************************/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2|RCC_APB1Periph_I2C1|RCC_APB1Periph_USART2,ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_ADC2|RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD,ENABLE);
}

void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_DeInit(GPIOA);
  GPIO_DeInit(GPIOB);
  GPIO_DeInit(GPIOC);
  GPIO_DeInit(GPIOD);
  GPIO_DeInit(GPIOE);
  
  //USART2�˿���ӳ��
  GPIO_PinRemapConfig(GPIO_Remap_USART2,ENABLE);
  
  //��ʼ����������Ӧ�Ĺܽ�Ϊ��©����������ܽ��ø�
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
  GPIO_SetBits(GPIOA,GPIO_Pin_3);
  
  //��ʼ���ĸ�LED��Ӧ�Ĺܽ�Ϊ��������������ܽ��ø�  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOD,&GPIO_InitStructure);
  GPIO_SetBits(GPIOD,GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_7);
  
  //��4��������Ӧ��IO����ɸ�������ģʽ
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOB,&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_13;
  GPIO_Init(GPIOC,&GPIO_InitStructure);
  
  //��4��������Ӧ��IO����Ϊ�ⲿ�ж�����ܽ�
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource10);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource4);
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource13);
   
  /*****************************************************************************
  ADC����ܽ����� PA0���������У���PB0����������������
  *****************************************************************************/
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /*****************************************************************************
  USART�ܽ�����
  *****************************************************************************/
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;//PD5���AFPP���ڷ���
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;//PD6���IN_FLOATING���ڽ���
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  //I2C1�ܽų�ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB,&GPIO_InitStructure);
  
  //SHT1x SCK�ܽų�ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC,&GPIO_InitStructure);
  
  //SHT1x DATA�ܽų�ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA,&GPIO_InitStructure);
}


void EXTI_Configuration(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
  
  EXTI_DeInit();
  //��ʼ���ⲿ�ж�Ϊ�½��ش���
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line4;
  EXTI_Init(&EXTI_InitStructure);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line10;
  EXTI_Init(&EXTI_InitStructure);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line13;
  EXTI_Init(&EXTI_InitStructure);
}


void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
  //�������ȼ����飬��λ��ռ���ȼ�����λ��Ӧ���ȼ�
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  //���ð��������ж�
  //KEY4
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  //KEY1
  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  //KEY2��KEY3
  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  //USART2
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void ADC_Configuration(void)
{
  ADC_InitTypeDef ADC_InitStructure;  
  u8 i;
  u32 result;
  //��ADC1�ļĴ�����λ��Ĭ��ֵ
  ADC_DeInit(ADC1);
  
  //��ʼ��ADC
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//��������ģʽ
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;//ɨ��ģʽ��ʹ��
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//��������ģʽ��ʹ��
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//û���ⲿ����Դ
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//ת������Ҷ���
  ADC_InitStructure.ADC_NbrOfChannel = 1;//����1��ͨ��Ҫת��
  ADC_Init(ADC1, &ADC_InitStructure);

  //���¶ȴ�����/�ڲ���ѹ�ο�
  ADC_TempSensorVrefintCmd(ENABLE);

  //��ADC
  ADC_Cmd(ADC1, ENABLE);
  
  //����ADC1У׼�Ĵ���
  ADC_ResetCalibration(ADC1);
  //�ȴ�ADC1У׼�Ĵ����������
  while(ADC_GetResetCalibrationStatus(ADC1));

  //��ʼУ׼ADC1
  ADC_StartCalibration(ADC1);
  //�ȴ�ADC1У׼���
  while(ADC_GetCalibrationStatus(ADC1));
  result = 0;
  ADC_RegularChannelConfig(ADC1,ADC_Channel_17,1,ADC_SampleTime_71Cycles5);
  for(i=0;i<16;i++)
  {
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)==RESET);
    result += (ADC_GetConversionValue(ADC1));
  }
  
  VCC = 1.2*16*4096/result; 
}

void USART_Configuration(void)
{
  USART_InitTypeDef USART_InitStructure;
    
  USART_InitStructure.USART_BaudRate = 9600;//������9600
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;//���ݳ���8λ
  USART_InitStructure.USART_StopBits = USART_StopBits_1;//1λֹͣλ
  USART_InitStructure.USART_Parity = USART_Parity_No;//û����żУ��
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ��������
  USART_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;//���ͺͽ���ʹ��

  
  //UART2
  USART_DeInit(USART2);//��λ
  USART_Init(USART2, &USART_InitStructure);//��ʼ��
  USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
  //ʹ��USART2
  USART_Cmd(USART2, ENABLE); 
}
/*******************************************************************************
SysTick��ʼ������
*******************************************************************************/
void SysTick_Configuration(void)
{
  /*****************************************************************************
  ʱ������Ƶ����CPUƵ�ʣ�72MHz��һ��������ʱ��ÿ��������72000000������Ҫÿ���Ӳ�
  ��n���жϣ���SysTick_Config�Ĳ���Ӧ��ΪCPUClock/n
  *****************************************************************************/
  if(SysTick_Config(72000000/1000))
    while(1);
  /*****************************************************************************
  ��SysTick�ж����ȼ���Ϊ��ߣ������������жϴ������е���delay����ʱ����SysTick
  �޷������ж϶�ʹ��delay���������˳���ע������SysTick��ռʽ���ȼ�һ������ߵ���
  �ϴ�������в��ܵ���delay����
  *****************************************************************************/
  NVIC_SetPriority(SysTick_IRQn,0x0);    
}

/*******************************************************************************
I2C1��ʼ������
*******************************************************************************/
void I2C_Configuration(void)
{
  I2C_InitTypeDef  I2C_InitStructure; 

  I2C_DeInit(I2C1);
  /* I2C configuration */
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;//I2Cģʽ
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;//ռ�ձ�
  I2C_InitStructure.I2C_OwnAddress1 = 0xA0;//�豸��ַ
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;//Ackʹ��
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;//7λ��ַģʽ
  I2C_InitStructure.I2C_ClockSpeed = 400000;//400KHz
  
  /* ʹ��I2C */
  I2C_Cmd(I2C1, ENABLE);
  /* ��ʼ��I2C */
  I2C_Init(I2C1, &I2C_InitStructure);

  /*����1�ֽ�1Ӧ��ģʽ*/
  I2C_AcknowledgeConfig(I2C1, ENABLE);   
}

void TIM_Configuration(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  
  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_Prescaler = 71;//Ԥ��ƵֵΪ72����ʱ��ʱ��Ϊ72MHz/72=1 MHz
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;//����Ƶ��ETR��TIx������ʱ������
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//���ϼ���ģʽ

  TIM_DeInit(TIM2);//��λ��ʼ��
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);//ʱ����ʼ��
  TIM_Cmd(TIM2,ENABLE);//ʹ�ܶ�ʱ��2
}