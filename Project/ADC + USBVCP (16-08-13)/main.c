
/* USB VCP and ADC test */


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "usbd_cdc_core.h"
#include "usbd_cdc_vcp.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4_discovery.h"
#include "usbd_hid_core.h"
#include "main.h"
/* USB Setup------------------------------------------------------------------*/
__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END ;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED

  #if defined ( __ICCARM__ ) /*!< IAR Compiler */

    #pragma data_alignment=4   

  #endif

#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

/*Function Prototypes---------------------------------------------------------*/
void SysTickInit(void);
void Delay(__IO uint32_t nTick);
void adc_configure(void);
int adc_convert(void);
void startUSBOTG(void);
void SysInit(void);
void InitTIM2(void);
void InitADC1_CH1_CH2(void);
uint32_t ADC_measure(ADC_TypeDef* ADCx, uint8_t ADC_Channel_x);
void ADC1_CH3_DMA_Config(void);
void ADC2_CH1_DMA_Config(void);
void ADC1_Config(void);
void SetSamplFreq(int SamplPerSec);
void startUSBHID(void); 
void delay_ms(__IO uint32_t nTime);
void TimingDelay_Decrement(void);
void DAC_mhp_init(void); //For DAC microhotplate
static void TIM6_Config(void); //For DAC microhotplate
void LED_Init(void);
void LED_On(int);
void LED_Off(int);


/* Global Variables and Definitions-------------------------------------------*/
#define ADC1_DR_ADDRESS     ((uint32_t)0x4001204C)
#define ADC2_DR_ADDRESS     ((uint32_t)0x4001214C)
#define ADC3_DR_ADDRESS     ((uint32_t)0x4001224C)
#define ON_BOARD 0

extern uint8_t BIG_BUF[];   
extern uint8_t BIG_BUF_Ptr; 

// These are external variables imported from CDC core to be used for IN 
//   transfer management. 
extern uint8_t  APP_Rx_Buffer []; /* Write CDC received data in this buffer.
                                     These data will be sent over USB IN endpoint
                                     in the CDC core functions. */
extern uint32_t APP_Rx_ptr_in;    /* Increment this pointer or roll it back to
                                     start address when writing received data
                                     in the buffer APP_Rx_Buffer. */
extern uint32_t APP_Rx_ptr_out;
extern uint32_t APP_Rx_length;

int adc1_trigger = 0;
uint16_t DMABuffer[100] = {0};
uint16_t ADCBuffer[4]    = {0};
uint16_t adc_16[100] = {0};
uint8_t adc_8[200] = {0};
uint16_t dsp_16[500] = {0};
uint16_t dsp_1[125] = {0};
uint16_t dsp_2[125] = {0};
uint16_t dsp_3[125] = {0};
uint16_t dsp_4[125] = {0};
int D1 = 0;
int D2 = 0;
int D3 = 0;
int D4 = 0;
int currentLED = 12;
int calibRun = 1;
int DSPPos = 0;
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
static __IO uint32_t TimingDelay;
const uint8_t aEscalator8bit[6] = {0x99, 0x99, 0x99, 0x99, 0x99, 0x99}; //For DAC microhotplate
int write  = 0;
struct pair 
{
  int min;
  int max;
};   //Used to store co-ords of maxima in data

int main(void)
{
  //Initial Set-up of: SPI, USB, TIM2, and ADC
  SysInit();
  LMP_SPI_Init();                       
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  InitTIM2();
  startUSBOTG();
  SysTickInit();
  ADC1_Config();
  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
  TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);
  SetSamplFreq(250);
  //DAC_mhp_init(); //For DAC microhotplate
  //TIM6_Config(); //For DAC microhotplate
  LED_Init();
  
  //Send LMP91050 settings over SPI
  LMP_SPI_send(0,250,4);

  //Main Loop: Transmit ADC result to PC
  while (1)
  {
    //Output over USB VCP, write set high at end of 
    //conversion interrupt
    if(write == 1 )
    {
      //Copy from DMA buffer to output array
      for(int i = 0; i<100; i++) adc_16[i] = DMABuffer[i];
#if ON_BOARD
      //Write data to dsp buffer for peak detect for LED control
      int position = DSPPos;
      for(int i = 0; i <4; i++) 
      {
          dsp_16[position + i] = ADCBuffer[i];
      }
      DSPPos += 4;
#endif
      //Convert 16b array elements in 100 element array to 8 bit words and store in 
      //200 element array
      for (int i=0; i<200; i++)
      {
          if (i%2 == 0) adc_8[i] = (adc_16[i/2] >> 8);
          else adc_8[i] = (adc_16[i/2]);
      }
      
     //Output 8 element array of ADC results over USB VCP Data line, APP_Rx etc.
     //to define output as 8 bit string. i.e. send 0 after each 8 bits
     
      APP_Rx_ptr_out = 0;
      APP_Rx_length = 0;
      APP_Rx_ptr_in = 0;
      for(int i=0; i<200; i++) VCP_DataTx(&adc_8[i], 0);
      
      //Set 0, waiting for next EOC interrupt to run 
      write = 0;
#if ON_BOARD
      //DSP, peak detect and LED output to run when DSP buffer full, 
      //i.e. DSPPos > 124. Runs every 0.5s. Set DSPPos = 0 to start 
      //writing from begining again for next 0.5s set
      if(DSPPos > 500)
      { 
          //Store signal data in an array for each thermopile ADC channel
          //Conditional on being more than background value, if less then set =0
          for(int i = 0; i<125; i++)
          {
            dsp_1[i] = (dsp_16[4*i] > D1 ? dsp_16[4*i] - D1 : 0);
            dsp_2[1+i] = (dsp_16[1+4*i] >D2 ? dsp_16[1+4*i] - D2 : 0);
            dsp_3[2+i] = (dsp_16[2+4*i] >D3 ? dsp_16[2+4*i] - D3 : 0);
            dsp_4[3+i] = (dsp_16[3+4*i] >D4 ? dsp_16[3+4*i] - D4 : 0);
          }
          int avg1 = 0, avg2 = 0, avg3 = 0, avg4 = 0;
          for(int i = 0; i<125; i++)
          {
            avg1 += dsp_1[i];
            avg2 += dsp_2[i];
            avg3 += dsp_3[i];
            avg4 += dsp_4[i];
          }
          avg1 = avg1/125;
          avg2 = avg2/125;
          avg3 = avg3/125;
          avg4 = avg4/125;      
          
          if(calibRun)
          {
            D1 = avg1;
            D2 = avg2;
            D3 = avg3;
            D4 = avg4;
            calibRun = 0;
          }
          int D1Max = 0, D2Max = 0, D3Max = 0, D4Max = 0; 
          int n1 = 0, n2 = 0, n3 = 0, n4 = 0;
          for(int i = 0; i< 125; i++)
          {
            if(dsp_1[i] > D1Max)
            {
            D1Max = dsp_1[i];
            n1 = i;
            }
            if(dsp_2[i] > D2Max)
            {
            D2Max = dsp_2[i];
            n2 = i;
            }
            if(dsp_3[i] > D3Max)
            {
            D3Max = dsp_3[i];
            n3 = i;
            }
            if(dsp_4[i] > D4Max)
            {
            D4Max = dsp_4[i];
            n4 = i;
            }
      
          }
          if (n1 > n2 && (D1Max - avg1 > 100 || D2Max - avg2 >100))
          {
            LED_Off(currentLED);
            currentLED = (currentLED == 15 ? 12 : currentLED++);  
            LED_On(currentLED);
          }
          if (n1 < n2 && (D1Max - avg1 >  100 || D2Max - avg2 > 100))
          {
            LED_Off(currentLED);
            currentLED = (currentLED == 12 ? 15 : currentLED--);  
            LED_On(currentLED);
          }

          if (n1 > n2 && (D1Max - avg1 > 100 || D2Max - avg2 >100))
          {
            LED_Off(15);
            LED_On(13);
          }
          if (n1 < n2 && (D1Max - avg1 >  100 || D2Max - avg2 > 100))
          {
            LED_Off(13);
            LED_On(15);
          }
          if (n3 > n1  && D3Max - avg3 > 100 && D1Max - avg1 > 100)
          {
            LED_Off(12);
            LED_On(14);
          }
          if (n3 < n1 && D3Max - avg3 >  100 && D1Max - avg1 > 100)
          {
            LED_Off(14);
            LED_On(12);
          }
          DSPPos = 0;
      }
#endif
    }
  }
}

  


/*Function Definitions--------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

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
 
}
//Set the sampling frequency in samples per second by adjusting TIM2 prescaler
void SetSamplFreq(int SamplPerSec)
{
  int presc;
  presc = (int)((84000000/100) / SamplPerSec);
  if (presc > 65535) presc = 65535;
  TIM_PrescalerConfig(TIM2, (uint32_t)presc, TIM_PSCReloadMode_Immediate);
}

//Starts USBOTG: Turns on and connects the USB-OTG port, ready for VCP
void startUSBHID(void) 
{
  USBD_Init(&USB_OTG_dev,
#ifdef USE_USB_OTG_HS 
  USB_OTG_HS_CORE_ID,
#else            
  USB_OTG_FS_CORE_ID,
#endif  
  &USR_desc, 
  &USBD_HID_cb,
  &USR_cb);
}

void startUSBOTG(void) 
{
  USBD_Init(&USB_OTG_dev,
#ifdef USE_USB_OTG_HS 
  USB_OTG_HS_CORE_ID,
#else            
  USB_OTG_FS_CORE_ID,
#endif  
  &USR_desc, 
  &USBD_CDC_cb,
  &USR_cb);
}

//Delay: Used to space output
void Delay(__IO uint32_t nTick)
{
  for(; nTick != 0; nTick--);
}


//TIM2 initialisation:TIM2 sets the sampling frequency of ADC3
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


void ADC1_Config(void)
{
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;
  NVIC_InitTypeDef      NVIC_InitStructure;

  /* Enable the DMA2_Stream0 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable ADC1, DMA2 and GPIOA clocks ***************************************/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  /* DMA2 Stream0 channel0 configuration **************************************/
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_ADDRESS; 
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&DMABuffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 100; 
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

  /* Configure ADC1 Channel2-5 pin as analog input*****************************/
  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2; 
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; 
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);

  /* ADC3 Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; 
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 4; 
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC3 regular channel12 configuration *************************************/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_15Cycles);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_15Cycles);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 3, ADC_SampleTime_15Cycles);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 4, ADC_SampleTime_15Cycles);

 /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
  
  /*Enable DMA transfer to buffer complete interrupts*/
  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE); 
  /*This fires an interrupt 
   after filling the DMA Buffer. You need to do something after interrupt or 
   nothing will happen(program will hang), this can be used for timing as 
   buffer takes set time to fill (Set by DMA clock)*/

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


void SysTickInit(void)
{
  //SystemCoreClock
  if (SysTick_Config(SystemCoreClock / 1000))

  { 

    /* Capture error */ 

    while (1);

  }
}

void DAC_mhp_init(void)
{
  DAC_InitTypeDef  DAC_InitStructure;

   GPIO_InitTypeDef GPIO_InitStructure;
  /* GPIOA clock enable (to be used with DAC) */
 // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);                         
  /* DAC Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  DMA_InitTypeDef DMA_InitStructure;

  /* DAC channel1 Configuration */
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_1, &DAC_InitStructure);

  /* DMA1_Stream5 channel7 configuration **************************************/  
  DMA_DeInit(DMA1_Stream5);
  DMA_InitStructure.DMA_Channel = DMA_Channel_7;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = DAC_DHR8R1_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&aEscalator8bit;
  DMA_InitStructure.DMA_BufferSize = 6;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream5, &DMA_InitStructure);    

  /* Enable DMA1_Stream5 */
  DMA_Cmd(DMA1_Stream5, ENABLE);
  
  /* Enable DAC Channel1 */
  DAC_Cmd(DAC_Channel_1, ENABLE);

  /* Enable DMA for DAC Channel1 */
  DAC_DMACmd(DAC_Channel_1, ENABLE);
}

static void TIM6_Config(void)
{
  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
  /* TIM6 Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  
  /* --------------------------------------------------------
  TIM3 input clock (TIM6CLK) is set to 2 * APB1 clock (PCLK1), 
  since APB1 prescaler is different from 1.   
    TIM6CLK = 2 * PCLK1  
    TIM6CLK = HCLK / 2 = SystemCoreClock /2 
          
  TIM6 Update event occurs each TIM6CLK/256 

  Note: 
   SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f4xx.c file.
   Each time the core clock (HCLK) changes, user had to call SystemCoreClockUpdate()
   function to update SystemCoreClock variable value. Otherwise, any configuration
   based on this variable will be incorrect.    

  ----------------------------------------------------------- */
  /* Time base configuration */
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
  TIM_TimeBaseStructure.TIM_Period = 0xFF;          
  TIM_TimeBaseStructure.TIM_Prescaler = 0;       
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;    
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

  /* TIM6 TRGO selection */
  TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
  
  /* TIM6 enable counter */
  TIM_Cmd(TIM6, ENABLE);
}
/*---------Initialise the on board LEDs---------------------------------------*/
void LED_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

 /* GPIOG Peripheral clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  /* Configure PG6 and PG8 in output pushpull mode */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 |GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);


}
/*-------------Lazy LED On/Off functions--------------------------------------*/
void LED_On(int pin)
{
   GPIOD->BSRRL = (1 << pin);
}
void LED_Off(int pin)
{
   GPIOD->BSRRH = (1 << pin);

}
/*-----------------Peak Detect (Maxima) Functions-----------------------------*/
struct pair getMinMax(int arr[], int n)
{
  struct pair minmax;     
  int i;  
 
  /* If array has even number of elements then 
    initialize the first two elements as minimum and 
    maximum */
  if(n%2 == 0)
  {         
    if(arr[0] > arr[1])     
    {
      minmax.max = arr[0];
      minmax.min = arr[1];
    }  
    else
    {
      minmax.min = arr[0];
      minmax.max = arr[1];
    }
    i = 2;  /* set the startung index for loop */
  }  
 
   /* If array has odd number of elements then 
    initialize the first element as minimum and 
    maximum */
  else
  {
    minmax.min = arr[0];
    minmax.max = arr[0];
    i = 1;  /* set the startung index for loop */
  }
   
  /* In the while loop, pick elements in pair and 
     compare the pair with max and min so far */   
  while(i < n-1)  
  {          
    if(arr[i] > arr[i+1])          
    {
      if(arr[i] > minmax.max)        
        minmax.max = arr[i];
      if(arr[i+1] < minmax.min)          
        minmax.min = arr[i+1];        
    } 
    else        
    {
      if(arr[i+1] > minmax.max)        
        minmax.max = arr[i+1];
      if(arr[i] < minmax.min)          
        minmax.min = arr[i];        
    }        
    i += 2; /* Increment the index by 2 as two 
               elements are processed in loop */
  }            
 
  return minmax;
}    
#ifdef USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

