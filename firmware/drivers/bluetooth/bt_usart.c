#include "debug.h"
#include "bluetooth.h"
#include "../system/system.h"
#include "app/packet_parser/parser.h"

static void __mew_bluetooth_reset(void);
static void __mew_bluetooth_send_at(uint32_t cmd, char* data);
static void __mew_hex_text_to_uit8_t_array(uint8_t* result, uint8_t* chars, uint32_t len);
static uint8_t __mew_hex_text_to_int(uint8_t ch);
static void __mew_bluetooth_transmit(uint8_t* data, uint16_t size, uint8_t sync_mode);
static unsigned int __mew_bluetooth_handler(void);

//static char MEW_BT_AT__SET_NAME[] = {'A', 'T', '+', 'N', 'A', 'M', 'E', 'M', 'e', 'W', '-', 'X', '\0'};

#define MEW_BT_NAME					"MeW Pro 0.2.2"

#define MEW_BT_AT_BUFFER_SIZE 		64
#define MEW_BT_AT_BUFFER_TIME_MS	50

#define MEW_BT_STATE_AT				0x00
#define MEW_BT_STATE_TRANSPARENT	0x01

#define MEW_BT_ATCMD_ZERO			0x00
#define MEW_BT_ATCMD_GET_MAC		0x20
#define MEW_BT_ATCMD_TO_TRANSPARENT	0x21
#define MEW_BT_ATCMD_SET_NAME		0x22

#define MEW_BT_RX_STATE_BUSY		0x00
#define MEW_BT_RX_STATE_FREE		0x01

static volatile uint8_t  _mew_dma_tx_state = 0;
static volatile uint32_t _mew_bt_cmd_timer = 0;
static volatile uint32_t _mew_bt_cmd_rx_state = MEW_BT_RX_STATE_FREE;
static volatile uint32_t _mew_bt_cmd_current_cmd = MEW_BT_ATCMD_ZERO;
static volatile uint32_t _mew_bt_cmd_channel_state = MEW_BT_STATE_AT;
static volatile uint32_t _mew_bt_at_buffer_counter = 0;

uint8_t _mew_adapter_mac[6];
char _mew_bt_at_buffer[MEW_BT_AT_BUFFER_SIZE];

static unsigned int __mew_bluetooth_handler(void) {
	char bt_at_buffer[MEW_BT_AT_BUFFER_SIZE];

	if (_mew_bt_cmd_channel_state == MEW_BT_STATE_TRANSPARENT) return 1;

	if (_mew_bt_cmd_rx_state == MEW_BT_RX_STATE_BUSY) {
		memcpy(bt_at_buffer, _mew_bt_at_buffer, MEW_BT_AT_BUFFER_SIZE);

		memset(_mew_bt_at_buffer, 0, MEW_BT_AT_BUFFER_SIZE);
		_mew_bt_at_buffer_counter = 0;
		_mew_bt_cmd_timer = 0;

		_mew_bt_cmd_rx_state = MEW_BT_RX_STATE_FREE;

		switch (_mew_bt_cmd_current_cmd) {
		case MEW_BT_ATCMD_GET_MAC:
			if (strstr(bt_at_buffer, "MAC")) {
				__mew_hex_text_to_uit8_t_array((uint8_t*)_mew_adapter_mac, (uint8_t*)(bt_at_buffer + 5), 12);
				__mew_bluetooth_send_at(MEW_BT_ATCMD_TO_TRANSPARENT, "AT+HOSTEN0");
				mew_debug_print_hex_ex("BT MAC: ", (char*)(_mew_adapter_mac), 6);
			}
			break;
		case MEW_BT_ATCMD_TO_TRANSPARENT:
			if (strstr(bt_at_buffer, "OK")) {
				mew_debug_print("Bluetooth was switched to transparent mode.");
				_mew_bt_cmd_channel_state = MEW_BT_STATE_TRANSPARENT;
				return 0;
			}
			break;
		case MEW_BT_ATCMD_SET_NAME:
			if (strstr(bt_at_buffer, "OK")) {
				__mew_bluetooth_send_at(MEW_BT_ATCMD_GET_MAC, "AT+MAC");
			}
			break;
		}
	}
	return 1;
}

static void __mew_hex_text_to_uit8_t_array(uint8_t* result, uint8_t* chars, uint32_t len) {
	uint32_t i;
	for (i=0; i<len; i=i+2) {
		result[i / 2] = (__mew_hex_text_to_int(chars[i]) << 4) | __mew_hex_text_to_int(chars[i + 1]);
	}
}

static uint8_t __mew_hex_text_to_int(uint8_t ch) {
	switch (ch) {
	case '0': return 0;
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;
	case 'A': return 0xa;
	case 'B': return 0xb;
	case 'C': return 0xc;
	case 'D': return 0xd;
	case 'E': return 0xe;
	case 'F': return 0xf;
	case 'a': return 0xa;
	case 'b': return 0xb;
	case 'c': return 0xc;
	case 'd': return 0xd;
	case 'e': return 0xe;
	case 'f': return 0xf;
	}
	return 0;
}

static void __mew_bluetooth_send_at(uint32_t cmd, char* data) {
	if (_mew_bt_cmd_channel_state == MEW_BT_STATE_TRANSPARENT) return;

	uint16_t data_len = strlen(data);
	_mew_bt_cmd_current_cmd = cmd;
	__mew_bluetooth_transmit((uint8_t*) data, data_len, 1);
}

void MEW_BLUETOOTH_ISR(void) {
	uint8_t ch;
	uint32_t time;

    if (USART1_SR & USART_SR_RXNE) {
    	ch = (uint8_t) MEW_BLUETOOTH_DMA_DR;

    	switch (_mew_bt_cmd_channel_state) {
    	case MEW_BT_STATE_AT:
    		if (_mew_bt_cmd_rx_state == MEW_BT_RX_STATE_BUSY) {
    			//mew_debug_print_hex((const char*)&ch, 1);
    			if (ch == 0) __asm__("NOP"); // prevent optimization
    		} else {
    			if (ch == 0x0a) {
    				_mew_bt_cmd_rx_state = MEW_BT_RX_STATE_BUSY;
    				_mew_bt_cmd_timer = 0;
    				return;
    			}

    			time = mew_get_millis() - MEW_BT_AT_BUFFER_TIME_MS;
    			if ((time > _mew_bt_cmd_timer) && (_mew_bt_cmd_timer != 0)) {
    				_mew_bt_cmd_rx_state = MEW_BT_RX_STATE_BUSY;
    				_mew_bt_cmd_timer = 0;
    				return;
    			}

    			_mew_bt_at_buffer[_mew_bt_at_buffer_counter] = ch;
				_mew_bt_at_buffer_counter++;
				if (_mew_bt_at_buffer_counter >= MEW_BT_AT_BUFFER_SIZE) {
					memset(_mew_bt_at_buffer, 0, MEW_BT_AT_BUFFER_SIZE);
					_mew_bt_at_buffer_counter = 0;
				}

    			_mew_bt_cmd_timer = mew_get_millis();
    		}
    		break;
    	case MEW_BT_STATE_TRANSPARENT:
    		mew_comm_add_byte(MEW_DRIVER_ID_BLUETOOTH, ch);
    		break;
    	}
    }
}

static void __mew_bluetooth_reset(void) {
    gpio_clear(MEW_BLUETOOTH_RESET_PORT, MEW_BLUETOOTH_RESET_PIN);
    mew_delay_ms(10);
    gpio_set(MEW_BLUETOOTH_RESET_PORT, MEW_BLUETOOTH_RESET_PIN);
    mew_delay_ms(50);
}

unsigned int mew_bluetooth_init(void) {
	gpio_mode_setup(MEW_BLUETOOTH_RESET_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, MEW_BLUETOOTH_RESET_PIN);
    gpio_set_output_options(MEW_BLUETOOTH_RESET_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, MEW_BLUETOOTH_RESET_PIN);
    __mew_bluetooth_reset();

    gpio_mode_setup(MEW_BLUETOOTH_POWER_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, MEW_BLUETOOTH_POWER_PIN);
    gpio_set_output_options(MEW_BLUETOOTH_POWER_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, MEW_BLUETOOTH_POWER_PIN);
    gpio_clear(MEW_BLUETOOTH_POWER_PORT, MEW_BLUETOOTH_POWER_PIN);
    mew_delay_ms(10);
    gpio_set(MEW_BLUETOOTH_POWER_PORT, MEW_BLUETOOTH_POWER_PIN);
    mew_delay_ms(10);
    
    gpio_mode_setup(MEW_BLUETOOTH_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, MEW_BLUETOOTH_PIN);
    gpio_set_output_options(MEW_BLUETOOTH_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, MEW_BLUETOOTH_PIN);
    gpio_set_af(MEW_BLUETOOTH_PORT, MEW_BLUETOOTH_PORT_AF, MEW_BLUETOOTH_PIN);
    
    usart_set_baudrate(MEW_BLUETOOTH_USART, MEW_BLUETOOTH_SPEED);
    usart_set_databits(MEW_BLUETOOTH_USART, 8);
    usart_set_stopbits(MEW_BLUETOOTH_USART, USART_STOPBITS_1);
    usart_set_mode(MEW_BLUETOOTH_USART, USART_MODE_TX_RX);
    usart_set_parity(MEW_BLUETOOTH_USART, USART_PARITY_NONE);
    usart_set_flow_control(MEW_BLUETOOTH_USART, USART_FLOWCONTROL_NONE);
    usart_enable_rx_interrupt(MEW_BLUETOOTH_USART);
    usart_disable_tx_interrupt(MEW_BLUETOOTH_USART);
    usart_enable_tx_dma(MEW_BLUETOOTH_USART);
    
    nvic_enable_irq(MEW_BLUETOOTH_IRQ);
    nvic_enable_irq(MEW_BLUETOOTH_DMA_NVIC_TX);
    
    usart_enable(MEW_BLUETOOTH_USART);

    mew_delay_ms(150); // Wait for BLE ready
    memset(_mew_bt_at_buffer, 0, MEW_BT_AT_BUFFER_SIZE);
    __mew_bluetooth_send_at(MEW_BT_ATCMD_SET_NAME, "AT+NAME"MEW_BT_NAME);
    
    while (__mew_bluetooth_handler() == 1) { mew_delay_ms(10); };

    return 0;
}

void mew_bluetooth_transmit(uint8_t* data, uint16_t size, uint8_t sync_mode) {
	__mew_bluetooth_transmit(data, size, sync_mode);
}

static void __mew_bluetooth_transmit(uint8_t* data, uint16_t size, uint8_t sync_mode) {
    mew_wait_for_state(&_mew_dma_tx_state, 0);
    _mew_dma_tx_state = 1;
    
    dma_stream_reset(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX);
    dma_set_transfer_mode(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    dma_set_priority(MEW_BLUETOOTH_DMA,  MEW_BLUETOOTH_DMA_STREAM_TX, DMA_SxCR_PL_HIGH);
    dma_set_memory_size(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX, DMA_SxCR_MSIZE_8BIT);
    dma_set_peripheral_size(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX, DMA_SxCR_PSIZE_8BIT);
    dma_enable_memory_increment_mode(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX);
    dma_set_peripheral_address(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX, (uint32_t) &MEW_BLUETOOTH_DMA_DR);
    dma_channel_select(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX, MEW_BLUETOOTH_DMA_CHANNEL_TX);
    dma_set_memory_address(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX, (uint32_t) data);
    dma_set_number_of_data(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX, size);
    dma_disable_fifo_error_interrupt(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX);
    dma_disable_half_transfer_interrupt(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX);
    dma_enable_transfer_complete_interrupt(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX);
    dma_enable_transfer_error_interrupt(MEW_BLUETOOTH_DMA,    MEW_BLUETOOTH_DMA_STREAM_TX);
    dma_enable_transfer_complete_interrupt(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX);
    
	memset(_mew_bt_at_buffer, 0, MEW_BT_AT_BUFFER_SIZE);
	_mew_bt_at_buffer_counter = 0;
    
    dma_enable_stream(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX);
    
    if (sync_mode == 1) {
        mew_wait_for_state(&_mew_dma_tx_state, 0);
    }
}

void MEW_BLUETOOTH_DMA_HANDLE(void) {
    dma_stream_reset(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX);
    dma_disable_stream(MEW_BLUETOOTH_DMA, MEW_BLUETOOTH_DMA_STREAM_TX);
    _mew_dma_tx_state = 0;
}

