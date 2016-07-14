/****************************************************************************
* 文件名：Timer3.c
* 内容简述: 
*			      定时器3初始化及相关函数定义
*			      初始化的三个步骤原则：
*			                           1、先使能相关时钟，2、配置外设 3、要用中断则配置中断控制器
*文件历史：
*			版本号	  创建日期		作者
*			 v0.1	 2012/11/10	   TianHei
*联系方式：Qq:763146170  Email：763146170@qq.com
* 说    明：
****************************************************************************/


#include "include.h"
#include "Timer3.h"
#include "stm32f10x_tim.h"

/*
 *简述 定时器3的基本初始化
 *参数 无
 *返回 无
*/
static __INLINE void TIM3_BaseInit(void)
{
	TIM_TimeBaseInitTypeDef  TIM3_TimeBaseStructure;

	//预分频器TIM3_PSC=72（对内部时钟的分频系数）
	TIM3_TimeBaseStructure.TIM_Prescaler = 7200;
	//计数器向上计数模式 TIM3_CR1[4]=0（记数方式）	 
	TIM3_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	//自动重装载寄存器TIM3_APR  定时15MS中断一次
	TIM3_TimeBaseStructure.TIM_Period = 1000;
	//时钟分频因子 TIM3_CR1[9:8]=00（对外部来的时钟进行的分频设置） 		     
	TIM3_TimeBaseStructure.TIM_ClockDivision = 0x0;
	//(重复记数次数，存在与高级定时器的使用)
	TIM3_TimeBaseStructure.TIM_RepetitionCounter = 0x0;

	TIM_TimeBaseInit(TIM3,&TIM3_TimeBaseStructure);	//写TIM3各寄存器参数

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);//使能定时器的更新事件中断
}

/*
 *简述 定时器3的初始化
 *参数 无
 *返回 无
*/
void TIM3_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//先使能时钟

	TIM3_BaseInit();

	TIM_ClearFlag(TIM3, TIM_IT_Update);//更新中断标志由软件清零

	TIM_Cmd(TIM3,ENABLE);//启动定时器3 TIM3_CR1[0]=1;
}







