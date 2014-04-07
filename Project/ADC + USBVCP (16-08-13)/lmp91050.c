#include "lmp91050.h"

int old_sampl_freq;
int offset_meas = 0; //(Phil) not declared? 

#if 0
void LMP_SPI_Init(void)
{
  SPI_InitTypeDef SPI2_InitStruct;
  SPI_InitTypeDef SPI3_InitStruct;
  GPIO_InitTypeDef GPIOB_InitStruct;
  
  /*Clock enabling*/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable the SPI clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

  /* Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd(SPI2_SCK_GPIO_CLK | SPI2_MOSI_GPIO_CLK | SPI2_MISO_GPIO_CLK | SPI2_SS_GPIO_CLK , ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  //GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  //GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;

  /* SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = SPI2_SCK_PIN;
  GPIO_Init(SPI2_SCK_GPIO_PORT, &GPIO_InitStructure);
  
  /* Connect SPI pins to AF5 */  
  GPIO_PinAFConfig(SPI2_SCK_GPIO_PORT, SPI2_SCK_SOURCE, SPI2_SCK_AF);
  
  /* SPI MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  SPI2_MOSI_PIN;
  GPIO_Init(SPI2_MOSI_GPIO_PORT, &GPIO_InitStructure);
  
  GPIO_PinAFConfig(SPI2_MOSI_GPIO_PORT, SPI2_MOSI_SOURCE, SPI2_MOSI_AF);
  
  /* SPI MISO pin configuration */
  GPIO_InitStructure.GPIO_Pin =  SPI2_MISO_PIN;
  GPIO_Init(SPI2_MISO_GPIO_PORT, &GPIO_InitStructure);
  
  GPIO_PinAFConfig(SPI2_MOSI_GPIO_PORT, SPI2_MOSI_SOURCE, SPI2_MOSI_AF);
  
  /* SPI SS pin configuration */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Pin =  SPI2_SS_PIN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_SetBits(SPI2_SS_GPIO_PORT , SPI2_SS_PIN);
  GPIO_Init(SPI2_SS_GPIO_PORT, &GPIO_InitStructure);
  
  SPI_StructInit(&SPI2_InitStruct);  
  
  //SPI2_InitStruct.SPI_Direction = SPI_Direction_1Line_Tx;
  SPI2_InitStruct.SPI_DataSize = SPI_DataSize_16b;
  SPI2_InitStruct.SPI_Mode = SPI_Mode_Master;
  SPI2_InitStruct.SPI_NSS = SPI_NSS_Soft;
  SPI2_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  
  SPI_Init(SPI2, &SPI2_InitStruct);
  //SPI_Cmd(SPI2, ENABLE);
  SPI_SSOutputCmd(SPI2, ENABLE);
}
#endif

void LMP_SPI_Init(void)
{
  SPI_InitTypeDef SPI2_InitStruct;
  SPI_InitTypeDef SPI3_InitStruct;
  GPIO_InitTypeDef GPIOB_InitStruct;
  
  /*Clock enabling*/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable the SPI clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

  /* Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd(SPI2_SCK_GPIO_CLK | SPI2_MOSI_GPIO_CLK | SPI2_MISO_GPIO_CLK | SPI2_SS_GPIO_CLK , ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  //GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  //GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;

  /* SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = SPI2_SCK_PIN;
  GPIO_Init(SPI2_SCK_GPIO_PORT, &GPIO_InitStructure);
  
  /* Connect SPI pins to AF5 */  
  GPIO_PinAFConfig(SPI2_SCK_GPIO_PORT, SPI2_SCK_SOURCE, SPI2_SCK_AF);
  
  /* SPI MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  SPI2_MOSI_PIN;
  GPIO_Init(SPI2_MOSI_GPIO_PORT, &GPIO_InitStructure);
  
  GPIO_PinAFConfig(SPI2_MOSI_GPIO_PORT, SPI2_MOSI_SOURCE, SPI2_MOSI_AF);
  
  /* SPI MISO pin configuration */
  GPIO_InitStructure.GPIO_Pin =  SPI2_MISO_PIN;
  GPIO_Init(SPI2_MISO_GPIO_PORT, &GPIO_InitStructure);
  
  GPIO_PinAFConfig(SPI2_MOSI_GPIO_PORT, SPI2_MOSI_SOURCE, SPI2_MOSI_AF);
  //Error 120813
  /* SPI SS and SS2 pin configuration */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Pin =  SPI2_SS_PIN | SPI2_SS2_PIN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_SetBits(SPI2_SS_GPIO_PORT , SPI2_SS_PIN);
  GPIO_SetBits(SPI2_SS2_GPIO_PORT , SPI2_SS2_PIN);
  GPIO_Init(SPI2_SS_GPIO_PORT, &GPIO_InitStructure);
  
  SPI_StructInit(&SPI2_InitStruct);  
  
  /* SPI SS3 and SS4 pin configuration */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Pin =  SPI2_SS3_PIN | SPI2_SS4_PIN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_SetBits(SPI2_SS3_GPIO_PORT , SPI2_SS3_PIN);
  GPIO_SetBits(SPI2_SS4_GPIO_PORT , SPI2_SS4_PIN);
  GPIO_Init(SPI2_SS3_GPIO_PORT, &GPIO_InitStructure);
  
  SPI_StructInit(&SPI2_InitStruct);  
  
  //SPI2_InitStruct.SPI_Direction = SPI_Direction_1Line_Tx;
  SPI2_InitStruct.SPI_DataSize = SPI_DataSize_16b;
  SPI2_InitStruct.SPI_Mode = SPI_Mode_Master;
  SPI2_InitStruct.SPI_NSS = SPI_NSS_Soft;
  SPI2_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  
  SPI_Init(SPI2, &SPI2_InitStruct);
  //SPI_Cmd(SPI2, ENABLE);
  SPI_SSOutputCmd(SPI2, ENABLE);
}

void LMP_SPI_send(uint8_t offset, uint8_t gain1, uint8_t gain2)
{
  /*0b01100000*/
  uint16_t device_conf = 0x0060; 
  uint16_t receive;
  uint16_t off = offset +(0x01<<8);
  int i=0;
  /*offset = 0x80;
  
  gain2 = 3;
  gain1 = 1;*/
  
  
  device_conf += (gain2<<1) + gain1;

  device_conf = 0x0060;
  off = 0x0180;
  
  SPI_Cmd(SPI2, ENABLE);
/*----------------------------------------------------------------------------*/  
  GPIO_ResetBits(SPI2_SS_GPIO_PORT , SPI2_SS_PIN);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

  SPI_I2S_SendData(SPI2, device_conf);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
 //SPI_I2S_SendData(SPI2, off);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
  //Delay(1000);  //120813 Changed from "delay_ms"
 
  GPIO_SetBits(SPI2_SS_GPIO_PORT , SPI2_SS_PIN);
  Delay(1000); //120813 Changed from "delay_ms"
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
/*----------------------------------------------------------------------------*/  
  GPIO_ResetBits(SPI2_SS2_GPIO_PORT , SPI2_SS2_PIN);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

  SPI_I2S_SendData(SPI2, device_conf);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
 // SPI_I2S_SendData(SPI2, off);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
  //Delay(1000);  //120813 Changed from "delay_ms"
 
  GPIO_SetBits(SPI2_SS2_GPIO_PORT , SPI2_SS2_PIN);
  Delay(1000); //120813 Changed from "delay_ms"
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
/*----------------------------------------------------------------------------*/  
  GPIO_ResetBits(SPI2_SS3_GPIO_PORT , SPI2_SS3_PIN);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

  SPI_I2S_SendData(SPI2, device_conf);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
  //SPI_I2S_SendData(SPI2, off);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
  //Delay(1000);  //120813 Changed from "delay_ms"
 
  GPIO_SetBits(SPI2_SS3_GPIO_PORT , SPI2_SS3_PIN);
  Delay(1000); //120813 Changed from "delay_ms"
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
/*----------------------------------------------------------------------------*/  
  GPIO_ResetBits(SPI2_SS4_GPIO_PORT , SPI2_SS4_PIN);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

  SPI_I2S_SendData(SPI2, device_conf);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
 while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
 // SPI_I2S_SendData(SPI2, off);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
  //Delay(1000);  //120813 Changed from "delay_ms"
 
  GPIO_SetBits(SPI2_SS4_GPIO_PORT , SPI2_SS4_PIN);
  Delay(1000); //120813 Changed from "delay_ms"
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
  
  
  /*//SPI_Cmd(SPI2, ENABLE);
  Delay(10000);
  GPIO_ResetBits(SPI2_SS_GPIO_PORT , SPI2_SS_PIN);
  Delay(10000);
  GPIO_ResetBits(SPI2_SS2_GPIO_PORT , SPI2_SS2_PIN);
  Delay(10000);
  GPIO_ResetBits(SPI2_SS3_GPIO_PORT , SPI2_SS3_PIN);
  Delay(10000);
  GPIO_ResetBits(SPI2_SS4_GPIO_PORT , SPI2_SS4_PIN);
  Delay(10000);
  SPI_Cmd(SPI2, DISABLE);
  */
}

#if 0
void LMP_SPI_send(uint8_t offset, uint8_t gain1, uint8_t gain2)
{
  /*0b01100000*/
  uint16_t device_conf = 0x0060; 
  uint16_t receive;
  uint16_t off = offset +(0x01<<8);
  int i=0;
  /*offset = 0x80;
  
  gain2 = 3;
  gain1 = 1;*/
  
  device_conf += (gain2<<1) + gain1;

  SPI_Cmd(SPI2, ENABLE);
  
  GPIO_ResetBits(SPI2_SS_GPIO_PORT , SPI2_SS_PIN);
  Delay(10); //120813 Changed from "delay_ms"
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

  SPI_I2S_SendData(SPI2, device_conf);
  
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

  SPI_I2S_SendData(SPI2, off);

  Delay(10);  //120813 Changed from "delay_ms"
 
  GPIO_SetBits(SPI2_SS_GPIO_PORT , SPI2_SS_PIN);
  
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET);
  //SPI_Cmd(SPI2, ENABLE);
  SPI_Cmd(SPI2, DISABLE);
  
}
#endif

void Offset_Measurement_On(void)
{  
   /*DAC DMA request disable to measure the dark detector output*/
   DAC_DMACmd(DAC_Channel_1, DISABLE);
   /*MHP Off*/
   DAC_SetChannel1Data(DAC_Align_12b_R, 0);
   
   /*DMA2 interrupt will not disable ADC3 because triggering is not needed for 
   offset measurement*/
    offset_meas = 1;  
   /*ADC3 is ENABLED because triggering from the DAC output is not needed*/
   //ADC_Cmd(ADC3, ENABLE);
   ADC_DMACmd(ADC3, ENABLE);
   old_sampl_freq = GetSamplFreq();
   
   /*4000 samples per second: we need to wait 0.25 seconds to fill the 
   1000 elements buffer*/
   SetSamplFreq(4000);
}

void Offset_Measurement_Off(void)
{
   /*Restoration of the system configuration*/
   DAC_DMACmd(DAC_Channel_1, ENABLE);
   SetSamplFreq(old_sampl_freq);
   //ADC_Cmd(ADC3, DISABLE);
   ADC_DMACmd(ADC3, DISABLE);
   offset_meas = 0;
}
