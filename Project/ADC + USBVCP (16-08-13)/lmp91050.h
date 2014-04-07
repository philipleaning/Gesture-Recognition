#ifndef __LMP91050_H
#define __LMP91050_H

#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

extern int offset_meas;
//extern uint16_t adc3ch2[2000];

#define SPI2_SCK_PIN                   GPIO_Pin_10
#define SPI2_SCK_GPIO_PORT             GPIOB
#define SPI2_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOB
#define SPI2_SCK_SOURCE                GPIO_PinSource10
#define SPI2_SCK_AF                    GPIO_AF_SPI2

#define SPI2_MOSI_PIN                  GPIO_Pin_3
#define SPI2_MOSI_GPIO_PORT            GPIOC
#define SPI2_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOC
#define SPI2_MOSI_SOURCE               GPIO_PinSource3
#define SPI2_MOSI_AF                   GPIO_AF_SPI2

#define SPI2_MISO_PIN                  GPIO_Pin_14
#define SPI2_MISO_GPIO_PORT            GPIOB
#define SPI2_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define SPI2_MISO_SOURCE               GPIO_PinSource14
#define SPI2_MISO_AF                   GPIO_AF_SPI2

#define SPI2_SS_PIN                    GPIO_Pin_12
#define SPI2_SS_GPIO_PORT              GPIOB
#define SPI2_SS_GPIO_CLK               RCC_AHB1Periph_GPIOB
#define SPI2_SS_SOURCE                 GPIO_PinSource12
#define SPI2_SS_AF                     GPIO_AF_SPI2

#define SPI2_SS2_PIN                    GPIO_Pin_9
#define SPI2_SS2_GPIO_PORT              GPIOB
#define SPI2_SS2_GPIO_CLK               RCC_AHB1Periph_GPIOB
#define SPI2_SS2_SOURCE                 GPIO_PinSource9
#define SPI2_SS2_AF                     GPIO_AF_SPI2

#define SPI2_SS3_PIN                    GPIO_Pin_15
#define SPI2_SS3_GPIO_PORT              GPIOA
#define SPI2_SS3_GPIO_CLK               RCC_AHB1Periph_GPIOA
#define SPI2_SS3_SOURCE                 GPIO_PinSource15
#define SPI2_SS3_AF                     GPIO_AF_SPI2

#define SPI2_SS4_PIN                    GPIO_Pin_4
#define SPI2_SS4_GPIO_PORT              GPIOA
#define SPI2_SS4_GPIO_CLK               RCC_AHB1Periph_GPIOA
#define SPI2_SS4_SOURCE                 GPIO_PinSource4
#define SPI2_SS4_AF                     GPIO_AF_SPI2

void LMP_SPI_send(uint8_t offset, uint8_t gain1, uint8_t gain2);
void LMP_SPI_Init(void);
void Offset_Measurement_On(void);
void Offset_Measurement_Off(void);

#endif /*__LMP91050_H*/