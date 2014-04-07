/* * Test code for the Discovery Board. In this version we use the DMA to store the information * of two converted channels (channel 14 and channel 15) into a pre-defined variable. The method * is continuous and the value of the ADC are continuously stored in the variable. */ 
//==Includes== 
#include "stm32f4xx.h"
#include "usbd_cdc_core.h"
#include "usbd_cdc_vcp.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
//==Definitions== 
#define ADC1_DR_Address ((uint32_t)0x4001244C) 

//==Global Variable declerations== 
uint16_t ADC_Val; //Stores the calculated ADC value
double voltage1; //Used to store the actual voltage calculated by ADC for the 1st channel 
double voltage2; //Used to store the actual voltage calculated by ADC for the 2nd channel
__IO uint16_t ADCConvertedValue[2]; //Array that is used to store the calculated DMA values for ADC1
/* * This function sets up the pins connected to the LED's as outputs; the blue LED * is connected to pin 8 (Port C) and the green LED is connected to pin 9 (Port C). */ 
void Configure_LED_Pins() {
 //==Configure the pins connected to the LED's to be outputs== 
  //Blue LED is on pin 8 of Port C and Green LED is on pin 9 of Port C 
  //Enable the clock for the port, by default this is off i.e. Enable GPIOC Clock 
 // RCC->APB2ENR |= RCC_APB2Periph_GPIOC; //APB2 indicates we dealing with the high speed bus
  //ENR means we want to enable the register 
  
  //Specify the pins as either inputs or outputs 
  //GPIOC->CRL = 0x11111111; //This is definition for pins 0 - 7; each pin is configured with respect to CONTROL:MODE 
  //All pins set to output mode (general purpose output push-pull)
  //GPIOC->CRH = 0x44444433; //This is definition for pins 8 - 15; pin 8 and 9 set as output (50Mhz), general purpose output 
  //Set all pins to 0V 
  GPIOC->ODR = 0x0000; //The output port only uses the first 16 bits; the last 16 isn't used...
  //Here we are turning off all the pins... }//end Configure_LED_Pins 
  /* * Sets up Pin.C4 (Channel 14) and Pin.C5 (Channel 15) to be used as analog inputs. The first channel 
  * of the DMA is also setup to be used with ADC1. ADC1 is setup to continuously output data to the 
  * array "ADCConvertedValue" */
  void ADC_DMA_Configuration() { 
    GPIO_InitTypeDef GPIO_InitStructure; //Variable used to setup the GPIO pins 
    DMA_InitTypeDef DMA_InitStructure; //Variable used to setup the DMA 
    ADC_InitTypeDef ADC_InitStructure; //Variable used to setup the ADC 
    
    //==Configure the systems clocks for the ADC and DMA== 
    ADCCLK = PCLK2 / 4 
    RCC_ADCCLKConfig(RCC_PCLK2_Div4); //Defines the ADC clock divider. This clock is derived from the APB2 clock (PCLK2). The 
    //ADCs are clocked by the clock of the high speed domian (APB2) dibivied by 2/4/6/8. The 
    //frequency can never be bigger than 14MHz!!!! 
    
    //--Enable DMA1 clock-- 
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); //--Enable ADC1 and GPIOC-- 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE); 
    
    //==Configure ADC pins (PC.04 -> Channel 14 and PC.05 -> Channel 15) as analog inputs== 
    GPIO_StructInit(&GPIO_InitStructure); // Reset init structure, if not it can cause issues... 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
    GPIO_Init(GPIOC, &GPIO_InitStructure); 
    
    //==Configure DMA1 - Channel1== 
    DMA_DeInit(DMA1_Channel1); //Set DMA registers to default values 
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address; //Address of peripheral the DMA must map to 
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) & ADCConvertedValue; //Variable to which ADC values will be stored
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; DMA_InitStructure.DMA_BufferSize = 2; //Buffer size (2 because we using two channels)
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure); //Initialise the DMA
    DMA_Cmd(DMA1_Channel1, ENABLE); //Enable the DMA1 - Channel1
   
    //==Configure ADC1 - Channel 14 and Channel 15== 
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE; 
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; 
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 
    ADC_InitStructure.ADC_NbrOfChannel = 2; //We using two channels 
    ADC_Init(ADC1, &ADC_InitStructure); //Initialise ADC1 
    
    //Setup order in which the Channels are sampled....
    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_55Cycles5); 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 2, ADC_SampleTime_55Cycles5); 
    ADC_DMACmd(ADC1, ENABLE); //Enable ADC1 DMA 
    ADC_Cmd(ADC1, ENABLE); //Enable ADC1 
    
    //==Calibrate ADC1== 
    
    //Enable ADC1 reset calibaration register 
    ADC_ResetCalibration(ADC1); 
    while (ADC_GetResetCalibrationStatus(ADC1)); //Check the end of ADC1 reset calibration register 
    
    //Start ADC1 calibaration 
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1)); //Check the end of ADC1 calibration 
  }//end ADC_Configuration 
  
  
  /* 
  * Simple program that reads our AD port, in this case pin 4 on port C. The following program does the following: 
  * The converted voltage is stored in the global variable 'voltage'; the following things are done when the voltage value is 
  * obtained: 
  * 0.000 < = V No LEDs on 
  * 0.825 <= V Green LED on; Blue LED off 
  * 1.650 <= V Blue LED on; Green LED off * 2.475 <= V Green LED on; Blue LED on 
  * 
  * Since the variable 'voltage' is a global variable, its value can be checked when debugging the code.... */
  void ADC_DMA_Test_Program() { 
    /* Start ADC1 Software Conversion */ 
    ADC_SoftwareStartConvCmd(ADC1, ENABLE); 
    while (1) { 
      //==Get the ADC value of channel 14== 
      ADC_Val = ADCConvertedValue[0]; 
      voltage1 = (2.984 * ADC_Val) / 4096; 
      //==Get the ADC value of channel 15== 
      ADC_Val = ADCConvertedValue[1]; 
      voltage2 = (2.984 * ADC_Val) / 4096;
     
      if ((voltage1 >= 0) && (voltage1 < 0.825)) {
        ODR = 0b0000000000000000; //Turn off both LED's 
      } else if ((voltage1 >= 0.825) && (voltage1 < 1.65)) {
        ODR = 0b0000001000000000; //Turn on the green LED
      } else if ((voltage1 >= 1.65) && (voltage1 < 2.475)) { 
        ODR = 0b0000000100000000; //Turn on the blue LED 
      } else {//voltage1 >= 2.475 && voltage
        ODR = 0b0000001100000000; //Turn on both LED's
      } 
    }//end while(1) 
  }//end ADC_Test_Program 
  
  /* * Main program start */ 
  
  int main(void) { 
    //**Micro clock settings** 
    //Done by default from startup_stm32f10x_xx.s before coming to main! You can edit SystemInit() in system_stm32f10x.c 
    Configure_LED_Pins(); 
    ADC_DMA_Configuration();
    ADC_DMA_Test_Program(); 
    return 0; 
  }//end main

