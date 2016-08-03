
#ifndef __ADXL345_H__
#define __ADXL345_H__

/*定义器件地址*/

//#define IIC_ACC_ADDR 0x1D	  	//请注意此处 SDO已接上拉电阻 SDO默认高电平 IIC地址为0X1D  若接地则IIC地址为0x53  
							
#define ADXL345_Addr 0x3A		//读写地址也要根据SDO的电平修改


/*寄存器地址定义*/

#define THRESH_TAP 0x1D    //敲击事件阀值

#define OFSX 0x1E   	     //偏移寄存器X

#define OFSY 0x1F 		     //偏移寄存器Y

#define OFSZ 0x20 		     //偏移寄存器Z

#define DUR	0x21					 //设定发生敲击事件的最大时间为0则禁用单击、双击功能

#define Latent 0x22  			 //时间相关，为0禁用双击功能

#define Window 0x23 			 //时间相关，为0禁用双击功能

#define THRESH_ACT 0x24		 //活动阀值

#define THRESH_INACT 0x25  //保存静止阀值

#define TIME_INACT 0x26    //时间相关

#define ACT_INACT_CTL 0x27 //中断相关使能禁止

#define THRESH_FF 0x28     //用于检测是否发生自由落体事件

#define TIME_FF 0x29       //检测自由落体时间相关

#define TAP_AXES 0x2A      //敲击检测相关，为1使能各轴进行敲击检测，为0从敲击检测参与轴排除选定轴

#define ACT_TAP_STATUS 0x2B //休眠设置与事件参与轴设置，为0不休眠不参与

#define BW_RATE 0x2C       //功耗和速率选择，为1是低功耗噪声增加

#define POWER_CTL 0x2D 		 //工作状态相关

#define INT_ENABLE 0x2E 	 //相关中断使能位设置

#define INT_MAP 0x2F 			 //中断发生引脚映射配置

#define INT_SOURCE 0x30    //只读，中断标志位

#define DATA_FORMAT 0x31   //数据格式相关设置

#define DATAAX0 0x32       //X轴数据0	为低有效字节

#define DATAAX1 0x33       //X轴数据1	为高有效字节

#define DATAAY0 0x34       //Y轴数据0

#define DATAAY1 0x35       //Y轴数据1

#define DATAAZ0 0x36       //Z轴数据0

#define DATAAZ1 0x37       //Z轴数据1

#define FIFO_CTL 0x38      //设置FIF0的模式

#define	FIFO_STATUS 0x39   //FIFO相关


/*数据类型声明*/

typedef struct
{
  int16_t ax;
  int16_t ay;
  int16_t az;   
}ADXL345_TYPE;


/*函数声明*/

void ADXL345_Init(void);

void ADXL345_Calibrate(void);

void ADXL345_Read(ADXL345_TYPE * ptResult);

void ADXL345_MultRead(ADXL345_TYPE * ptResult);

void ADXL345_Printf(ADXL345_TYPE * ptResult);

//void ADXL345_ReadData(s16 * pwGyroX, s16 * pwGyroY, s16 * pwGyroZ);

#endif

