/****************************************************************************
* 文件名：adxl.c
* 内容简述: 
*			      加速度传感器ADXL345相关函数
*			   
*			                          
*文件历史：
*			版本号	  创建日期		作者
*			 v0.1	 2012/11/03	   TianHei
*联系方式：Qq:763146170  Email：763146170@qq.com
* 说    明：部分程序来自网络
****************************************************************************/

#include "include.h"
#include "adxl.h"
#include "I2C.h"
#include <stdio.h>
#include <math.h>


/*
 *简述 ADCL345初始化配置
 *参数 无
 *返回 无
*/
void ADXL345_Init(void)
{
	 /*写0禁止单双击功能*/
	 Single_Write(ADXL345_Addr,DUR,0);

	 /*直流耦合方式不参与禁止活动检测*/
	 Single_Write(ADXL345_Addr,ACT_INACT_CTL,0);

	 /*不进行敲击检测*/
	 Single_Write(ADXL345_Addr,TAP_AXES,0);

	 /*正常工作模式，转换速率为100hz输出数据速率，默认为0x0a*/
	 Single_Write(ADXL345_Addr,BW_RATE,0x0A);

	 /*测量模式，不休眠*/
	 Single_Write(ADXL345_Addr,POWER_CTL,0x08);

	 /*禁止所有中断*/
	 Single_Write(ADXL345_Addr,INT_ENABLE,0);

	 /*配置中断引脚映射，为0则在中断0引脚上*/
	 Single_Write(ADXL345_Addr,INT_MAP,0);

	 /*禁止自测力，4线SPI模式，中断高电平有效，全分辨率模式，数据右对齐，+-16g范围*/
	 Single_Write(ADXL345_Addr,DATA_FORMAT,0x0B);

	 /*旁路FIFO*/
	 Single_Write(ADXL345_Addr,FIFO_CTL,0);
}

/*
 *简述 ADCL345校准
 *参数 无
 *返回 无
*/
void ADXL345_Calibrate(void)
{
	 uint8_t i;
   ADXL345_TYPE tmp_adxl345;
   int32_t ax,ay,az;
    
//   Delayms(2);
//   Single_Write(ADXL345_Addr,0x31,0x0B);   //测量范围,正负16g，13位模式
//   Single_Write(ADXL345_Addr,0x2C,0x0e);   //速率设定为100hz 参考pdf13页
//   Single_Write(ADXL345_Addr,0x2D,0x08);   //选择电源模式   参考pdf24页
//   Single_Write(ADXL345_Addr,0x2E,0x80);   //使能 DATA_READY 中断
//   Single_Write(ADXL345_Addr,0x1E,0);      //先将补偿参数清零,然后再校正
//   Single_Write(ADXL345_Addr,0x1F,0);
//   Single_Write(ADXL345_Addr,0x20,0);
    
   ax = ay = az = 0;
   for(i=0; i<100; i++)
   {
//      Delayms(12);
      ADXL345_Read(&tmp_adxl345);
      ax += tmp_adxl345.ax;
      ay += tmp_adxl345.ay;
      az += tmp_adxl345.az;
   }
   ax = -(ax/400);
   ay = -(ay/400);
   az = -(az/100 -256)/4;
   //if(abs(ax) > 255)ax=0;
   //if(abs(ay) > 255)ay=0;
   //if(abs(az) > 255)az=0;
   //修正偏移值
   Single_Write(ADXL345_Addr,0x1E,(uint8_t)ax);
   Single_Write(ADXL345_Addr,0x1F,(uint8_t)ay);
   Single_Write(ADXL345_Addr,0x20,(uint8_t)az);
}

/*
 *简述 读取ADCL345数据
 *参数 ptResult：指向存放数据的指针
 *返回 无
*/
void ADXL345_Read(ADXL345_TYPE* ptResult)
{
    uint8_t tmp[6];
    
    tmp[0]=Single_Read(ADXL345_Addr,0x32);//OUT_X_L_A
    tmp[1]=Single_Read(ADXL345_Addr,0x33);//OUT_X_H_A
    
    tmp[2]=Single_Read(ADXL345_Addr,0x34);//OUT_Y_L_A
    tmp[3]=Single_Read(ADXL345_Addr,0x35);//OUT_Y_H_A
    
    tmp[4]=Single_Read(ADXL345_Addr,0x36);//OUT_Z_L_A
    tmp[5]=Single_Read(ADXL345_Addr,0x37);//OUT_Z_H_A
    
    ptResult->ax    = (int16_t)((tmp[1]<<8)+tmp[0]);  //合成数据
    ptResult->ay    = (int16_t)((tmp[3]<<8)+tmp[2]);
    ptResult->az    = (int16_t)((tmp[5]<<8)+tmp[4]);
}

/*
 *简述 读取ADCL345数据，采用连续读取的方式
 *参数 ptResult：指向存放数据的指针
 *返回 无
*/
void ADXL345_MultRead(ADXL345_TYPE* ptResult)
{
    uint8_t tmp[6];
    
		I2C_ReadData(ADXL345_Addr, 0x32, tmp, 6);
//    if(true == Mult_Read(ADXL345_Addr,0x32,tmp,6))
//    {   //ptResult->ax    = (int16_t)((tmp[1]<<8)+tmp[0]);  //合成数据
        //ptResult->ay    = (int16_t)((tmp[3]<<8)+tmp[2]);
        //ptResult->az    = (int16_t)((tmp[5]<<8)+tmp[4]);
        ptResult->ax      = *( (int16_t *)(&tmp[0]) );      //优化效果明显
        ptResult->ay      = *( (int16_t *)(&tmp[2]) );
        ptResult->az      = *( (int16_t *)(&tmp[4]) );
//    }
}

/*
 *简述 打印读回来的数据
 *参数 ptResult：指向存放数据的指针
 *返回 无
*/
void ADXL345_Printf(ADXL345_TYPE* ptResult)
{
    float tempX,tempY,tempZ;
		float roll,pitch,yaw;
    
    //temp=(float)dis_data*3.9;  //计算数据和显示,查考ADXL345快速入门第4页
    tempX = (float)ptResult->ax * 0.0039;
    tempY = (float)ptResult->ay * 0.0039;
    tempZ = (float)ptResult->az * 0.0039;
//    printf("ADXL345:\tax: %.3fg,\tay: %.3fg,\tsz: %.3fg\n\r",tempX,tempY,tempZ);
	
		roll =  (float)(((atan2(tempZ,tempX)*180)/3.1416)-90); //x轴角度
		pitch = (float)(((atan2(tempZ,tempY)*180)/3.1416)-90); //y轴角度
		yaw =   (float)((atan2(tempX,tempY)*180)/3.1416);      //Z轴角度
		printf("ADXL345:\tax: %.3fg,\tay: %.3fg,\tsz: %.3fg\n\r",roll,pitch,yaw);
}




#if 0
/*
 *简述 ADCL345初始化配置
 *参数 无
 *返回 无
*/
void ADXL345_Init(void)
{
	u8 reg_addr = 0;
	u8 byData[13] = {0};
	
  //数据通信格式;设置为自检功能禁用,4线制SPI接口,低电平中断输出,13位全分辨率,输出数据右对齐,16g量程
  reg_addr = 0x31;
  byData[0] = 0x2B;
	
  I2C_WriteData(IIC_ACC_WR, reg_addr, byData, 1);			//0x31寄存器开始设置	 DATA_FORMAT

  reg_addr = 0x1E;

  byData[0] = 0x00;  //X轴误差补偿; (15.6mg/LSB)
  byData[1] = 0x00;  //Y轴误差补偿; (15.6mg/LSB)
  byData[2] = 0x00;  //Z轴误差补偿; (15.6mg/LSB)
  byData[3] = 0x00;  //敲击延时0:禁用; (1.25ms/LSB)
  byData[4] = 0x00;  //检测第一次敲击后的延时0:禁用; (1.25ms/LSB)
  byData[5] = 0x00;  //敲击窗口0:禁用; (1.25ms/LSB)
  byData[6] = 0x01;  //保存检测活动阀值; (62.5mg/LSB)
  byData[7] = 0x01;  //保存检测静止阀值; (62.5mg/LSB)
  byData[8] = 0x2B;  //检测活动时间阀值; (1s/LSB)
  byData[9] = 0x00;
  byData[10] = 0x09;   //自由落体检测推荐阀值; (62.5mg/LSB)
  byData[11] =  0xFF;  //自由落体检测时间阀值,设置为最大时间; (5ms/LSB)
  byData[12] =  0x80;
  I2C_WriteData(IIC_ACC_WR, reg_addr, byData, 13);
														//0x1E
  reg_addr = 0x2C;

  byData[0] = 0x0A;
  byData[1] = 0x28;  //开启Link,测量功能;关闭自动休眠,休眠,唤醒功能
  byData[2] = 0x00;  //所有均关闭
  byData[3] = 0x00;  //中断功能设定,不使用中断
  I2C_WriteData(IIC_ACC_WR, reg_addr, byData, 4);
    
  //FIFO模式设定,Stream模式，触发连接INT1,31级样本缓冲
  reg_addr = 0x38;
  byData[0] = 0x9F;
  I2C_WriteData(IIC_ACC_WR, reg_addr, byData, 1);
}

/*
 *简述 读ADCL345
 *参数 pwGyroX：存放X轴数据地址
 *参数 pwGyroY：存放Y轴数据地址
 *参数 pwGyroZ：存放Y轴数据地址
 *返回 无
*/
void ADXL345_ReadData(s16 * pwGyroX, s16 * pwGyroY, s16 * pwGyroZ)
{
	u8 reg_addr;
  u8 byData[6];
  s16 wTemp;

  reg_addr = 0x32; //数据存放起始地址
    
  I2C_ReadData(IIC_ACC_WR, reg_addr, byData, 6); //器件地址，数据寄存器地址，读出存放，个数

  wTemp  = 0;
  wTemp = byData[1] << 8;		//数据先低后高输出
  wTemp |= byData[0];
  *pwGyroX = wTemp;

  wTemp  = 0;
  wTemp = byData[3] << 8;
  wTemp |= byData[2];
  *pwGyroY = wTemp;
    
  wTemp  = 0;
  wTemp = byData[5] << 8;
  wTemp |= byData[4];
  *pwGyroZ = wTemp;
}

#endif
