/*******************************************************************************
* File Name          : main
* Author             : luohui
* Date               : 21-08-2014
* E-mail             : luohui2320@gmail.com
* Description        : 	
											利用STM32的USB库实现 USB_keyboard功能
********************************************************************************/
#include "include.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include <stdio.h>
#include <stdlib.h>

#define WHITE_LED_PIN GPIO_Pin_15
#define YELLOW_LED_PIN GPIO_Pin_3

#define FLASH_RANDOM_POOL 0x0801FC00
#define FLASH_SIGN 0x01010101
#define PASSWORD_LEN 25

__IO uint8_t PrevXferComplete = 1;
INT8U Send_Buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};

unsigned char password_base[PASSWORD_LEN];
unsigned char write_buf[1024];
unsigned int  password_mask = 0x00;
unsigned int  not_enter = 1;
unsigned int  mode_2 = 1;

volatile int btn = 0;

void 				System_Init(void);
int 				flash_init(void);
uint32_t 		flash_read(uint32_t address);
void 				flash_erase(unsigned int pageAddress);
void 				flash_write(unsigned char* data, unsigned int address, unsigned int count);
void        send_key(int mod, int key);
void 				send_array(unsigned char *array, unsigned int size);
void 				get_password(int num);

void TIM3_IRQHandler(void) {
	GPIOA->BSRR = WHITE_LED_PIN;
	if (mode_2 == 0) GPIOB->BSRR = YELLOW_LED_PIN; else GPIOB->BRR = YELLOW_LED_PIN;
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
}

int main(void) {
	int i;
	for (i=0; i<5000000; i++);
	
	FLASH->KEYR = 0x45670123;
	FLASH->KEYR = 0xCDEF89AB;
	 	
  Set_System();
	System_Init();
  USB_Interrupts_Config();
  Set_USBClock();
  USB_Init();
	SysTick_Config(0xFFF);
	flash_init();
	
	if (((GPIOB->IDR & GPIO_Pin_12) == 0) && ((GPIOB->IDR & GPIO_Pin_4) == 0)) {
		flash_erase(FLASH_RANDOM_POOL);
		while (1) {
			GPIOB->ODR ^= YELLOW_LED_PIN;
			for (i=0; i<1000000; i++);
		}
	}
	
	if ((GPIOB->IDR & GPIO_Pin_12) == 0) {
		GPIOA->BRR = WHITE_LED_PIN;
		not_enter = 0;
	  while ((GPIOB->IDR & GPIO_Pin_12)  == 0) __NOP();	
	}
	
	TIM3_Init();
	
  while (1)
  {
    if (bDeviceState == CONFIGURED) {
			if (btn == 0) {
				if ((GPIOB->IDR & GPIO_Pin_4)  == 0) btn = 1;
				if ((GPIOB->IDR & GPIO_Pin_12) == 0) btn = 2;
				if ((GPIOB->IDR & GPIO_Pin_8)  == 0) btn = 3;
				if ((GPIOB->IDR & GPIO_Pin_9)  == 0) btn = 4;
				if ((GPIOB->IDR & GPIO_Pin_2)  == 0) btn = 5;
				if ((GPIOB->IDR & GPIO_Pin_11) == 0) btn = 6;
				
				if (btn != 0) {
					if (btn == 1) {
						if (mode_2 == 0) {
							mode_2 = 1; 
						} else {
							mode_2 = 0;
						}
					} else {
						get_password(btn);
						send_array(password_base, PASSWORD_LEN);
					}
					
					TIM3->CNT = 0;
					GPIOA->BRR = WHITE_LED_PIN;
					
					for (i=0; i<5000000; i++);
					btn = 0;
				}
			}
    } else {
			TIM3->CNT = 0;
			GPIOA->BRR = WHITE_LED_PIN;
			for (i=0; i<5000000; i++);
		}
  }
}

void System_Init(void) {
	GPIO_InitTypeDef xIn;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
  xIn.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_9 | GPIO_Pin_8 | GPIO_Pin_12 | GPIO_Pin_11 | GPIO_Pin_2;
  xIn.GPIO_Speed = GPIO_Speed_50MHz;
  xIn.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOB, &xIn);
	
  xIn.GPIO_Pin =  WHITE_LED_PIN;
  xIn.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &xIn);
	GPIO_ResetBits(GPIOA, WHITE_LED_PIN);
	
  xIn.GPIO_Pin = YELLOW_LED_PIN;
	xIn.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &xIn);
	GPIO_ResetBits(GPIOB, YELLOW_LED_PIN);
}

void send_key(int mod, int key) {
	PrevXferComplete = 0;
	Send_Buffer[0] = mod;
	Send_Buffer[2] = key;
	USB_SIL_Write(EP1_IN, Send_Buffer, 8);
	SetEPTxValid(ENDP1);
	while (PrevXferComplete==0) __NOP();
}

void send_array(unsigned char *array, unsigned int size) {
	int i, pmask, smask;
	for (i=0; i<size; i++) {
		pmask = i % 32;
		if (mode_2 == 1) 
			smask = (((password_mask >> pmask) & 0x01) == 1) ? (array[i] < 0x1E ? 0x02 : 0) : 0;
		else 
			smask = (((password_mask >> pmask) & 0x01) == 1) ? 0x02 : 0;
		send_key(smask, (mode_2 == 1) ? array[i] : (0x27+0x04)-array[i]);
		send_key(0, 0);
	}
	
	if (not_enter) {
		send_key(0, 0x28);
		send_key(0, 0);
	}
}

void flash_erase(unsigned int pageAddress) {
	while (FLASH->SR & FLASH_SR_BSY);
	if (FLASH->SR & FLASH_SR_EOP) {
		FLASH->SR = FLASH_SR_EOP;
	}

	FLASH->CR |= FLASH_CR_PER;
	FLASH->AR = pageAddress;
	FLASH->CR |= FLASH_CR_STRT;
	while (!(FLASH->SR & FLASH_SR_EOP));
	FLASH->SR = FLASH_SR_EOP;
	FLASH->CR &= ~FLASH_CR_PER;
}

uint32_t flash_read(uint32_t address) {
  return (*(__IO uint32_t*) address);
}

void flash_write(unsigned char* data, unsigned int address, unsigned int count) {
	unsigned int i;

	while (FLASH->SR & FLASH_SR_BSY);
	if (FLASH->SR & FLASH_SR_EOP) {
		FLASH->SR = FLASH_SR_EOP;
	}

	FLASH->CR |= FLASH_CR_PG;

	for (i = 0; i < count; i += 2) {
		*(volatile unsigned short*)(address + i) = (((unsigned short)data[i + 1]) << 8) + data[i];
		while (!(FLASH->SR & FLASH_SR_EOP));
		FLASH->SR = FLASH_SR_EOP;
	}

	FLASH->CR &= ~(FLASH_CR_PG);
}

void get_password(int num) {
	unsigned int i, j = 0, off, fl;
	off = FLASH_RANDOM_POOL + 64 + (num * 32);
	for (i=off; i<(PASSWORD_LEN+off); i++, j++) {
		fl = flash_read(i);
		password_base[j] = (fl >> 24) & 0xFF;	
	}

	//password_base[PASSWORD_LEN-1] = 0x00;
	password_mask = flash_read(FLASH_RANDOM_POOL + (num * 4) + 32);
}

int flash_init(void) {
	int i, j, pinval, last;
	GPIO_InitTypeDef xIn;
	
	uint32_t val = flash_read(FLASH_RANDOM_POOL);
	if (val == FLASH_SIGN) return 1;
	
	flash_erase(FLASH_RANDOM_POOL);
	write_buf[0] = 0x01;
	write_buf[1] = 0x01;
	write_buf[2] = 0x01;
	write_buf[3] = 0x01;
	for (i=4; i<32; i++) write_buf[i] = 0x00;
	
  xIn.GPIO_Pin = GPIO_Pin_All;
  xIn.GPIO_Speed = GPIO_Speed_50MHz;
  xIn.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &xIn);
	
	i = 32;
	while (1) {
		pinval = (GPIOB->IDR & 0xFF) ^ ((GPIOB->IDR >> 8) & 0xFF) ^ (SysTick->VAL & 0xFF) ^ (rand() & 0xFF);
		if ((pinval > 0x03) && (pinval < 0x28) && (pinval != last)) {
			write_buf[i] = pinval & 0xFF;
			last = pinval;
			i++;
			if (i >= 1024) break;
		}
		GPIOA->ODR ^= WHITE_LED_PIN;
		if ((rand()/1111) == 0) for (j=0; j<(50); j++) __NOP();
	}
	
	flash_write(write_buf, 0x0801FC00, 1024);
	
	while (1) {
		GPIOA->ODR ^= WHITE_LED_PIN;
		for (i=0; i<1000000; i++);
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
