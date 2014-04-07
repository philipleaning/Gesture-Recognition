#include <stm32f4xx.h>
#include <stm32f4xx_spi.h>

//To start SPI peripheral

void init_SPI1(void) {
  //Create initiliasing structs
  GPIO_InitTypeDef GPIO_InitStruct;
  SPI_InitTypeDef SPI_InitStruct;

//Enable clock for IO pins
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
 //Configure pins used by SPI
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_5;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; //Alternate function
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  //Update GPIO settings
  GPIO_Init(GPIOA, &GPIO_InitStruct);
 
  //Connect SPI1 to AF pins
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource5, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource6, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource7, GPIO_AF_SPI1);
  
  //Enable clock
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  
  //Configure chip select pin (pin7)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStruct);
    
    GPIOE->BSRRL |= GPIO_Pin_7; // set PE7 high
  
  //Configure SPI1
  
  //Separate MOSI and MISO lines
  SPI_InitStruct. SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  //Master mode
  SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
  //Transmit 8bit data packets
  SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
  //Clock is low when idle
  SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
  //Data sampled at first edge
  SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
  //Internal NSS, set high
  SPI_InitStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set;
  //Set SPI1 frequency = APB2 freq.
  SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  //Set data order to transmit MSB first
  SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_Init(SPI1, &SPI_InitStruct);
  //Actually enable SPI1
  SPI_Cmd(SPI1,ENABLE);
}

//This function to transmit and receive data over SPI

uint8_t SPI1_send(uint8_t data) {
  //Write data to be written to the SPI Data Register
  SPI1->DR = data;
  //Wait until operation of sending is complete
  while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
  while( !(SPI1->SR & SPI_I2S_FLAG_RXNE) ); // wait until receive complete
  while( SPI1->SR & SPI_I2S_FLAG_BSY ); // wait until SPI is not busy anymore
  
  return SPI1->DR; //Return received data from the SPI Data Register
 }

int main(void) {
  uint8_t received_val = 0;
  init_SPI1();
   while(1){
          
          GPIOE->BSRRH |= GPIO_Pin_7; // set PE7 (CS) low
          SPI1_send(0xAA);  // transmit data
          received_val = SPI1_send(0x00); // transmit dummy byte and receive data
          GPIOE->BSRRL |= GPIO_Pin_7; // set PE7 (CS) high

}
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  

  
	
}