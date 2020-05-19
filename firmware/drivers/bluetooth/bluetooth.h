#ifndef MEW_BLUETOOTH_H
#define MEW_BLUETOOTH_H

#include "mew.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/dma.h>

#include <stdio.h>

#define MEW_BLUETOOTH_USART         USART1
#define MEW_BLUETOOTH_PORT          GPIOB
#define MEW_BLUETOOTH_PORT_AF       GPIO_AF7
#define MEW_BLUETOOTH_PIN           GPIO6 | GPIO7
#define MEW_BLUETOOTH_SPEED         115200
#define MEW_BLUETOOTH_IRQ           NVIC_USART1_IRQ
#define MEW_BLUETOOTH_ISR           usart1_isr

#define MEW_BLUETOOTH_POWER_PIN     GPIO1
#define MEW_BLUETOOTH_POWER_PORT    GPIOB

#define MEW_BLUETOOTH_RESET_PIN     GPIO5
#define MEW_BLUETOOTH_RESET_PORT    GPIOB

#define MEW_BLUETOOTH_DMA 				DMA2
#define MEW_BLUETOOTH_DMA_HANDLE                        dma2_stream7_isr
#define MEW_BLUETOOTH_DMA_STREAM_TX 			DMA_STREAM7
#define MEW_BLUETOOTH_DMA_CHANNEL_TX			DMA_SxCR_CHSEL_4
#define MEW_BLUETOOTH_DMA_DR				USART1_DR
#define MEW_BLUETOOTH_DMA_NVIC_TX			NVIC_DMA2_STREAM7_IRQ

//#define MEW_BT_RECEIVE_BUFFER_SIZE                      256
//
//#define MEW_BT_MODE_AT                  1
//#define MEW_BT_MODE_TRANSPARENT         2
//
//#define MEW_BT_CS_ERROR                 1
//#define MEW_BT_CS_OK                    2
//#define MEW_BT_CS_TIMEOUT               3

//typedef struct {
//    volatile uint32_t code;
//    volatile uint32_t state;
//    const char* command;
//    const char* wait_reply_pattern;
//} mew_bt_at_command;

unsigned int mew_bluetooth_init(void);
//unsigned int mew_bluetooth_handler(void);
void mew_bluetooth_transmit(uint8_t* data, uint16_t size, uint8_t sync_mode);

#endif /* BLUETOOTH_H */

