/**
  ******************************************************************************
  * @file    app.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-July-2011
  * @brief   This file provides all the Application firmware functions.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/ 


#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "stm32f4xx.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"
#include "stm32f4_discovery.h"
#include "parsing.h"
#include "lmp91050.h"

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED

  #if defined ( __ICCARM__ ) /*!< IAR Compiler */

    #pragma data_alignment=4   

  #endif

#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

/*Global variables definition*/

__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END ;
uint8_t UserButtonPressed;

extern uint8_t BIG_BUF[];   /*In questo buffer trovo i byte che sono stati ricevuti
                              dal computer*/
extern uint8_t BIG_BUF_Ptr; /*Numero di byte ricevuti*/

/* These are external variables imported from CDC core to be used for IN 
   transfer management. */
extern uint8_t  APP_Rx_Buffer []; /* Write CDC received data in this buffer.
                                     These data will be sent over USB IN endpoint
                                     in the CDC core functions. 
                                     Qui dentro mi trovo pure i dati che mando dal device (mi sa che non e' cosi')*/
extern uint32_t APP_Rx_ptr_in;    /* Increment this pointer or roll it back to
                                     start address when writing received data
                                     in the buffer APP_Rx_Buffer. */
extern uint32_t APP_Rx_ptr_out;

extern uint32_t APP_Rx_length;

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
uint16_t PrescalerValue = 0;
static __IO uint32_t TimingDelay;
DAC_InitTypeDef  DAC_InitStructure;
uint16_t const_out = 4095;
extern  Host_comm host = {0};
Host_comm1 host1 = {0};
Host_comm host_old = {0};
Host_comm1 host1_old = {0};
int adc3_trigger = 0;
int adc2_trigger = 0;
int adc1_trigger = 0;
set_struct set_array = {0};
uint16_t adc3ch2[1000] = {0};
uint16_t adc2ch1[1000] = {0};
uint16_t adc1ch3[1000] = {0};
uint8_t adc3ch2_8bit[2000] = {0};
uint8_t adc3ch2_16bit[2000] = {0};
uint8_t adc2ch1_16bit[2000] = {0};
uint8_t adc1ch3_16bit[2000] = {0};
uint8_t average_16bit[2];
uint8_t average_16bit_all[6];
int average = 0;
uint16_t DAC2_out = 4095;
extern int freq = 0;
extern int pulse = 0;
extern int duty_cycle = 0;



/*Functions prototypes*/
void InitTIM4(void);
void SysInit(void);
void InitGPIO(void);
void LED_Config(void);
void InitADC1_CH1_CH2(void);
void SysTickInit(void);
void delay_ms(__IO uint32_t nTime);
void TimingDelay_Decrement(void);
void DAC_Ch1_Config(void);
void SetFreq(int freq);
void SetDutyC(int duty_cycle);
uint32_t ADC_measure(ADC_TypeDef* ADCx, uint8_t ADC_Channel_x);
void InitSetStruct( Host_comm* host_new, Host_comm* host_old, set_struct* set_array);
void ADC3_CH2_DMA_Config(void);
void ADC2_CH1_DMA_Config(void);
void ADC1_CH3_DMA_Config(void);
void InitTIM2(void);
void SetSamplFreq(int SamplPerSec);
int GetSamplFreq(void);
int AverageM (int n_ms, int signal_freq, int sampling_freq, int duty_cycle, uint16_t m_vector[]);
int AverMeas (int delay, int n_sampl, uint16_t m_vector[]);
void SetVoltageDAC_Ch1(int voltage);
void InitGPIO_SDWN(void);


/**
  * @brief  Program entry point
  * @param  None
  * @retval None
  */

int main(void)

{

  /*!< At this stage the microcontroller clock setting is already configured, 
  this is done through SystemInit() function which is called from startup
  file (startup_stm32fxxx_xx.s) before to branch to application main.
  To reconfigure the default setting of SystemInit() function, refer to
  system_stm32fxxx.c file
  */  

  //__IO uint32_t i = 0;
  /*Device identificator (STM-32F4)*/
  uint8_t dev_id[9] = {'#','S','T','M','-','3','2','F','4'} ;
  int i = 0;
  int int_buf_ptr = 0;
  int length;
  
  /*This vector will be used to store the incoming data from host*/
  uint8_t internal_buff[100] = {0x00};
  
  SysInit();
  
  LMP_SPI_Init();
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  InitTIM2();
  
  USBD_Init(&USB_OTG_dev,
#ifdef USE_USB_OTG_HS 
            USB_OTG_HS_CORE_ID,
#else            
            USB_OTG_FS_CORE_ID,
#endif  
            &USR_desc, 
            &USBD_CDC_cb, 
            &USR_cb);
  
  /*Initialisation for delay_ms function 
   *Must be placed after USB Init to get delay function working properly */
  SysTickInit();
  InitTIM4(); 
  InitGPIO_SDWN();
  
  
  /* ADC3 configuration *******************************************************/
  /*  - Enable peripheral clocks                                              */
  /*  - DMA2_Stream0 channel2 configuration                                   */
  /*  - Configure ADC Channel2 pin as analog input                            */
  /*  - Configure ADC3 Channel2                                               */
  ADC3_CH2_DMA_Config();
  ADC2_CH1_DMA_Config();
  ADC1_CH3_DMA_Config();
  
  /************System setting*************/
  /*Initialise current at the maximum level for the DAC*/
  host.current = 30;
  host.voltage = 0;
  
  host.freq = 1;
  host.duty_cycle = 50;
  host.sampling_freq = 1000;
  host.pulse = 0;
  
  freq = host.freq;
  pulse = host.pulse;
  duty_cycle = host.duty_cycle;
  
  host.aver_delay = 100;
  host.aver_nsampl = 30;
  
  set_array.sampl_freq = 1;
  set_array.voltage = 1;
  set_array.pulse = 1;
  set_array.freq = 1; 
  set_array.duty_cycle = 1;
  set_array.lmp_change = 1;
  
  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
  TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);
  
  while (1)
  { 
    /*Reading from host (only if input buffer is not empty)*/
    if (BIG_BUF_Ptr != 0)
    {
      /*Saving control sequence length*/
      host.length = BIG_BUF_Ptr;
      /*Immediately clear the pointer to allow consequent strings to be captured
      without using a delay in the host code. This is necessary because if we zero BIG_BUF_Ptr
      at the end of this if{} a new string could be added to the input FIFO buffer after 
      copying the buffer and before zeroing BIG_BUF_Ptr: this string(these characters) 
      would be lost */
      BIG_BUF_Ptr = 0;
      
      /* wait for EOL character before reading the buffer */
      /*There is no need to do this because at this point BIG_BUF_Ptr is always correctly updated
      by the USB part of the firmware. This variable is updated in a while loop that cannot be interrupted
      by the following code*/
      //while ( BIG_BUF[BIG_BUF_Ptr-1] != '\n' );
      
      /*saving the old command stuct to compare later whit the new one*/
      host_old = host;
      host1_old = host1;
      
      /*the internal buffer must be erased, otherwise old commands could be decoded if the 
      previous input string was longer than the previous one*/
      for (i = 0; i < 100; i++) internal_buff[i] = 0;
      
      /*copying of the input string to the internal buffer for decoding*/
      for (i = 0; i <= (host.length - 1); i++) internal_buff[i] = BIG_BUF[i];

      parser(internal_buff, &host, &host1);

      /*Comparison of the new commands with the old ones*/
      InitSetStruct( &host, &host_old, &set_array);         
    } 
    /*update the variables passed to the interrupt routines code*/
    freq = host.freq;
    pulse = host.pulse;
    duty_cycle = host.duty_cycle;
    /*****************System settings required by host*************************/
    /*system reset routine*/
    if (host.reset == 1)
    {
      host.reset = 0;
      set_array.sampl_freq = 1;
      set_array.voltage = 1;
      set_array.pulse = 1;
      set_array.freq = 1;
      set_array.duty_cycle = 1;
      set_array.lmp_change = 1;
      host.sampling_freq = 1000;
      host.voltage = 0;
      host.pulse = 0;
      host.freq = 1;
      host.duty_cycle = 50;
      host.aver_delay = 100;
      host.aver_nsampl = 30;
      host.offset = 128;
      host.gain1 = 0; 
      host.gain2 = 0;
    }
    
    
    /*Sampling frequency setting*/
    if (set_array.sampl_freq == 1)
    {
      SetSamplFreq(host.sampling_freq);
      set_array.sampl_freq = 0;
    }
       
    /*set frequency and duty cycle*/
    if ( (set_array.pulse || set_array.freq) == 1)
    {
      if (host.pulse == 1) 
      {
        if (host.freq != 0)
        {
          SetFreq(host.freq);
        }
      }
      else if (host.pulse == 0)
      {
        ADC_DMACmd(ADC1, ENABLE);
        ADC_DMACmd(ADC2, ENABLE);
        ADC_DMACmd(ADC3, ENABLE);
      }
      set_array.pulse = 0;
      set_array.freq = 0; 
    }
    if (set_array.duty_cycle == 1)
    {
      SetDutyC(host.duty_cycle);
      set_array.duty_cycle = 0;   
    }
    
    /*configuration for LMP91050 offset measurement if requested: DAC output
    at 0V*/
    if (set_array.lmp_change == 1)
    {
      LMP_SPI_send((uint8_t)(host.offset), (uint8_t)(host.gain1), (uint8_t)(host.gain2));
      set_array.lmp_change = 0;
    }
    
    if (set_array.ampli_setting == 1)
    {
      if (host.ampli_setting == 1)
      {
        Offset_Measurement_On();
      }
      else if (host.ampli_setting == 0)
      {
        Offset_Measurement_Off();
      }
      set_array.ampli_setting = 0;
    }
    
    
    /*********************************************************************/

    
    /**********************Transmission to host***************************/
    
    /*Send ADC3 converted voltage to USB, the data buffer is updated in hardware
    by the DMA*/
    
    /*Samples from memory are copied to the sending buffer only if there is 
    a measurement request and there is not a repeat request*/
    
    if (host.repeat == 0 && host.meas_request == 1)
    {  
      
    /*Conversion from 1 word samples to 2 equivalent bytes*/
    /*the first byte contains the 4 MSB of the sample, the second byte contains
    8 LSB  (12 bit)*/
    if (adc1_trigger==0 || host.pulse==0 || host.freq == 0)
    {
      for (i=0; i<2000; i++)
      {
        if (i%2 == 0) adc1ch3_16bit[i] = (adc1ch3[i/2] >> 8);
        else adc1ch3_16bit[i] = (adc1ch3[i/2]);
      }
      adc1_trigger=1;
    }
    if (adc2_trigger==0 || host.pulse==0 || host.freq == 0)
    {
      for (i=0; i<2000; i++)
      {
        if (i%2 == 0) adc2ch1_16bit[i] = (adc2ch1[i/2] >> 8);
        else adc2ch1_16bit[i] = (adc2ch1[i/2]);
      }
      adc2_trigger=1;
    }
    if (adc3_trigger==0 || host.pulse==0 || host.freq == 0)
    {
      for (i=0; i<2000; i++)
      {
        if (i%2 == 0) adc3ch2_16bit[i] = (adc3ch2[i/2] >> 8);
        else adc3ch2_16bit[i] = (adc3ch2[i/2]);
      }
      adc3_trigger=1;
    }
    adc3_trigger = 1;
    adc2_trigger = 1;
    adc1_trigger = 1; 
    }
    else host.repeat = 0;
    /*endif (repeat == 0)*/
    host.meas_request = 0;
    
    
    
    if (host.wave_req_adc1ch3 == 1)
    {
      /*the following 3 lines fix the TX buffer problem with consecutive data
      sending*/
      APP_Rx_ptr_out = 0;
      APP_Rx_length = 0;
      APP_Rx_ptr_in = 0;
      for (i=0; i < 2000; i++) VCP_DataTx (&adc1ch3_16bit[i],0);
      
      host.wave_req_adc1ch3 = 0;
      STM_EVAL_LEDToggle(LED3);
    }
    
    if (host.wave_req_adc2ch1 == 1)
    {
      /*the following 3 lines fix the TX buffer problem with consecutive data
      sending*/
      APP_Rx_ptr_out = 0;
      APP_Rx_length = 0;
      APP_Rx_ptr_in = 0;
      for (i=0; i < 2000; i++) VCP_DataTx (&adc2ch1_16bit[i],0);

      host.wave_req_adc2ch1 = 0;
      STM_EVAL_LEDToggle(LED3);
    }
    
    if (host.wave_req_adc3ch2 == 1)
    {
      /*the following 3 lines fix the TX buffer problem with consecutive data
      sending*/
      APP_Rx_ptr_out = 0;
      APP_Rx_length = 0;
      APP_Rx_ptr_in = 0;
      for (i=0; i < 2000; i++) VCP_DataTx (&adc3ch2_16bit[i],0);

      host.wave_req_adc3ch2 = 0;
      STM_EVAL_LEDToggle(LED3);
    }
    
    if (host.aver_req_adc3ch2 == 1)
    {
      average = AverMeas (host.aver_delay, host.aver_nsampl, adc3ch2);
      average_16bit[0] = average>>8;
      average_16bit[1] = average;
      APP_Rx_ptr_out = 0;
      APP_Rx_length = 0;
      APP_Rx_ptr_in = 0;

      VCP_DataTx (&average_16bit[0],0);
      VCP_DataTx (&average_16bit[1],0);
      host.aver_req_adc3ch2 = 0;
    }
    if (host.aver_req_adc2ch1 == 1)
    {
      average = AverMeas (host.aver_delay, host.aver_nsampl, adc2ch1);
      average_16bit[0] = average>>8;
      average_16bit[1] = average;
      APP_Rx_ptr_out = 0;
      APP_Rx_length = 0;
      APP_Rx_ptr_in = 0;

      VCP_DataTx (&average_16bit[0],0);
      VCP_DataTx (&average_16bit[1],0);
      host.aver_req_adc2ch1 = 0;
    }
    if (host.aver_req_adc1ch3 == 1)
    {
      average = AverMeas (host.aver_delay, host.aver_nsampl, adc1ch3);
      average_16bit[0] = average>>8;
      average_16bit[1] = average;
      APP_Rx_ptr_out = 0;
      APP_Rx_length = 0;
      APP_Rx_ptr_in = 0;

      VCP_DataTx (&average_16bit[0],0);
      VCP_DataTx (&average_16bit[1],0);
      host.aver_req_adc1ch3 = 0;
    }
    if (host.aver_req_all == 1)
    {
      int i;
      average = AverMeas (host.aver_delay, host.aver_nsampl, adc2ch1);
      average_16bit_all[0] = average>>8;
      average_16bit_all[1] = average;
      average = AverMeas (host.aver_delay, host.aver_nsampl, adc3ch2);
      average_16bit_all[2] = average>>8;
      average_16bit_all[3] = average;
      average = AverMeas (host.aver_delay, host.aver_nsampl, adc1ch3);
      average_16bit_all[4] = average>>8;
      average_16bit_all[5] = average;
      APP_Rx_ptr_out = 0;
      APP_Rx_length = 0;
      APP_Rx_ptr_in = 0;

      for(i=0; i<6;i++) VCP_DataTx (&average_16bit_all[i],0);

      host.aver_req_all = 0;
    }
    if (host.id_request == 1)
    {
      APP_Rx_ptr_out = 0;
      APP_Rx_length = 0;
      APP_Rx_ptr_in = 0;

      for (i=0; i < 9; i++) VCP_DataTx (&dev_id[i],0);
      host.id_request = 0;
    }
  }

}/*end main()*/




#ifdef USE_FULL_ASSERT

/**
* @brief  assert_failed
*         Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param  File: pointer to the source file name
* @param  Line: assert_param error line source number
* @retval None
*/

void assert_failed(uint8_t* file, uint32_t line)

{
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}

#endif

void LED_Config(void)

{
  /* Initialize Leds mounted on STM32F4-Discovery board */
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);

  /* Turn off LED4, LED3, LED5 and LED6 */
  STM_EVAL_LEDOff(LED4);
  STM_EVAL_LEDOff(LED3);
  STM_EVAL_LEDOff(LED5);
  STM_EVAL_LEDOff(LED6);
}

/*TIM4 initialisation*/
void InitTIM4(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* TIM4 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

  /* Enable the TIM4 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /*STM32F4 LED initialisation*/
  LED_Config();

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 8399;
  TIM_TimeBaseStructure.TIM_Prescaler = 10000;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

  /* Prescaler configuration */
  TIM_SetAutoreload(TIM4, (uint32_t)1);
  TIM_PrescalerConfig(TIM4, (uint32_t)350, TIM_PSCReloadMode_Immediate);
  
  /*Enable TIM4 Output trigger*/
  TIM_SelectOutputTrigger(TIM4, TIM_TRGOSource_Update);
  TIM_SelectMasterSlaveMode(TIM4, TIM_MasterSlaveMode_Enable);

  /*capture compare config*/
  TIM_SetCompare1(TIM4, 4200);
  /* TIM3 enable counter */
  TIM_Cmd(TIM4, ENABLE);
}

/* System initialisation*/
void SysInit(void)
{
  /* Enable DMA1 clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
  
  /*TIM4 CLK ENABLE*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  
  /*DAC Clk enable*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
  
  /* Enable GPIOD clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
  /* Enable GPIOA clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
  /* Enable GPIOE clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
}

/*GPIO Initialisation*/
void InitGPIO(void)
{
  GPIO_InitTypeDef* GPIO_Struct;
  GPIO_DeInit(GPIOD);
  GPIO_StructInit(GPIO_Struct);
  GPIO_Struct->GPIO_Pin = GPIO_Pin_12;
  GPIO_Struct->GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Struct->GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Struct->GPIO_OType = GPIO_OType_PP;
  GPIO_Struct->GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOD, GPIO_Struct);
  
  /*DAC Ch1 Output pin setting*/
  GPIO_DeInit(GPIOA);
  GPIO_StructInit(GPIO_Struct);
  GPIO_Struct->GPIO_Pin = GPIO_Pin_4;
  GPIO_Struct->GPIO_Mode = GPIO_Mode_AN;
  GPIO_Struct->GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, GPIO_Struct);
  
  /*DAC Ch2 Output pin setting*/
  GPIO_DeInit(GPIOA);
  GPIO_StructInit(GPIO_Struct);
  GPIO_Struct->GPIO_Pin = GPIO_Pin_5;
  GPIO_Struct->GPIO_Mode = GPIO_Mode_AN;
  GPIO_Struct->GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, GPIO_Struct);
}

/*ADC initialisation*/
void InitADC1_CH1_CH2(void)
{
  GPIO_InitTypeDef GPIOA_Struct;
  ADC_CommonInitTypeDef ADC_CommonInitStruct;
  ADC_InitTypeDef ADC1_Struct;
  
  /*ADC1 clock enable*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); 
  /*ADC GPIOs clock enable (ADC1 IN1 -> PA1)*/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
  /*Configure pin in analog mode*/
  GPIO_DeInit(GPIOA);
  GPIO_StructInit(&GPIOA_Struct);

  /*Analog mode*/
  GPIOA_Struct.GPIO_Pin  = GPIO_Pin_1 | GPIO_Pin_2;
  GPIOA_Struct.GPIO_Mode = GPIO_Mode_AN;
  
  GPIO_Init(GPIOA, &GPIOA_Struct);
  
  /*ADC configuration*/
  ADC_DeInit();
  ADC_StructInit(&ADC1_Struct);

  ADC_CommonStructInit(&ADC_CommonInitStruct);
  ADC_Init(ADC1, &ADC1_Struct);
  ADC_CommonInit(&ADC_CommonInitStruct);
  /* ADC123_IN1 -> Channel_1*/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_15Cycles);
  /* ADC123_IN2 -> Channel_2*/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_15Cycles);
  /*EOC signal enable*/
  ADC_EOCOnEachRegularChannelCmd(ADC1, ENABLE);
  
  /*ADC enable*/
  ADC_Cmd(ADC1, ENABLE);
}


/* Setup SysTick Timer for 1 msec interrupts.
     ------------------------------------------
    1. The SysTick_Config() function is a CMSIS function which configure:
       - The SysTick Reload register with value passed as function parameter.
       - Configure the SysTick IRQ priority to the lowest value (0x0F).
       - Reset the SysTick Counter register.
       - Configure the SysTick Counter clock source to be Core Clock Source (HCLK).
       - Enable the SysTick Interrupt.
       - Start the SysTick Counter.
    
    2. You can change the SysTick Clock source to be HCLK_Div8 by calling the
       SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8) just after the
       SysTick_Config() function call. The SysTick_CLKSourceConfig() is defined
       inside the misc.c file.

    3. You can change the SysTick IRQ priority by calling the
       NVIC_SetPriority(SysTick_IRQn,...) just after the SysTick_Config() function 
       call. The NVIC_SetPriority() is defined inside the core_cm3.h file.

    4. To adjust the SysTick time base, use the following formula:
                            
         Reload Value = SysTick Counter Clock (Hz) x  Desired Time base (s)
    
       - Reload Value is the parameter to be passed for SysTick_Config() function
       - Reload Value should not exceed 0xFFFFFF
   */
void SysTickInit(void)
{
  //SystemCoreClock
  if (SysTick_Config(SystemCoreClock / 1000))

  { 

    /* Capture error */ 

    while (1);

  }
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void delay_ms(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;
  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */

void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}
void SetFreq(int freq)
{
  int freq_tim;
  int prescaler;
  if (freq == 0)
  {
    //DMA_Cmd(DMA1_Stream5, DISABLE);
    DMA_Cmd(DMA1_Stream5, DISABLE);
    DAC_SetChannel1Data(DAC_Align_12b_R, const_out);  
  }
  else if (freq > 0)
  {
    freq_tim = freq;
      
    prescaler = (int)(10000 / freq_tim);
    /* Prescaler configuration */
    TIM_SetAutoreload(TIM4, (uint32_t)8399);
    TIM_PrescalerConfig(TIM4, (uint32_t) prescaler, TIM_PSCReloadMode_Immediate);
    //DMA_Cmd(DMA1_Stream5, ENABLE);
    
    DMA_Cmd(DMA1_Stream5, ENABLE);
  }
}

void SetDutyC(int duty_cycle)
{
  if (duty_cycle > 100) duty_cycle = 100;
  float dc = ((float)duty_cycle)/100;
  if (dc != 0 && dc != 1) 
  {
    TIM_SetCompare1(TIM4, (int)(dc*8399));
    STM_EVAL_LEDToggle(LED6);
  }
}

uint32_t ADC_measure(ADC_TypeDef* ADCx, uint8_t ADC_Channel_x)
{
    uint16_t ADC1ConvertedValue = 0;
    uint32_t ADC1ConvertedVoltage = 0;
    
    ADC_RegularChannelConfig(ADCx, ADC_Channel_x, 1, ADC_SampleTime_15Cycles);
    ADC_SoftwareStartConv(ADCx);
    while( ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC) == 0 );
    ADC1ConvertedValue = ADC_GetConversionValue(ADCx);
    ADC1ConvertedVoltage = ADC1ConvertedValue *3000/0xFFF;
    
    return ADC1ConvertedVoltage;
}





void InitSetStruct( Host_comm* host_new, Host_comm* host_old, set_struct* set_array)
{
  /*Function used to determine which paramenters need to be update*/ 
  set_array->scope = (host_new->scope != host_old->scope);
  set_array->diode = (host_new->diode != host_old->diode);
  set_array->pulse = (host_new->pulse != host_old->pulse);
  set_array->duty_cycle = (host_new->duty_cycle != host_old->duty_cycle);
  set_array->temperature = (host_new->temperature != host_old->temperature);
  set_array->current = (host_new->current != host_old->current);
  set_array->voltage = (host_new->voltage != host_old->voltage);
  set_array->freq = (host_new->freq != host_old->freq);
  set_array->sampl_freq = (host_new->sampling_freq != host_old->sampling_freq);
  set_array->ampli_setting = (int)(host_new->ampli_setting != host_old->ampli_setting);
  /*set_array->lmp_change = (int)((host_new->offset != host_old->offset) || 
                                (host_new->gain1 != host_old->gain1) ||
                                (host_new->gain2 != host_old->gain2));*/
  int off_change = (int) (host_new->offset != host_old->offset);
  int gain1_change = (int) (host_new->gain1 != host_old->gain1);
  int gain2_change = (int) (host_new->gain2 != host_old->gain2);
  //set_array->lmp_change = (int)((host_new->offset != host_old->offset));
  //set_array->lmp_change = (int)((off_change + gain1_change + gain2_change)>0);
  set_array->lmp_change = (int)(off_change || gain1_change || gain2_change);
}

/*ADC3 ch2 sampling detector voltage*/
void ADC3_CH2_DMA_Config(void)
{
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;
  NVIC_InitTypeDef      NVIC_InitStructure;

  /* Enable the DMA2_Stream0 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable ADC3, DMA2 and GPIO clocks ****************************************/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

  /* DMA2 Stream0 channel0 configuration **************************************/
  DMA_InitStructure.DMA_Channel = DMA_Channel_2;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC3_DR_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&adc3ch2;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 1000;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);
  DMA_Cmd(DMA2_Stream0, ENABLE);

  /* Configure ADC3 Channel2 pin as analog input*******************************/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC3 Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 2;
  ADC_Init(ADC3, &ADC_InitStructure);

  /* ADC3 regular channel12 configuration *************************************/
  ADC_RegularChannelConfig(ADC3, ADC_Channel_2, 1, ADC_SampleTime_15Cycles);
  ADC_RegularChannelConfig(ADC3, ADC_Channel_1, 2, ADC_SampleTime_15Cycles);

 /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);

  /* Enable ADC3 DMA */
  ADC_DMACmd(ADC3, ENABLE);

  /* Enable ADC3 */
  ADC_Cmd(ADC3, ENABLE);
  
  /*Enable DMA interrupts*/
  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
}

/*TIM2 initialisation*/
/*TIM2 sets the sampling frequency of ADC3*/
void InitTIM2(void)
{ 
  /* TIM2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  /* Prescaler configuration */
  TIM_SetAutoreload(TIM2, (uint32_t)99);
  //TIM_PrescalerConfig(TIM4, (uint32_t)700, TIM_PSCReloadMode_Immediate);
  //TIM_PrescalerConfig(TIM2, (uint32_t)1400, TIM_PSCReloadMode_Immediate);
  TIM_PrescalerConfig(TIM2, (uint32_t)42000, TIM_PSCReloadMode_Immediate);
  
  /*Enable TIM2 Output trigger*/
  TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
  //TIM_SelectOutputTrigger(TIM4, TIM_TRGOSource_OC1);
  TIM_SelectMasterSlaveMode(TIM2, TIM_MasterSlaveMode_Enable);
  /* TIM3 enable counter */
  TIM_Cmd(TIM2, ENABLE);
}

/*ADC2 ch1 sampling diode voltage*/
void ADC2_CH1_DMA_Config(void)
{
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;
  NVIC_InitTypeDef      NVIC_InitStructure;

  /* Enable the DMA2_Stream2 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable ADC3, DMA2 and GPIO clocks ****************************************/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);

  /* DMA2 Stream0 channel0 configuration **************************************/
  DMA_InitStructure.DMA_Channel = DMA_Channel_1;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC2_DR_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&adc2ch1;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 1000;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream2, &DMA_InitStructure);
  DMA_Cmd(DMA2_Stream2, ENABLE);

  /* Configure ADC1 Channel2 pin as analog input*******************************/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC1 Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 2;
  ADC_Init(ADC2, &ADC_InitStructure);

  /* ADC1 regular channel12configuration **************************************/
  ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 1, ADC_SampleTime_15Cycles);

 /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC2, ENABLE);

  /* Enable ADC2 DMA */
  ADC_DMACmd(ADC2, ENABLE);

  /* Enable ADC2 */
  ADC_Cmd(ADC2, ENABLE);
  
  /*Enable DMA interrupts*/
  DMA_ITConfig(DMA2_Stream2, DMA_IT_TC, ENABLE);
}

/*SamplPerSec is the number of acquired samples per second*/
void SetSamplFreq(int SamplPerSec)
{
  int presc;
  presc = (int)((84000000/100) / SamplPerSec);
  if (presc > 65535) presc = 65535;
  TIM_PrescalerConfig(TIM2, (uint32_t)presc, TIM_PSCReloadMode_Immediate);
}

/*Get sampling frequency value*/
int GetSamplFreq(void)
{
  int SamplPerSec;
  int presc;
  presc = TIM_GetPrescaler(TIM2);
  SamplPerSec = (int)((84000000/100) / presc);

  return SamplPerSec;
}

/*Measurement of average voltage: starting from n_ms millisececonds after the DMA 
transfer complete and until the output signal is active (depending on frequency and 
duty cycle)*/

int AverageM (int n_ms, int signal_freq, int sampling_freq, int duty_cycle, uint16_t m_vector[])
{
  int average;
  int N;
  int N_i;
  float t_end;
  float t_init;
  int i;
  
  t_end = ((float)(duty_cycle) / 100) / signal_freq;
  //t_end = ((float)(duty_cycle) / 100) / 1;
  //t_end = 0.5;
  
  t_init = (float)(n_ms)/1000;
    
  N_i = (int)(t_init*sampling_freq);
  //N_i = (int)(0.2*sampling_freq);
  N = (int)((t_end - t_init)*sampling_freq);
  //N = (int)((0.3)*sampling_freq);
  
  if (N > 1000) N = 1000;
  
  average = 0;
  /*
  for (i=N_i; i<N_i+N; i++) average += m_vector[i];
  average = (int)(average / N);
  */
  
  for (i=N_i; i<N_i+N; i++) average += m_vector[i]/N;
  average = (int)(average);
  
  //for (i=N_i; i<N_i+10; i++) average += m_vector[i];
  //average = (int)(average / 10);
  
  if (t_end - n_ms/1000 < 0) average = -1;
  
  return average;
  //return t_end*1000;
  //return N_i;
  //return N;
}

/*This function computes the average of the input signal starting from 
sample #<delay> over n_sampl samples. The input signal is triggered with
the DAC output*/
int AverMeas (int delay, int n_sampl, uint16_t m_vector[])
{
  int average = 0;
  int i;
  
  if (delay > 1000) delay = 1000;
  if (n_sampl > 1000) n_sampl = 1000;
  
  for (i=delay; i<delay+n_sampl; i++) average += m_vector[i];
  average = (int)(average/n_sampl);
  
  return average;
}

/*ADC1 ch3 (PA3) sampling MHP voltage/current*/
void ADC1_CH3_DMA_Config(void)
{
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;
  NVIC_InitTypeDef      NVIC_InitStructure;

  /* Enable the DMA2_Stream4 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable ADC3, DMA2 and GPIO clocks ****************************************/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  /* DMA2 Stream4 channel0 configuration **************************************/
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&adc1ch3;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 1000;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream4, &DMA_InitStructure);
  DMA_Cmd(DMA2_Stream4, ENABLE);

  /* Configure ADC1 Channel3 pin as analog input ******************************/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC1 Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel3 configuration *************************************/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 1, ADC_SampleTime_15Cycles);

 /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
  
  /*Enable DMA interrupts*/
  DMA_ITConfig(DMA2_Stream4, DMA_IT_TC, ENABLE);
}



void InitGPIO_SDWN(void)
{
  /*set PE11 as digital output pin to switch on and off the LDO*/
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  
  //GPIO_ResetBits(GPIOE, GPIO_Pin_11);
  //STM_EVAL_LEDToggle(LED4);
}
/**

  * @}

  */ 





/**

  * @}

  */ 





/**

  * @}

  */ 



/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
