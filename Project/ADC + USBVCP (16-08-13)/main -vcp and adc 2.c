/* USB VCP and ADC test */


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "usbd_cdc_core.h"
#include "usbd_cdc_vcp.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

/* USB Setup------------------------------------------------------------------*/
__ALIGN_BEGIN USB_OTG_CORE_HANDLE    USB_OTG_dev __ALIGN_END ;

/*Function Prototypes---------------------------------------------------------*/
void Delay(__IO uint32_t nTick);
void adc_configure(void);
int adc_convert(void);
void startUSBOTG(void);
void SysInit(void);
void InitTIM2(void);

/* Variables------------------------------------------------------------------*/
int ConvertedValue = 0; //Converted value readed from ADC
uint8_t ADCReading[2000] = {0};
uint8_t ADCInput[1000] = {0};
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

int main(void)
{
  SysInit();
  LMP_SPI_Init();
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  InitTIM2();
  startUSBOTG();
  adc_configure();
//char* TextOut = "Test Text";
  while (1)
  {
    for (int i=0; i<1000; i++)
    {
      ADCInput[i] = i;
      Delay(0xFFFF);
    }
    for (int i=0; i<2000; i++)
      {
        if (i%2 == 0) ADCReading[i] = (ADCInput[i/2] >> 8);
        else ADCReading[i] = (ADCInput[i/2]);
      }
      
   for (int i=0; i < 2000; i++) VCP_DataTx (&ADCReading[i],0);

    Delay(0xFFFF);
  }
}

/*Function Definitions--------------------------------------------------------*/
//Start USBOTG: Turns on and connects the USB-OTG port, ready for VCP
void startUSBOTG(void) {
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

//ADC Configuration: Sets up GPIO for Analogue mode, configures ADC, Enables ADC
void adc_configure(){
 ADC_InitTypeDef ADC_init_structure; //Structure for adc confguration
 GPIO_InitTypeDef GPIO_initStructre; //Structure for analog input pin
 //Clock configuration
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);//The ADC1 is connected the APB2 peripheral bus thus we will use its clock source
 RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOCEN,ENABLE);//Clock for the ADC port!! Do not forget about this one ;)
 //Analog pin configuration
 GPIO_initStructre.GPIO_Pin = GPIO_Pin_0;//The channel 10 is connected to PC0
 GPIO_initStructre.GPIO_Mode = GPIO_Mode_AN; //The PC0 pin is configured in analog mode
 GPIO_initStructre.GPIO_PuPd = GPIO_PuPd_NOPULL; //We don't need any pull up or pull down
 GPIO_Init(GPIOC,&GPIO_initStructre);//Affecting the port with the initialization structure configuration
 //ADC structure configuration
 ADC_DeInit();
 ADC_init_structure.ADC_DataAlign = ADC_DataAlign_Right;//data converted will be shifted to right
 ADC_init_structure.ADC_Resolution = ADC_Resolution_12b;//Input voltage is converted into a 12bit number giving a maximum value of 4096
 ADC_init_structure.ADC_ContinuousConvMode = ENABLE; //the conversion is continuous, the input data is converted more than once
 ADC_init_structure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;// conversion is synchronous with TIM1 and CC1 (actually I'm not sure about this one :/)
 ADC_init_structure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//no trigger for conversion
 ADC_init_structure.ADC_NbrOfConversion = 1;//I think this one is clear :p
 ADC_init_structure.ADC_ScanConvMode = DISABLE;//The scan is configured in one channel
 ADC_Init(ADC1,&ADC_init_structure);//Initialize ADC with the previous configuration
 //Enable ADC conversion
 ADC_Cmd(ADC1,ENABLE);
 //Select the channel to be read from
 ADC_RegularChannelConfig(ADC1,ADC_Channel_10,1,ADC_SampleTime_144Cycles);
}

//ADC Conversion: Takes ADC value and returns integer result
int adc_convert(){
 ADC_SoftwareStartConv(ADC1);//Start the conversion
 while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));//Processing the conversion
 return ADC_GetConversionValue(ADC1); //Return the converted data
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