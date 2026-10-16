;*******************************************************************************
;* 文件名 : startup_stm32f103c6_keil.s
;* 说明   : STM32F103C6 (Cortex-M3) 启动文件 —— Keil (ARM 汇编) 语法版本
;*
;*  内容: 1) 栈空间
;*        2) 中断向量表(RESET 段会被链接器放在 Flash 最前面 0x08000000)
;*        3) 复位处理: 调用 SystemInit(), 再跳到 C 库入口 __main(由它清零/搬移
;*           数据段并调用 main)
;*        4) 所有中断的默认处理(弱定义, 可以在 C 文件里重写)
;*
;*  本文件不依赖 ST 标准外设库, 与 stm32f10x.h / 库文件都无关。
;*  如果使用 gcc/zig 编译, 请改用 gcc 目录里的 startup_stm32f103c6.s。
;*******************************************************************************

Stack_Size      EQU     0x00000400              ; 1KB 栈空间

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp

;-------------------------------------------------------------------------------
; 中断向量表
;-------------------------------------------------------------------------------
                PRESERVE8
                THUMB

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp               ; 0x00 栈顶指针
                DCD     Reset_Handler              ; 0x04 复位
                DCD     NMI_Handler                ; 0x08
                DCD     HardFault_Handler          ; 0x0C
                DCD     MemManage_Handler          ; 0x10
                DCD     BusFault_Handler           ; 0x14
                DCD     UsageFault_Handler         ; 0x18
                DCD     0                          ; 0x1C
                DCD     0                          ; 0x20
                DCD     0                          ; 0x24
                DCD     0                          ; 0x28
                DCD     SVC_Handler                ; 0x2C
                DCD     DebugMon_Handler           ; 0x30
                DCD     0                          ; 0x34
                DCD     PendSV_Handler             ; 0x38
                DCD     SysTick_Handler            ; 0x3C  (在 delay.c 里实现)

                ; ---- 外部中断 0 ~ 59 ----
                DCD     WWDG_IRQHandler            ; 0
                DCD     PVD_IRQHandler             ; 1
                DCD     TAMPER_IRQHandler          ; 2
                DCD     RTC_IRQHandler             ; 3
                DCD     FLASH_IRQHandler           ; 4
                DCD     RCC_IRQHandler             ; 5
                DCD     EXTI0_IRQHandler           ; 6
                DCD     EXTI1_IRQHandler           ; 7
                DCD     EXTI2_IRQHandler           ; 8
                DCD     EXTI3_IRQHandler           ; 9
                DCD     EXTI4_IRQHandler           ; 10
                DCD     DMA1_Channel1_IRQHandler   ; 11
                DCD     DMA1_Channel2_IRQHandler   ; 12
                DCD     DMA1_Channel3_IRQHandler   ; 13
                DCD     DMA1_Channel4_IRQHandler   ; 14
                DCD     DMA1_Channel5_IRQHandler   ; 15
                DCD     DMA1_Channel6_IRQHandler   ; 16
                DCD     DMA1_Channel7_IRQHandler   ; 17
                DCD     ADC1_2_IRQHandler          ; 18
                DCD     USB_HP_CAN1_TX_IRQHandler  ; 19
                DCD     USB_LP_CAN1_RX0_IRQHandler ; 20
                DCD     CAN1_RX1_IRQHandler        ; 21
                DCD     CAN1_SCE_IRQHandler        ; 22
                DCD     EXTI9_5_IRQHandler         ; 23
                DCD     TIM1_BRK_IRQHandler        ; 24
                DCD     TIM1_UP_IRQHandler         ; 25
                DCD     TIM1_TRG_COM_IRQHandler    ; 26
                DCD     TIM1_CC_IRQHandler         ; 27
                DCD     TIM2_IRQHandler            ; 28
                DCD     TIM3_IRQHandler            ; 29
                DCD     TIM4_IRQHandler            ; 30
                DCD     I2C1_EV_IRQHandler         ; 31
                DCD     I2C1_ER_IRQHandler         ; 32
                DCD     I2C2_EV_IRQHandler         ; 33
                DCD     I2C2_ER_IRQHandler         ; 34
                DCD     SPI1_IRQHandler            ; 35
                DCD     SPI2_IRQHandler            ; 36
                DCD     USART1_IRQHandler          ; 37
                DCD     USART2_IRQHandler          ; 38
                DCD     USART3_IRQHandler          ; 39
                DCD     EXTI15_10_IRQHandler       ; 40
                DCD     RTCAlarm_IRQHandler        ; 41
                DCD     USBWakeUp_IRQHandler       ; 42
                DCD     TIM8_BRK_IRQHandler        ; 43
                DCD     TIM8_UP_IRQHandler         ; 44
                DCD     TIM8_TRG_COM_IRQHandler    ; 45
                DCD     TIM8_CC_IRQHandler         ; 46
                DCD     ADC3_IRQHandler            ; 47
                DCD     FSMC_IRQHandler            ; 48
                DCD     SDIO_IRQHandler            ; 49
                DCD     TIM5_IRQHandler            ; 50
                DCD     SPI3_IRQHandler            ; 51
                DCD     UART4_IRQHandler           ; 52
                DCD     UART5_IRQHandler           ; 53
                DCD     TIM6_IRQHandler            ; 54
                DCD     TIM7_IRQHandler            ; 55
                DCD     DMA2_Channel1_IRQHandler   ; 56
                DCD     DMA2_Channel2_IRQHandler   ; 57
                DCD     DMA2_Channel3_IRQHandler   ; 58
                DCD     DMA2_Channel4_5_IRQHandler ; 59
__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors

;-------------------------------------------------------------------------------
; 复位入口
;-------------------------------------------------------------------------------
                AREA    |.text|, CODE, READONLY

Reset_Handler   PROC
                EXPORT  Reset_Handler              [WEAK]
                IMPORT  SystemInit
                IMPORT  __main                     ; C 库入口: 初始化数据段后调用 main
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP

;-------------------------------------------------------------------------------
; 默认中断处理(死循环) —— 全部弱定义, 在 C 文件里写同名函数即可覆盖
;-------------------------------------------------------------------------------
Default_Handler PROC
                EXPORT  NMI_Handler                 [WEAK]
                EXPORT  HardFault_Handler           [WEAK]
                EXPORT  MemManage_Handler           [WEAK]
                EXPORT  BusFault_Handler            [WEAK]
                EXPORT  UsageFault_Handler          [WEAK]
                EXPORT  SVC_Handler                 [WEAK]
                EXPORT  DebugMon_Handler            [WEAK]
                EXPORT  PendSV_Handler              [WEAK]
                EXPORT  SysTick_Handler             [WEAK]
                EXPORT  WWDG_IRQHandler             [WEAK]
                EXPORT  PVD_IRQHandler              [WEAK]
                EXPORT  TAMPER_IRQHandler           [WEAK]
                EXPORT  RTC_IRQHandler              [WEAK]
                EXPORT  FLASH_IRQHandler            [WEAK]
                EXPORT  RCC_IRQHandler              [WEAK]
                EXPORT  EXTI0_IRQHandler            [WEAK]
                EXPORT  EXTI1_IRQHandler            [WEAK]
                EXPORT  EXTI2_IRQHandler            [WEAK]
                EXPORT  EXTI3_IRQHandler            [WEAK]
                EXPORT  EXTI4_IRQHandler            [WEAK]
                EXPORT  DMA1_Channel1_IRQHandler    [WEAK]
                EXPORT  DMA1_Channel2_IRQHandler    [WEAK]
                EXPORT  DMA1_Channel3_IRQHandler    [WEAK]
                EXPORT  DMA1_Channel4_IRQHandler    [WEAK]
                EXPORT  DMA1_Channel5_IRQHandler    [WEAK]
                EXPORT  DMA1_Channel6_IRQHandler    [WEAK]
                EXPORT  DMA1_Channel7_IRQHandler    [WEAK]
                EXPORT  ADC1_2_IRQHandler           [WEAK]
                EXPORT  USB_HP_CAN1_TX_IRQHandler   [WEAK]
                EXPORT  USB_LP_CAN1_RX0_IRQHandler  [WEAK]
                EXPORT  CAN1_RX1_IRQHandler         [WEAK]
                EXPORT  CAN1_SCE_IRQHandler         [WEAK]
                EXPORT  EXTI9_5_IRQHandler          [WEAK]
                EXPORT  TIM1_BRK_IRQHandler         [WEAK]
                EXPORT  TIM1_UP_IRQHandler          [WEAK]
                EXPORT  TIM1_TRG_COM_IRQHandler     [WEAK]
                EXPORT  TIM1_CC_IRQHandler          [WEAK]
                EXPORT  TIM2_IRQHandler             [WEAK]
                EXPORT  TIM3_IRQHandler             [WEAK]
                EXPORT  TIM4_IRQHandler             [WEAK]
                EXPORT  I2C1_EV_IRQHandler          [WEAK]
                EXPORT  I2C1_ER_IRQHandler          [WEAK]
                EXPORT  I2C2_EV_IRQHandler          [WEAK]
                EXPORT  I2C2_ER_IRQHandler          [WEAK]
                EXPORT  SPI1_IRQHandler             [WEAK]
                EXPORT  SPI2_IRQHandler             [WEAK]
                EXPORT  USART1_IRQHandler           [WEAK]
                EXPORT  USART2_IRQHandler           [WEAK]
                EXPORT  USART3_IRQHandler           [WEAK]
                EXPORT  EXTI15_10_IRQHandler        [WEAK]
                EXPORT  RTCAlarm_IRQHandler         [WEAK]
                EXPORT  USBWakeUp_IRQHandler        [WEAK]
                EXPORT  TIM8_BRK_IRQHandler         [WEAK]
                EXPORT  TIM8_UP_IRQHandler          [WEAK]
                EXPORT  TIM8_TRG_COM_IRQHandler     [WEAK]
                EXPORT  TIM8_CC_IRQHandler          [WEAK]
                EXPORT  ADC3_IRQHandler             [WEAK]
                EXPORT  FSMC_IRQHandler             [WEAK]
                EXPORT  SDIO_IRQHandler             [WEAK]
                EXPORT  TIM5_IRQHandler             [WEAK]
                EXPORT  SPI3_IRQHandler             [WEAK]
                EXPORT  UART4_IRQHandler            [WEAK]
                EXPORT  UART5_IRQHandler            [WEAK]
                EXPORT  TIM6_IRQHandler             [WEAK]
                EXPORT  TIM7_IRQHandler             [WEAK]
                EXPORT  DMA2_Channel1_IRQHandler    [WEAK]
                EXPORT  DMA2_Channel2_IRQHandler    [WEAK]
                EXPORT  DMA2_Channel3_IRQHandler    [WEAK]
                EXPORT  DMA2_Channel4_5_IRQHandler  [WEAK]

NMI_Handler
HardFault_Handler
MemManage_Handler
BusFault_Handler
UsageFault_Handler
SVC_Handler
DebugMon_Handler
PendSV_Handler
SysTick_Handler
WWDG_IRQHandler
PVD_IRQHandler
TAMPER_IRQHandler
RTC_IRQHandler
FLASH_IRQHandler
RCC_IRQHandler
EXTI0_IRQHandler
EXTI1_IRQHandler
EXTI2_IRQHandler
EXTI3_IRQHandler
EXTI4_IRQHandler
DMA1_Channel1_IRQHandler
DMA1_Channel2_IRQHandler
DMA1_Channel3_IRQHandler
DMA1_Channel4_IRQHandler
DMA1_Channel5_IRQHandler
DMA1_Channel6_IRQHandler
DMA1_Channel7_IRQHandler
ADC1_2_IRQHandler
USB_HP_CAN1_TX_IRQHandler
USB_LP_CAN1_RX0_IRQHandler
CAN1_RX1_IRQHandler
CAN1_SCE_IRQHandler
EXTI9_5_IRQHandler
TIM1_BRK_IRQHandler
TIM1_UP_IRQHandler
TIM1_TRG_COM_IRQHandler
TIM1_CC_IRQHandler
TIM2_IRQHandler
TIM3_IRQHandler
TIM4_IRQHandler
I2C1_EV_IRQHandler
I2C1_ER_IRQHandler
I2C2_EV_IRQHandler
I2C2_ER_IRQHandler
SPI1_IRQHandler
SPI2_IRQHandler
USART1_IRQHandler
USART2_IRQHandler
USART3_IRQHandler
EXTI15_10_IRQHandler
RTCAlarm_IRQHandler
USBWakeUp_IRQHandler
TIM8_BRK_IRQHandler
TIM8_UP_IRQHandler
TIM8_TRG_COM_IRQHandler
TIM8_CC_IRQHandler
ADC3_IRQHandler
FSMC_IRQHandler
SDIO_IRQHandler
TIM5_IRQHandler
SPI3_IRQHandler
UART4_IRQHandler
UART5_IRQHandler
TIM6_IRQHandler
TIM7_IRQHandler
DMA2_Channel1_IRQHandler
DMA2_Channel2_IRQHandler
DMA2_Channel3_IRQHandler
DMA2_Channel4_5_IRQHandler
                B       .                           ; 原地循环
                ENDP

                ALIGN
                END
