/**
  ******************************************************************************
  * @file    stm32_it.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include "stm32_it.h"
#include "usb_istr.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "platform_config.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//INT32U key_count = 0;
	/*保存当前按键端口状态*/
//__IO INT8U key1 = 0xe0;
	/*保存上一状态按键端口状态*/
//__IO INT8U key2 = 0xe0;
/* Extern variables ----------------------------------------------------------*/
//extern INT8U flag;
//extern INT8U key_flag;
//extern ADXL345_TYPE ADXL345_data;

volatile int counter = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M Processor Exceptions Handlers                         */
/******************************************************************************/

/*******************************************************************************
* Function Name  : NMI_Handler
* Description    : This function handles NMI exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NMI_Handler(void)
{
}

/*******************************************************************************
* Function Name  : HardFault_Handler
* Description    : This function handles Hard Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : MemManage_Handler
* Description    : This function handles Memory Manage exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : BusFault_Handler
* Description    : This function handles Bus Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : UsageFault_Handler
* Description    : This function handles Usage Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : SVC_Handler
* Description    : This function handles SVCall exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SVC_Handler(void)
{
}

/*******************************************************************************
* Function Name  : DebugMon_Handler
* Description    : This function handles Debug Monitor exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DebugMon_Handler(void)
{
}

/*******************************************************************************
* Function Name  : PendSV_Handler
* Description    : This function handles PendSVC exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PendSV_Handler(void)
{
}

/*******************************************************************************
* Function Name  : SysTick_Handler
* Description    : This function handles SysTick Handler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*            STM32 Peripherals Interrupt Handlers                            */
/*******************************************************************************
* Function Name  : USB_IRQHandler
* Description    : This function handles USB Low Priority interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  USB_Istr();
}
/*******************************************************************************
* Function Name  : EXTI_IRQHandler
* Description    : This function handles External lines  interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
//  if (EXTI_GetITStatus(KEY_BUTTON_EXTI_LINE) != RESET)
//  {
    /* Check if the remote wakeup feature is enabled (it could be disabled 
        by the host through ClearFeature request) */
//    if (pInformation->Current_Feature & 0x20) 
//    {      
//      pInformation->Current_Feature &= ~0x20;  
      /* Exit low power mode and re-configure clocks */
//      Resume(RESUME_INTERNAL);
//    }
  
    /* Clear the EXTI line pending bit */
//    EXTI_ClearITPendingBit(KEY_BUTTON_EXTI_LINE);
//  }
}

/*******************************************************************************
* Function Name  : USBWakeUp_IRQHandler
* Description    : This function handles USB WakeUp interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBWakeUp_IRQHandler(void)
{
  EXTI_ClearITPendingBit(EXTI_Line18);
}

/*******************************************************************************
* Function Name  : TIM3_IRQHandler
* Description    : This function handles TIM3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
//void TIM3_IRQHandler(void)
//{
//	counter++;
//	if (counter >= 3) counter = 0;
//	GPIOB->ODR |= (GPIO_Pin_3 | GPIO_Pin_4);
//	GPIOA->ODR |= (GPIO_Pin_4);
//	
//	switch (counter) {
//		case 0:
//			GPIOA->ODR &= ~(GPIO_Pin_4);
//			break;
//		case 1:
//			GPIOB->ODR &= ~(GPIO_Pin_3);
//			break;
//		case 2:
//			GPIOB->ODR &= ~(GPIO_Pin_4);
//			break;
//	}

	
//	/*保存当前按键端口状态*/
////  static	__IO INT8U key1 = 0xf0;
//	/*保存上一状态按键端口状态*/
////	static	__IO INT8U key2 = 0xf0;
//	
//	INT16U key = 0;
//	
//	/*连续读取陀螺仪的数值*/
//	//ADXL345_MultRead(&ADXL345_data);
//	
//	/*读按键端口值*/
//	key = GPIO_ReadInputData(GPIOB);
//	
//	//key1 = (INT8U) (key >> 8);
//	key1 = (INT8U)key;
//	
//	key1 &= 0xe0;
//	
//	
//	/*表示按键发现了改变*/
//	if(key1 != key2)
//	{
//		key_count++; //相当于延时
//		if(key_count >= 0x02)
//		{
//			key2 = key1;
//			key_flag = key2;
//		}
//	}
//	else
//	{
//		key_count = 0;
//	}
//	
//	/*设置标志*/
//	flag = 1;
	
	/*手动清数据更新中断标志*/
//	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
//}

/******************************************************************************/
/*                 STM32 Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32xxx.s).                                            */
/******************************************************************************/

/*******************************************************************************
* Function Name  : PPP_IRQHandler
* Description    : This function handles PPP interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*void PPP_IRQHandler(void)
{
}*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

