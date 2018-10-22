#ifndef MEW_DISPLAY_CONFIG
#define MEW_DISPLAY_CONFIG

#ifdef __MEW_CONFIG__TS_USE_TSC2007
#ifdef __MEW_CONFIG__TS_USE_ADC
#error "Conflict: touchscreen may be tsc2007 or adc"
#endif
#endif

#include "../config.h"

#include "../lv_conf.h"
#include "../lvgl/lvgl.h"

void mew_display_init(void);

#define MEW_DISPLAY_W                       240
#define MEW_DISPLAY_H                       320
#define MEW_DISPLAY_BUFFER_SIZE             (MEW_DISPLAY_W * MEW_DISPLAY_H)

#define MEW_DISPLAY_DMA                     DMA1
#define MEW_DISPLAY_DMA_HANDLE              dma1_stream4_isr
#define MEW_DISPLAY_SPI_I2SCFGR             SPI2_I2SCFGR
#define MEW_DISPLAY_DMA_STREAM              DMA_STREAM4
#define MEW_DISPLAY_DMA_CHANNEL             DMA_SxCR_CHSEL_0
#define MEW_DISPLAY_DMA_DR                  SPI2_DR
#define MEW_DISPLAY_DMA_NVIC                NVIC_DMA1_STREAM4_IRQ

#define MEW_DISPLAY_SPI                     SPI2
#define MEW_DISPLAY_SPI_GPIO_PINS           GPIO15 | GPIO13 | GPIO14
#define MEW_DISPLAY_SPI_GPIO_PORT           GPIOB
#define MEW_DISPLAY_SPI_GPIO_AF_NUMBER      GPIO_AF5

#define MEW_DISPLAY_BL_PIN                  GPIO11
#define MEW_DISPLAY_BL_PORT                 GPIOC

#define MEW_DISPLAY_RESET_PIN               GPIO12
#define MEW_DISPLAY_RESET_PORT              GPIOB

#define MEW_DISPLAY_DC_PIN                  GPIO11
#define MEW_DISPLAY_DC_PORT                 GPIOB

#define MEW_DISPLAY_CS_PIN                  GPIO10
#define MEW_DISPLAY_CS_PORT                 GPIOB

#ifdef __MEW_CONFIG__TS_USE_TSC2007

#define MEW_TSC2007_IRQ_PIN                 GPIO2
#define MEW_TSC2007_IRQ_PORT                GPIOC
#define MEW_TSC2007_IRQ_ISR                 exti2_isr
#define MEW_TSC2007_IRQ_EXTI                EXTI2
#define MEW_TSC2007_IRQ_NVIC                NVIC_EXTI2_IRQ

#define MEW_TSC2007_MEASURE_TEMP0           (0x0 << 4)
#define MEW_TSC2007_MEASURE_AUX             (0x2 << 4)
#define MEW_TSC2007_MEASURE_TEMP1           (0x4 << 4)
#define MEW_TSC2007_ACTIVATE_XN             (0x8 << 4)
#define MEW_TSC2007_ACTIVATE_YN             (0x9 << 4)
#define MEW_TSC2007_ACTIVATE_YP_XN          (0xa << 4)
#define MEW_TSC2007_SETUP                   (0xb << 4)
#define MEW_TSC2007_MEASURE_X               (0xc << 4)
#define MEW_TSC2007_MEASURE_Y               (0xd << 4)
#define MEW_TSC2007_MEASURE_Z1              (0xe << 4)
#define MEW_TSC2007_MEASURE_Z2              (0xf << 4)

#define MEW_TSC2007_POWER_OFF_IRQ_EN        (0x0 << 2)
#define MEW_TSC2007_ADC_ON_IRQ_DIS0         (0x1 << 2)
#define MEW_TSC2007_ADC_OFF_IRQ_EN          (0x2 << 2)
#define MEW_TSC2007_ADC_ON_IRQ_DIS1         (0x3 << 2)

#define MEW_TSC2007_12BIT                   (0x0 << 1)
#define MEW_TSC2007_8BIT                    (0x1 << 1)

#define	MEW_TSC2007_MAX_12BIT               ((1 << 12) - 1)

#define MEW_TSC2007_ADC_ON_12BIT	(MEW_TSC2007_12BIT | MEW_TSC2007_ADC_ON_IRQ_DIS0)

#define MEW_TSC2007_READ_Y		(MEW_TSC2007_ADC_ON_12BIT  | MEW_TSC2007_MEASURE_Y)
#define MEW_TSC2007_READ_Z1		(MEW_TSC2007_ADC_ON_12BIT  | MEW_TSC2007_MEASURE_Z1)
#define MEW_TSC2007_READ_Z2		(MEW_TSC2007_ADC_ON_12BIT  | MEW_TSC2007_MEASURE_Z2)
#define MEW_TSC2007_READ_X		(MEW_TSC2007_ADC_ON_12BIT  | MEW_TSC2007_MEASURE_X)
#define MEW_TSC2007_PWRDOWN		(MEW_TSC2007_12BIT | MEW_TSC2007_POWER_OFF_IRQ_EN)

void mew_touchscreen_init(void);

#endif

#ifdef __MEW_CONFIG__TS_USE_ADC
#define TS_ADC_BUF_SIZE                 24

#define TS_ADC                          ADC1

#define TS_XP_PIN                       GPIO4
#define TS_XP_PORT                      GPIOC
#define TS_XP_ADC_CHANNEL               14

#define TS_DETECT_NVIC                  NVIC_EXTI1_IRQ
#define TS_DETECT_EXTI                  EXTI1
#define TS_IRQ_HANDLE                   exti1_isr

#define TS_XN_PIN                       GPIO0
#define TS_XN_PORT                      GPIOB

#define TS_YP_PIN                       GPIO5
#define TS_YP_PORT                      GPIOC
#define TS_YP_ADC_CHANNEL               15

#define TS_YN_PIN                       GPIO1
#define TS_YN_PORT                      GPIOB
#endif

#endif
