/****************************************************************************
* 文件名：I2C.c
* 内容简述: 
*			      模拟I2C
*			   
*			                          
*文件历史：
*			版本号	  创建日期		作者
*			 v0.1	 2012/11/03	   TianHei
*联系方式：Qq:763146170  Email：763146170@qq.com
* 说    明：
****************************************************************************/

#include "include.h"
#include "stm32f10x_gpio.h"
#include "I2C.h"



/*
 *简述 I2C引脚配置
 *参数 无
 *返回 无
*/
void I2C_GPIOInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	
		/*使能端口B的时钟*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB
							,ENABLE);
	
	/* Configure I2C1 pins: SCL and SDA */
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	/*开漏输出模式，需要上拉电阻*/
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
  SDA_H;						//SDA置高
  SCL_H;						//SCL置高
}

/*
 *简述 I2C延时控制
 *参数 无
 *返回 无
*/
void I2C_delay(void)
{	
   u8 i=30; 
   while(i) 
   { 
     i--; 
   } 
}

/*
 *简述 启动一次I2C通信
 *参数 无
 *返回 成功返回1，失败返回0
*/
bool I2C_Start(void)
{
	SDA_H;						//SDA置高
	SCL_H;						//SCL置高
	I2C_delay();
	if(!SDA_read)return FALSE;	//SDA线为低电平则总线忙,退出
	SDA_L;
	I2C_delay();
	if(SDA_read) return FALSE;	//SDA线为高电平则总线出错,退出
	SDA_L;						//SDA置低
	I2C_delay();
	return TRUE;
}

/*
 *简述 停止I2C
 *参数 无
 *返回 无
*/
void I2C_Stop(void)
{
	SCL_L;				  
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SDA_H;
	I2C_delay();
}

/*
 *简述 I2C ACK应答
 *参数 无
 *返回 无
*/
void I2C_Ack(void)
{	
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}

/*
 *简述 I2C 无应答
 *参数 无
 *返回 无
*/
void I2C_NoAck(void)
{	
	SCL_L;
	I2C_delay();
	SDA_H;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}

/*
 *简述 I2C 等待应答
 *参数 无
 *返回 返回为:=1有ACK,=0无ACK
*/
bool I2C_WaitAck(void)
{
	SCL_L;
	I2C_delay();
	SDA_H;			
	I2C_delay();
	SCL_H;
	I2C_delay();
	if(SDA_read)
	{
     SCL_L;
		 I2C_delay();
     return FALSE;
	}
	SCL_L;
	return TRUE;
}

/*
 *简述 I2C 发送一字节 数据从高位到低位
 *参数 SendByte：要发送发字节
 *返回 无
*/
void I2C_SendByte(u8 SendByte)
{
  u8 i = 8;
	
  while(i--)
  {
    SCL_L;
    I2C_delay();
    if(SendByte&0x80)
      SDA_H;  
    else 
      SDA_L;   
    SendByte<<=1;
    I2C_delay();
		SCL_H;
    I2C_delay();
  }
	
  SCL_L;
}

/*
 *简述 I2C 接收一字节 数据从高位到低
 *参数 无
 *返回 收到的数据
*/
u8 I2C_ReceiveByte(void)
{ 
  u8 i = 8;
  u8 ReceiveByte = 0;

  SDA_H;				
  while(i--)
  {
    ReceiveByte<<=1;      
    SCL_L;
    I2C_delay();
	  SCL_H;
    I2C_delay();	
    if(SDA_read)
    {
      ReceiveByte|=0x01;
    }
  }
  SCL_L;
	
  return ReceiveByte;
}

/*
 *简述 I2C 写单字节函数
 *参数 dev_addr：器件地址
 *参数 reg_addr：写入数据的起始地址
 *参数 data：需要写入的数据
 *返回 成功或失败
*/
bool Single_Write(uint8_t dev_addr,uint8_t reg_Addr,uint8_t data)
{
    if(!I2C_Start())return FALSE;
    I2C_SendByte(dev_addr);   //发送设备地址+写信号//I2C_SendByte(((REG_Address & 0x0700) >>7) | SlaveAddress & 0xFFFE);//设置高起始地址+器件地址 
    if(!I2C_WaitAck()){I2C_Stop(); return FALSE;}
    I2C_SendByte(reg_Addr);   //设置低起始地址      
    I2C_WaitAck();  
    I2C_SendByte(data);
    I2C_WaitAck();   
    I2C_Stop(); 
    //delay5ms();
    return TRUE;
}

/*
 *简述 I2C 单字节读函数
 *参数 dev_addr：器件地址
 *参数 reg_addr：读取数据的地址
 *返回 读取的数据
*/
uint8_t Single_Read(uint8_t dev_addr,uint8_t reg_Addr)
{   
    uint8_t REG_data;       
    if(!I2C_Start())return FALSE;
    I2C_SendByte(dev_addr); //I2C_SendByte(((REG_Address & 0x0700) >>7) | REG_Address & 0xFFFE);//设置高起始地址+器件地址 
    if(!I2C_WaitAck()){I2C_Stop();return FALSE;}
    I2C_SendByte((u8) reg_Addr);   //设置低起始地址      
    I2C_WaitAck();
    I2C_Start();
    I2C_SendByte(dev_addr+1);
    I2C_WaitAck();

    REG_data= I2C_ReceiveByte();
    I2C_NoAck();
    I2C_Stop();
    //return true;
    return REG_data;
}

/*
 *简述 I2C 写数据到器件的指定起始地址
 *参数 dev_addr：器件地址
 *参数 reg_addr：写入数据的起始地址
 *参数 pdata：指向需要写入的数据的起始地址
 *参数 count：需要写入数据的个数
 *返回 无
*/
void I2C_WriteData(u8 dev_addr, u8 reg_addr, u8 *pdata, u8 count)
{
	u8 i = 0;
	
	I2C_Start();
	I2C_SendByte(dev_addr);				//写从器件地址
	I2C_WaitAck();
	
	I2C_SendByte(reg_addr);				//写数据地址
	I2C_WaitAck();
	I2C_Start();                  //重新启动总线
	
	for(i=count; i!=0; --i)
	{
		I2C_SendByte(*pdata++);			 //写数据
		I2C_WaitAck();
//		pdata++;
	}
	
	I2C_Stop();
}

/*
 *简述 I2C 读取器件指定起始地址的数据
 *参数 dev_addr：器件地址
 *参数 reg_addr：读取数据的起始地址
 *参数 pdata：指向存放数据的地址
 *参数 count：需要读取数据的个数
 *返回 无
*/
void I2C_ReadData(u8 dev_addr, u8 reg_addr, u8 *pdata, u8 count)
{
	u8 i = 0;
	
	I2C_Start();
	I2C_SendByte(dev_addr);				//写从器件地址
	I2C_WaitAck();
	
	I2C_SendByte(reg_addr);				//写数据地址
	I2C_WaitAck();

	I2C_Start();                  //重新启动总线
	I2C_SendByte(dev_addr + 1);   //器件地址加一表示要读
  I2C_WaitAck();
	
	for(i=1; i<count; ++i)
	{
		*pdata++ = I2C_ReceiveByte();			 //读数据
		I2C_Ack();
//		pdata++;
	}

	*pdata++ = I2C_ReceiveByte();			 //读数据
	I2C_NoAck();
	I2C_Stop();
}





