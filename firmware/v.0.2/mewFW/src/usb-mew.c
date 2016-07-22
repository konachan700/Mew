#include "stm32f10x_conf.h"
#include "usb_istr.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "platform_config.h"
#include "board_config.h"

__IO uint8_t PrevXferComplete = 1;

void USBWakeUp_IRQHandler(void) {
  EXTI_ClearITPendingBit(EXTI_Line18);
}

void USB_LP_CAN1_RX0_IRQHandler(void) {
  USB_Istr();
}
