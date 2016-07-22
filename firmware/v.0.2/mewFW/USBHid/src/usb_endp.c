/**
  ******************************************************************************
  * @file    usb_endp.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Endpoint routines
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
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_istr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern __IO uint8_t PrevXferComplete;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : EP1_OUT_Callback.
* Description    : EP1 OUT Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_IN_Callback(void)
{
  /* Set the transfer complete token to inform upper layer that the current 
  transfer has been complete */
  PrevXferComplete = 1; 
}

/*******************************************************************************
* Function Name  : EP1_OUT_Callback.
* Description    : EP1 OUT Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_OUT_Callback(void)
{
u8 DataLen; //保存接收数据的长度
u8 DataBuffer[64]; //保存接收数据的缓冲区

DataLen = GetEPRxCount(ENDP1); //获取收到的长度
PMAToUserBufferCopy(DataBuffer, ENDP1_RXADDR, DataLen); //复制数据
SetEPRxValid(ENDP1); //设置端点有效，以接收下一次数据

if(DataLen==1) //收到一字节的输出报告
{
  //D0位表示数字键盘灯，D1位表示大写字母锁定灯
//  if(DataBuffer[0]&0x01)  //数字键盘灯亮
//  {
//   GPIOC->BSRR=(1<<6); //亮LED3
//  }
//  else
//  {
//   GPIOC->BRR=(1<<6); //灭LED3
//  }
//  if(DataBuffer[0]&0x02) //大写字母锁定键
//  {
//   GPIOC->BSRR=(1<<7); //亮LED2
//  }
//  else
//  {
//   GPIOC->BRR=(1<<7); //灭LED2
//  }
}

}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

