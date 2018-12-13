#include "comm.h"

static void __mew_comm_reset(void);
static void __mew_comm_timeout_reset(void);

static uint8_t _mew_comm_buffer[MEW_COMM_BUF_MAX_SIZE];
static volatile uint32_t _mew_comm_buffer_counter = 0;
static volatile uint32_t _mew_comm_payload_length = 0;
static volatile uint32_t _mew_comm_payload_crc32  = 0;

static volatile uint32_t _mew_comm_driver_id = 0;
static volatile uint32_t _mew_comm_last_time = 0;

static uint8_t _mew_comm_payload[MEW_COMM_PAYLOAD_MAX_SIZE];
static volatile uint32_t _mew_comm_payload_present  = 0;
static volatile uint32_t _mew_comm_last_error       = 0;

static void __mew_comm_reset(void) {
    _mew_comm_buffer_counter  = 0;
    _mew_comm_payload_crc32   = 0;
}

static void __mew_comm_timeout_reset(void) {
    if ((_mew_comm_last_time > 0) && (_mew_comm_last_time < mew_get_millis()) && (_mew_comm_buffer_counter > 0)) {
        _mew_comm_last_error = MEW_COMM_ERR_TIMEOUT;
        __mew_comm_reset();
    }
    _mew_comm_last_time = mew_get_millis() + MEW_COMM_TIMEOUT_MS;
}

uint32_t mew_comm_is_payload_present(void) {
    return _mew_comm_payload_present;
}

void mew_comm_get_payload(uint8_t* payload, uint32_t* length) {
    *length = _mew_comm_payload_length;
    memcpy(payload, _mew_comm_payload, _mew_comm_payload_length);
    _mew_comm_payload_present = 0;
    _mew_comm_payload_length  = 0;
}

void mew_comm_add_byte(uint32_t driver_id, uint8_t b) {
    uint32_t current_crc32;
    
    // если в буфере данных что-то есть - больше ничего с порта не принимаем, пока данные не заберут
    if (_mew_comm_payload_present == 1) {
        _mew_comm_last_error = MEW_COMM_ERR_BISY;
        __mew_comm_reset();
        return;
    }
    
    // очистка буфера по таймауту, чтобы данные не залеживались =)
    __mew_comm_timeout_reset();

    // защита от перемешивания байт с разных источников, на всякий случай
    if (driver_id != _mew_comm_driver_id) {
        _mew_comm_last_error = MEW_COMM_ERR_DRIVER_COLLISION;
        __mew_comm_reset();
        _mew_comm_driver_id = driver_id;
        return;
    }
    
    _mew_comm_buffer[_mew_comm_buffer_counter] = b;

    // если буфер пустой - ничего не делаем
    if (_mew_comm_buffer_counter < 2) {
        return;
    }
    
    // если в буфере нет магического числа - очищаем буфер
    if ((_mew_comm_buffer_counter == 2) && 
            ((_mew_comm_buffer[0] != MEW_COMM_MAGIC_1) || (_mew_comm_buffer[1] != MEW_COMM_MAGIC_2))) {
        _mew_comm_last_error = MEW_COMM_ERR_BAD_PACKAGE_MAGIC;
        __mew_comm_reset();
        return;
    }
    
    // проверяем длину полезной части - если она больше максимально разрешенной - очищаем буфер
    if (_mew_comm_buffer_counter == 4) {
        _mew_comm_payload_length = MEW_COMM_BYTES_TO_UINT16(_mew_comm_buffer[2], _mew_comm_buffer[3]);
        if (_mew_comm_payload_length > MEW_COMM_PAYLOAD_MAX_SIZE) {
            _mew_comm_last_error = MEW_COMM_ERR_BAD_PAYLOAD_SIZE;
            __mew_comm_reset();
            return;
        }
    }
    
    // запоминаем контрольную сумму пакета
    if (_mew_comm_buffer_counter == 8) {
        _mew_comm_payload_crc32 = MEW_COMM_BYTES_TO_UINT32(_mew_comm_buffer[4], _mew_comm_buffer[5], _mew_comm_buffer[6], _mew_comm_buffer[7]);
    }
    
    // если длина буфера равна заголовку плюс полезным данным, проверяем контрольную сумму
    if (_mew_comm_buffer_counter >= (_mew_comm_payload_length + 8)) {
        current_crc32 = crc_calculate_block(_mew_comm_buffer + 8, _mew_comm_payload_length);
        if (current_crc32 != _mew_comm_payload_crc32) {
            _mew_comm_last_error = MEW_COMM_ERR_BAD_PAYLOAD_CRC32;
            __mew_comm_reset();
            return;
        }
        
        memcpy(_mew_comm_payload, _mew_comm_buffer + 8, _mew_comm_payload_length);
        _mew_comm_payload_present = 1;
        _mew_comm_last_error = 0;
        __mew_comm_reset();
    }

    _mew_comm_buffer_counter++;
    if (_mew_comm_buffer_counter >= MEW_COMM_BUF_MAX_SIZE) {
        _mew_comm_last_error = MEW_COMM_ERR_BUFFER_OVERFLOW;
        __mew_comm_reset();
        return;
    }
}

void mew_comm_handler(void) {
    if (_mew_comm_last_error > 0) {
        
        _mew_comm_last_error = 0;
    }
    
    
    
}