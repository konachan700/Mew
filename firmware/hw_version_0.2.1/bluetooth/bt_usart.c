#include "../debug/debug.h"
#include "bluetooth.h"

static void __mew_write_byte_to_buf_at(uint8_t b);
static void __mew_write_byte_to_buf_transparent(uint8_t b);
static void __mew_bluetooth_clear_buf(void);

static volatile uint32_t _mew_bt_timer = 0;
static volatile uint32_t _mew_bt_count = 0;
static uint8_t  _mew_bt_buffer[MEW_BT_RECEIVE_BUFFER_SIZE];

static volatile uint8_t _mew_dma_tx_state = 0;

void mew_bluetooth_init(void) {
    gpio_mode_setup(MEW_BLUETOOTH_POWER_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, MEW_BLUETOOTH_POWER_PIN);
    gpio_set_output_options(MEW_BLUETOOTH_POWER_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, MEW_BLUETOOTH_POWER_PIN);
    gpio_clear(MEW_BLUETOOTH_POWER_PORT, MEW_BLUETOOTH_POWER_PIN);
    
    gpio_mode_setup(MEW_BLUETOOTH_RESET_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, MEW_BLUETOOTH_RESET_PIN);
    gpio_set_output_options(MEW_BLUETOOTH_RESET_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ, MEW_BLUETOOTH_RESET_PIN);
    gpio_set(MEW_BLUETOOTH_RESET_PORT, MEW_BLUETOOTH_RESET_PIN);
    
    gpio_mode_setup(MEW_BLUETOOTH_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, MEW_BLUETOOTH_PIN);
    gpio_set_af(MEW_BLUETOOTH_PORT, MEW_BLUETOOTH_PORT_AF, MEW_BLUETOOTH_PIN);
    
    usart_disable(MEW_BLUETOOTH_USART);
    usart_set_baudrate(MEW_BLUETOOTH_USART, MEW_BLUETOOTH_SPEED);
    usart_set_databits(MEW_BLUETOOTH_USART, 8);
    usart_set_stopbits(MEW_BLUETOOTH_USART, USART_STOPBITS_1);
    usart_set_mode(MEW_BLUETOOTH_USART, USART_MODE_TX_RX);
    usart_set_parity(MEW_BLUETOOTH_USART, USART_PARITY_NONE);
    usart_set_flow_control(MEW_BLUETOOTH_USART, USART_FLOWCONTROL_NONE);
    usart_enable_rx_interrupt(MEW_BLUETOOTH_USART);
    usart_disable_tx_interrupt(MEW_BLUETOOTH_USART);
    usart_enable_tx_dma(MEW_BLUETOOTH_USART);
    usart_enable(MEW_BLUETOOTH_USART);
    
    nvic_enable_irq(MEW_BLUETOOTH_IRQ);
    nvic_enable_irq(MEW_BLUETOOTH_DMA_NVIC_TX);
    
    memset(_mew_bt_buffer, 0, MEW_BT_RECEIVE_BUFFER_SIZE);
}

void mew_bluetooth_transmit(uint8_t* data, uint16_t size, uint8_t sync_mode) {
    mew_check_dma_memory((void*)data, "mew_bluetooth_transmit");
    
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
    
    __mew_bluetooth_clear_buf();
    
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

void mew_bluetooth_transmit_string(char* string) {
    mew_bluetooth_transmit((uint8_t*) string, strlen(string), 1);
}

void MEW_BLUETOOTH_ISR(void) {
    if (USART1_SR & USART_SR_RXNE) {
        if (_mew_bt_timer < mew_get_millis()) {
            __mew_bluetooth_clear_buf();
        }
        
        _mew_bt_buffer[_mew_bt_count] = (uint8_t) MEW_BLUETOOTH_DMA_DR;
        _mew_bt_count++;
        if (_mew_bt_count >= MEW_BT_RECEIVE_BUFFER_SIZE) {
            __mew_bluetooth_clear_buf();
        }
        
        _mew_bt_timer = mew_get_millis() + 50;
    }
}

uint32_t mew_bluetooth_execute_at(const mew_bt_at_command *at) {
    uint32_t i = mew_get_millis() + 256;
    mew_bluetooth_transmit_string((char*)at->command);
    
    while (i > mew_get_millis()) {
        if (_mew_bt_count > 1) {
            if (strchr((char*)_mew_bt_buffer, '\n')) {
                if (strstr((char*)_mew_bt_buffer, at->wait_reply_pattern)) {
                    __mew_bluetooth_clear_buf();
                    return MEW_BT_CS_OK;
                } 

                //mew_debug_print("AT bad request");
                mew_debug_print((char*) _mew_bt_buffer);
    
                __mew_bluetooth_clear_buf();
                return MEW_BT_CS_ERROR;
            }
        }
    }
    
    //mew_debug_print("AT timeout error");
    __mew_bluetooth_clear_buf();
    return MEW_BT_CS_TIMEOUT;
}

static void __mew_bluetooth_clear_buf(void) {
    _mew_bt_count = 0;
    memset((void*)_mew_bt_buffer, 0, MEW_BT_RECEIVE_BUFFER_SIZE);
}
