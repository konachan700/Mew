
#ifndef I2C_H_
#define I2C_H_

/*I2C控制线的定义*/
#define SCL_H         GPIOB->BSRR = GPIO_Pin_11			   
#define SCL_L         GPIOB->BRR  = GPIO_Pin_11 
   
#define SDA_H         GPIOB->BSRR = GPIO_Pin_10
#define SDA_L         GPIOB->BRR  = GPIO_Pin_10

#define SCL_read      GPIOB->IDR  & GPIO_Pin_11
#define SDA_read      GPIOB->IDR  & GPIO_Pin_10


void I2C_GPIOInit(void);

void I2C_delay(void);

bool I2C_Start(void);

void I2C_Stop(void);

void I2C_Ack(void);

void I2C_NoAck(void);

bool I2C_WaitAck(void);

u8 I2C_ReceiveByte(void);

void I2C_SendByte(u8 SendByte);

bool Single_Write(uint8_t dev_addr,uint8_t reg_Addr,uint8_t data);

uint8_t Single_Read(uint8_t dev_addr,uint8_t reg_Addr);

void I2C_WriteData(u8 dev_addr, u8 reg_addr, u8 *pdata, u8 count);

void I2C_ReadData(u8 dev_addr, u8 reg_addr, u8 *pdata, u8 count);

#endif

