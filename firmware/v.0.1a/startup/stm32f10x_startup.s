
;***************************************************************************************
; Amount of memory (in bytes) allocated for Stack and Heap
; Tailor those values to your application needs          
;***************************************************************************************
Stack_Size   EQU     0x400
Heap_Size    EQU     0x400

;*******************************************************************************
; Allocate space for the Stack
;*******************************************************************************
  AREA    STACK, NOINIT, READWRITE, ALIGN=3

Stack
    SPACE   Stack_Size

;*******************************************************************************
; Allocate space for the Heap
;*******************************************************************************
  AREA    HEAP, NOINIT, READWRITE, ALIGN=3

Heap
    SPACE   Heap_Size

;********************************************************************************
;*  Declarations for the interrupt handlers that are used by the application.                                                                     
;********************************************************************************
  		IMPORT  __main

  		IMPORT  OSPendSV
  		IMPORT  SysTickHandler
  		IMPORT  WWDG_IRQHandler
  		IMPORT  PVD_IRQHandler
  		IMPORT  TAMPER_IRQHandler
  		IMPORT  RTC_IRQHandler
  		IMPORT  FLASH_IRQHandler
  		IMPORT  RCC_IRQHandler
  		IMPORT  EXTI0_IRQHandler
  		IMPORT  EXTI1_IRQHandler
  		IMPORT  EXTI2_IRQHandler
  		IMPORT  EXTI3_IRQHandler
  		IMPORT  EXTI4_IRQHandler
  		IMPORT  DMAChannel1_IRQHandler
  		IMPORT  DMAChannel2_IRQHandler
  		IMPORT  DMAChannel3_IRQHandler
  		IMPORT  DMAChannel4_IRQHandler
  		IMPORT  DMAChannel5_IRQHandler
  		IMPORT  DMAChannel6_IRQHandler
  		IMPORT  DMAChannel7_IRQHandler
  		IMPORT  ADC_IRQHandler
  		IMPORT  USB_HP_CAN_TX_IRQHandler
  		IMPORT  USB_LP_CAN_RX0_IRQHandler
  		IMPORT  CAN_RX1_IRQHandler
  		IMPORT  CAN_SCE_IRQHandler
  		IMPORT  EXTI9_5_IRQHandler
  		IMPORT  TIM1_BRK_IRQHandler
  		IMPORT  TIM1_UP_IRQHandler
  		IMPORT  TIM1_TRG_COM_IRQHandler
  		IMPORT  TIM1_CC_IRQHandler
  		IMPORT  TIM2_IRQHandler
  		IMPORT  TIM3_IRQHandler
  		IMPORT  TIM4_IRQHandler
  		IMPORT  I2C1_EV_IRQHandler
  		IMPORT  I2C1_ER_IRQHandler
  		IMPORT  I2C2_EV_IRQHandler
  		IMPORT  I2C2_ER_IRQHandler
  		IMPORT  SPI1_IRQHandler
  		IMPORT  SPI2_IRQHandler
  		IMPORT  USART1_IRQHandler
  		IMPORT  USART2_IRQHandler
  		IMPORT  USART3_IRQHandler
  		IMPORT  EXTI15_10_IRQHandler
  		IMPORT  RTCAlarm_IRQHandler
  		IMPORT  USBWakeUp_IRQHandler

  
          
		PRESERVE8

;**********************************************************************************
;*  Reset code section.                                                                                                           
;**********************************************************************************
        AREA    RESET, CODE, READONLY
        THUMB

;*******************************************************************************
; Fill-up the Vector Table entries with the exceptions ISR address
;*******************************************************************************
    	EXPORT  __Vectors
__Vectors                      
    	DCD  Stack + Stack_Size            ; Top of Stack
    	DCD  Reset_Handler
    	DCD  NMIException
    	DCD  HardFaultException
    	DCD  MemManageException
    	DCD  BusFaultException
    	DCD  UsageFaultException
    	DCD  0                 ; Reserved
    	DCD  0                 ; Reserved
    	DCD  0                 ; Reserved
    	DCD  0                 ; Reserved
    	DCD  SVCHandler
    	DCD  DebugMonitor
    	DCD  0                 ; Reserved
    	DCD  OSPendSV
    	DCD  SysTickHandler
    	DCD  WWDG_IRQHandler
    	DCD  PVD_IRQHandler
    	DCD  TAMPER_IRQHandler
    	DCD  RTC_IRQHandler
    	DCD  FLASH_IRQHandler
    	DCD  RCC_IRQHandler
    	DCD  EXTI0_IRQHandler
    	DCD  EXTI1_IRQHandler
    	DCD  EXTI2_IRQHandler
    	DCD  EXTI3_IRQHandler
    	DCD  EXTI4_IRQHandler
    	DCD  DMAChannel1_IRQHandler
    	DCD  DMAChannel2_IRQHandler
    	DCD  DMAChannel3_IRQHandler
    	DCD  DMAChannel4_IRQHandler
    	DCD  DMAChannel5_IRQHandler
    	DCD  DMAChannel6_IRQHandler
    	DCD  DMAChannel7_IRQHandler
    	DCD  ADC_IRQHandler
    	DCD  USB_HP_CAN_TX_IRQHandler
    	DCD  USB_LP_CAN_RX0_IRQHandler
    	DCD  CAN_RX1_IRQHandler
    	DCD  CAN_SCE_IRQHandler
    	DCD  EXTI9_5_IRQHandler
    	DCD  TIM1_BRK_IRQHandler
    	DCD  TIM1_UP_IRQHandler
    	DCD  TIM1_TRG_COM_IRQHandler
    	DCD  TIM1_CC_IRQHandler
    	DCD  TIM2_IRQHandler
    	DCD  TIM3_IRQHandler
    	DCD  TIM4_IRQHandler
    	DCD  I2C1_EV_IRQHandler
    	DCD  I2C1_ER_IRQHandler
    	DCD  I2C2_EV_IRQHandler
    	DCD  I2C2_ER_IRQHandler
    	DCD  SPI1_IRQHandler
    	DCD  SPI2_IRQHandler
    	DCD  USART1_IRQHandler
    	DCD  USART2_IRQHandler
    	DCD  USART3_IRQHandler
    	DCD  EXTI15_10_IRQHandler
    	DCD  RTCAlarm_IRQHandler
    	DCD  USBWakeUp_IRQHandler 
    	DCD  0                 ; Reserved
    	DCD  0                 ; Reserved
    	DCD  0                 ; Reserved
    	DCD  0                 ; Reserved
    	DCD  0                 ; Reserved

;******************************************************************************************
;*  Reset entry
;******************************************************************************************
        EXPORT  Reset_Handler
Reset_Handler
        IMPORT  __main
        LDR     R0, =__main
        BX      R0


;******************************************************************************************
;*  NMI exception handler. 
;*  It simply enters an infinite loop.
;******************************************************************************************
NMIException
        B       NMIException


;******************************************************************************************
;*  Fault interrupt handler. 
;*  It simply enters an infinite loop.
;******************************************************************************************
HardFaultException
        B       HardFaultException

;******************************************************************************************
;*  MemManage interrupt handler. 
;*  It simply enters an infinite loop.
;******************************************************************************************
MemManageException
        B       MemManageException

;******************************************************************************************
;*  Bus Fault interrupt handler. 
;*  It simply enters an infinite loop.
;******************************************************************************************
BusFaultException
        B       BusFaultException

;******************************************************************************************
;*  UsageFault interrupt handler. 
;*  It simply enters an infinite loop.
;******************************************************************************************
UsageFaultException
        B       UsageFaultException

;******************************************************************************************
;*  DebugMonitor interrupt handler. 
;*  It simply enters an infinite loop.
;******************************************************************************************
DebugMonitor
        B       DebugMonitor

;******************************************************************************************
;*  SVCall interrupt handler. 
;*  It simply enters an infinite loop.
;******************************************************************************************
SVCHandler
        B       SVCHandler



;*******************************************************************************************
;*  Make sure the end of this section is aligned.
;*******************************************************************************************
        ALIGN


;********************************************************************************************
;*  Code section for initializing the heap and stack                                                                                                          
;********************************************************************************************
		AREA    |.text|, CODE, READONLY


;********************************************************************************************
;*  The function expected of the C library startup 
;*  code for defining the stack and heap memory locations. 
;********************************************************************************************
        IMPORT  __use_two_region_memory
        EXPORT  __user_initial_stackheap 
__user_initial_stackheap
        LDR     R0, =Heap
        LDR     R1, =(Stack + Stack_Size)
        LDR     R2, =(Heap + Heap_Size)
        LDR     R3, =Stack
        BX      LR

;******************************************************************************************
;*  Make sure the end of this section is aligned.
;******************************************************************************************
        ALIGN


;*******************************************************************************************
;*  End Of File                                                     
;*******************************************************************************************
        END

