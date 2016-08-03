

#ifndef USART_H_
#define USART_H_

extern uint8_t rec_f;  		//接收标志位
extern uint8_t RxCounter;	//接收字符记数
extern uint8_t RxBuff[];		//接收缓冲

void USART_InitConfig(void);

void USARTx_PrintfChar(USART_TypeDef* USARTx, uint8_t m_char);

void USARTx_PrintfString(USART_TypeDef* USARTx, uint8_t* pm_string);

void USARTx_PrintfUint32(USART_TypeDef* USARTx, uint32_t m_uint32);

void UART3_ReportIMU(int16_t yaw,int16_t pitch,int16_t roll,int16_t alt,int16_t tempr,int16_t press,int16_t IMUpersec);

void UART3_ReportMotion(int16_t ax,int16_t ay,int16_t az,int16_t gx,int16_t gy,int16_t gz,int16_t hx,int16_t hy,int16_t hz);

#endif
