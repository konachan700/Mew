/****************************************************************************
* 文件名：USART.c
* 内容简述: 
*			      串口初始化及相关函数定义
*			      初始化的三个步骤原则：
*			                           1、先使能相关时钟，2、配置外设 3、要用中断则配置中毒案控制器
*文件历史：
*			版本号	  创建日期		作者
*			 v0.1	 2012/11/02	   TianHei
*联系方式：Qq:763146170  Email：763146170@qq.com
* 说    明：
****************************************************************************/

#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "include.h"
#include <stdio.h>

//uint8_t rec_f = 0;  		      //接收标志位
//uint8_t RxCounter = 0;	      //接收字符记数
//uint8_t RxBuff[512] = {0};		//接收缓冲

/*
 *简述 相关时钟使能
 *参数 无
 *返回 无
*/
__inline void Uart_ClockInit(void)
{
	/*使能端口A的时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA
							,ENABLE);
	
	/*使能串口1时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
}

/*
 *简述 串口1引脚初始化操作
 *参数 无
 *返回 无
*/
__inline void Uart_GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				//USART1 TX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//A端口

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				//USART1 RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//复用开漏输入
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//A端口
}

/*
 *简述 串口初始化操作
 *参数 USARTx：要初始化的串口号
 *返回 无
*/
void USART_InitConfig(void)
{
	USART_InitTypeDef USART_InitStructure;
  
	/*时钟初始化*/
	Uart_ClockInit();
	
	/*引脚初始化*/
	Uart_GPIOInit();
	
	USART_InitStructure.USART_BaudRate = 115200;					           //速率115200bps
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		 //数据位8位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;			   //停止位1位
	USART_InitStructure.USART_Parity = USART_Parity_No;				     //无校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		 //无硬件流控
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						 //收发模式

	USART_Init(USART1, &USART_InitStructure);							//配置串口参数函数

//	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);                    //使能接收中断
//	USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);						        //使能发送缓冲空中断

	USART_Cmd(USART1, ENABLE);							//使能串口
}

/*
 * 描述 串口发送一字节数据
 * 参数 USARTx：串口号
 * 参数 m_char：要发送的数据
 * 返回 无 
 */
void USARTx_PrintfChar(USART_TypeDef* USARTx, uint8_t m_char)
{
	USART_SendData(USARTx, m_char);
  while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
}

/*
 * 描述 串口发送一字节数据
 * 参数 USARTx：串口号
 * 参数 pm_string：指向要发送字符串的指针
 * 返回 无 
 */
void USARTx_PrintfString(USART_TypeDef* USARTx, uint8_t* pm_string)
{
	while((*pm_string) != '\0')
	{
		USARTx_PrintfChar(USARTx, *pm_string);
		pm_string++;
	}
}

/*
 * 描述 以十进制发送32位整型数据
 * 参数 USARTx：串口号
 * 参数 m_uint32：要发送的数
 * 返回 无 
 */
void USARTx_PrintfUint32(USART_TypeDef* USARTx, uint32_t m_uint32)
{
	uint8_t i;
	uint8_t buff[10];
	for(i=9;i!=0;i--)
	{
	 	buff[i] = m_uint32%10 + '0';  //转字符串
		m_uint32 /=	10;
	}
	for(i=0;i<10;i++)
	{
		USARTx_PrintfChar(USARTx, buff[i]);
	}
}

/*
 * 描述 重定向c库函数printf到USART3
 * 参数
 * 返回
 */
#if 1
int fputc(int ch, FILE *f)
{
  /*将Printf内容发往串口*/
  USART_SendData(USART1, (unsigned char) ch);
  while (!(USART1->SR & USART_FLAG_TXE));
  return (ch);
}
#endif

