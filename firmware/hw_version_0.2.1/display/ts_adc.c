#include "display.h" 
#include "../debug/debug.h"
#ifdef __MEW_CONFIG__TS_USE_ADC

//static void __mew_gpio_as_input_od(uint32_t port, uint16_t gpio);
static void __mew_gpio_as_input_pullup(uint32_t port, uint16_t gpio);
static void __mew_gpio_as_input_pulldown(uint32_t port, uint16_t gpio);
static void __mew_gpio_as_output_low(uint32_t port, uint16_t gpio);
static void __mew_gpio_as_output_high(uint32_t port, uint16_t gpio);
static void __mew_gpio_as_analog(uint32_t port, uint16_t gpio);

static void __mew_adc_prepare(const uint8_t channel);
static uint32_t __mew_adc_get(void);
static uint32_t __mew_adc_get_x(void);
static uint32_t __mew_adc_get_y(void);
static uint16_t __mew_adc_is_pressed(void);
static void __mew_adc_disable(void);

static bool __mew_input_read(lv_indev_data_t *data);
static int __mew_compare_int32(const void* a, const void* b);
static void __mew_wait(uint32_t delay);

static uint16_t _mew_last_x = 0;
static uint16_t _mew_last_y = 0;

/*static void __mew_gpio_as_input_od(uint32_t port, uint16_t gpio) {
    gpio_mode_setup(port, GPIO_MODE_INPUT, GPIO_PUPD_NONE, gpio);
    gpio_set(port, gpio);
}*/

static void __mew_wait(uint32_t delay) {
    uint32_t i;
    for (i=0; i<delay; i++) __asm__("NOP");
}

static void __mew_gpio_as_input_pullup(uint32_t port, uint16_t gpio) {
    gpio_mode_setup(port, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, gpio);
    gpio_set(port, gpio);
}

static void __mew_gpio_as_input_pulldown(uint32_t port, uint16_t gpio) {
    gpio_mode_setup(port, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, gpio);
    gpio_set(port, gpio);
}

static void __mew_gpio_as_output_low(uint32_t port, uint16_t gpio) {
    gpio_mode_setup(port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, gpio);
    gpio_clear(port, gpio);
}

static void __mew_gpio_as_output_high(uint32_t port, uint16_t gpio) {
    gpio_mode_setup(port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, gpio);
    gpio_set(port, gpio);
}

static void __mew_gpio_as_analog(uint32_t port, uint16_t gpio) {
    gpio_mode_setup(port, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, gpio);
}

static void __mew_adc_prepare(const uint8_t channel) {
    adc_power_off(TS_ADC);
    adc_set_clk_prescale(ADC_CCR_ADCPRE_BY8);
    adc_disable_scan_mode(TS_ADC);
    adc_set_single_conversion_mode(TS_ADC);
    adc_set_resolution(TS_ADC, ADC_CR1_RES_12BIT);
    adc_set_sample_time(TS_ADC, channel, ADC_SMPR_SMP_480CYC);
    adc_set_regular_sequence(TS_ADC, 1, (uint8_t[]) { channel });
    adc_set_multi_mode(ADC_CCR_MULTI_INDEPENDENT);
    adc_power_on(TS_ADC);
}

static int __mew_compare_int32( const void* a, const void* b ) {
    if( *(uint32_t*)a == *(uint32_t*)b ) return 0;
    return *(uint32_t*)a < *(uint32_t*)b ? -1 : 1;
}

static uint32_t __mew_adc_get(void) {
    uint32_t i, acc = 0;
    uint32_t value[TS_ADC_BUF_SIZE];
    
    for (i=0; i<TS_ADC_BUF_SIZE; i++) {
        adc_start_conversion_regular(TS_ADC);
        while (!adc_eoc(TS_ADC));
        value[i] = adc_read_regular(TS_ADC);
    }

    qsort(value, TS_ADC_BUF_SIZE, sizeof(uint32_t), __mew_compare_int32);

    for (i=4; i<(TS_ADC_BUF_SIZE-4); i++) {
    	acc += value[i];
    }
    
    return acc / (TS_ADC_BUF_SIZE - 8);
}

static uint32_t __mew_adc_get_x(void) {   
    __mew_gpio_as_analog(TS_YP_PORT, TS_YP_PIN);
    __mew_gpio_as_input_pulldown(TS_YN_PORT, TS_YN_PIN);
    __mew_gpio_as_output_high(TS_XN_PORT, TS_XN_PIN);
    __mew_gpio_as_output_low(TS_XP_PORT, TS_XP_PIN);
    __mew_adc_prepare(15);
    return __mew_adc_get();
}

static uint32_t __mew_adc_get_y(void) {   
    __mew_gpio_as_analog(TS_XP_PORT, TS_XP_PIN);
    __mew_gpio_as_input_pulldown(TS_XN_PORT, TS_XN_PIN);
    __mew_gpio_as_output_high(TS_YN_PORT, TS_YN_PIN);
    __mew_gpio_as_output_low(TS_YP_PORT, TS_YP_PIN);
    __mew_adc_prepare(14);
    return __mew_adc_get();
}

static uint16_t __mew_adc_is_pressed(void) {
    __mew_gpio_as_input_pullup(TS_XP_PORT, TS_XP_PIN);
    __mew_gpio_as_input_pullup(TS_XN_PORT, TS_XN_PIN);
    __mew_gpio_as_output_low(TS_YP_PORT, TS_YP_PIN);
    __mew_gpio_as_output_low(TS_YN_PORT, TS_YN_PIN);

    return gpio_get(TS_XP_PORT, TS_XP_PIN);
}

static void __mew_adc_disable(void) {
    __mew_gpio_as_input_pulldown(TS_XP_PORT, TS_XP_PIN);
    __mew_gpio_as_input_pulldown(TS_XN_PORT, TS_XN_PIN);
    __mew_gpio_as_input_pulldown(TS_YP_PORT, TS_YP_PIN);
    __mew_gpio_as_input_pulldown(TS_YN_PORT, TS_YN_PIN);
    adc_power_off(TS_ADC);
}

void mew_touchscreen_init(void) {
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);     /*Basic initialization*/
	indev_drv.type = LV_INDEV_TYPE_POINTER;              /*See below.*/
	indev_drv.read = __mew_input_read;               /*See below.*/
	lv_indev_drv_register(&indev_drv);     /*Register the driver in LittlevGL*/
}

static bool __mew_input_read(lv_indev_data_t *data) {
	float x_raw, y_raw, x_disp, y_disp;
	uint32_t pressed = __mew_adc_is_pressed();

	if (pressed == 0) {
		x_raw = __mew_adc_get_x();
		y_raw = __mew_adc_get_y();
		__mew_adc_disable();

		x_disp = (MEW_DISPLAY_W * (x_raw / (0x1000 - 500))) - 20;
		y_disp = (MEW_DISPLAY_H * (y_raw / (0x1000 - 700))) - 20;

		data->point.x = (lv_coord_t) ((x_disp < 0) ? 0 : x_disp);
		data->point.y = (lv_coord_t) ((y_disp < 0) ? 0 : y_disp);

		data->state = LV_INDEV_STATE_PR;
		_mew_last_x = data->point.x;
		_mew_last_y = data->point.y;
	}

	__mew_wait(500);
	pressed = __mew_adc_is_pressed();
	if (pressed != 0) {
		data->state = LV_INDEV_STATE_REL;
		data->point.x = (lv_coord_t) _mew_last_x;
		data->point.y = (lv_coord_t) _mew_last_y;
	}

	__mew_adc_disable();
	return false;
}
#endif
