#include "parser.h"
#include "debug.h"
#include "drivers/hw_crypt/P256-cortex-ecdh.h"

static void __mew_pp_reset(void);
static void __mew_comm_timeout_reset(void);
static uint32_t __mew_pp_checksum(uint8_t * data, uint32_t len);
static void __mew_send_pubkey(void);

static uint8_t _mew_comm_buffer[MEW_COMM_BUF_MAX_SIZE];
static volatile uint32_t _mew_comm_buffer_counter = 0;
static volatile uint32_t _mew_comm_payload_length = 0;
static volatile uint32_t _mew_comm_payload_crc32  = 0;
static volatile uint32_t _mew_comm_command 		  = 0;

static volatile uint32_t _mew_comm_driver_id = 0;
static volatile uint32_t _mew_comm_last_time = 0;

static uint8_t _mew_comm_payload[MEW_COMM_PAYLOAD_MAX_SIZE];
static volatile uint32_t _mew_comm_payload_present  = 0;
static volatile uint32_t _mew_comm_last_error       = 0;
static volatile uint32_t current_checksum = 0;


//static void __mew_comm_reset(void) {
//    _mew_comm_buffer_counter  = 0;
//    _mew_comm_payload_crc32   = 0;
//    
//    mew_debug_print_hex((const char*)&_mew_comm_last_error, sizeof(uint32_t));
//    _mew_comm_last_error = 0;
//}

static void __mew_comm_timeout_reset(void) {
    if ((_mew_comm_last_time > 0) && (_mew_comm_last_time < mew_get_millis()) && (_mew_comm_buffer_counter > 0)) {
        _mew_comm_last_error = MEW_COMM_ERR_TIMEOUT;
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
    uint32_t i = 0;

    if (_mew_comm_last_error > 0) return;
    if (_mew_comm_payload_present == 1) return;
    
    // очистка буфера по таймауту, чтобы данные не залеживались =)
    __mew_comm_timeout_reset();

    // защита от перемешивания байт с разных источников, на всякий случай
//    if (driver_id != _mew_comm_driver_id) {
//        _mew_comm_last_error = MEW_COMM_ERR_DRIVER_COLLISION;
//        __mew_comm_reset();
//        _mew_comm_driver_id = driver_id;
//        return;
//    }
    
    _mew_comm_buffer[_mew_comm_buffer_counter] = b;
    _mew_comm_buffer_counter++;
    if (_mew_comm_buffer_counter >= MEW_COMM_BUF_MAX_SIZE) {
        _mew_comm_last_error = MEW_COMM_ERR_BUFFER_OVERFLOW;
        return;
    }

    if (_mew_comm_buffer_counter <= 10) {
		switch (_mew_comm_buffer_counter) {
		case 2: // MAGIC VERIFY
			if (MEW_COMM_IS_MAGIC_INVALID(_mew_comm_buffer)) {
				_mew_comm_last_error = MEW_COMM_ERR_BAD_PACKAGE_MAGIC;
				return;
			}
			break;
		case 4: // COMMAND
			_mew_comm_command = MEW_COMM_GET_COMMAND(_mew_comm_buffer);
			break;
		case 6: // PAYLOAD SIZE
			_mew_comm_payload_length = MEW_COMM_GET_PL_SIZE(_mew_comm_buffer);
			if (_mew_comm_payload_length > MEW_COMM_PAYLOAD_MAX_SIZE) {
				_mew_comm_last_error = MEW_COMM_ERR_BAD_PAYLOAD_SIZE;
				return;
			}
			break;
		case 10:
			_mew_comm_payload_crc32 = MEW_COMM_GET_PL_CRC32(_mew_comm_buffer);
			break;
		}
    } else if (_mew_comm_buffer_counter >= (_mew_comm_payload_length + MEW_COMM_HEADER_MAX_SIZE)) {
        memset(_mew_comm_payload, 0, MEW_COMM_PAYLOAD_MAX_SIZE);
        memcpy(_mew_comm_payload, _mew_comm_buffer + MEW_COMM_HEADER_MAX_SIZE, _mew_comm_payload_length);
        
        current_checksum = __mew_pp_checksum(_mew_comm_payload, _mew_comm_payload_length);
        if (current_checksum != _mew_comm_payload_crc32) {
            _mew_comm_last_error = MEW_COMM_ERR_BAD_PAYLOAD_CRC32;
            return;
        }
        
        _mew_comm_payload_present = 1;
        _mew_comm_last_error      = 0;
        _mew_comm_buffer_counter  = 0;
        _mew_comm_payload_crc32   = 0;
    }
}

static uint32_t __mew_pp_checksum(uint8_t* data, uint32_t len) {
	uint32_t i = 0;
	uint32_t checksum = 0x437700FF;

	for (i=0; i<len; i++) checksum ^= (data[i] << (i % 24));

	return checksum;
}

static void __mew_pp_reset(void) {
    memset(_mew_comm_buffer, 0, MEW_COMM_BUF_MAX_SIZE);
    memset(_mew_comm_payload, 0, MEW_COMM_PAYLOAD_MAX_SIZE);

    _mew_comm_buffer_counter  = 0;
    _mew_comm_payload_crc32   = 0;
    _mew_comm_last_error      = 0;
    _mew_comm_payload_present = 0;
    _mew_comm_payload_length  = 0;
    _mew_comm_driver_id       = 0;
}

void send_ok_packet(uint8_t* data, uint32_t size) {
	uint8_t ok_data[10];
	uint32_t checksum = __mew_pp_checksum(data, size);

	ok_data[0] = 0x43;
	ok_data[1] = 0x77;
	ok_data[2] = MEW_SHL8(_mew_comm_command);
	ok_data[3] = MEW_SHL0(_mew_comm_command);
	ok_data[4] = MEW_SHL8(size);
	ok_data[5] = MEW_SHL0(size);
	ok_data[9] = MEW_SHL0(checksum);
	ok_data[8] = MEW_SHL8(checksum);
	ok_data[7] = MEW_SHL16(checksum);
	ok_data[6] = MEW_SHL24(checksum);

	mew_bluetooth_transmit(ok_data, 10, 1);
	mew_bluetooth_transmit(data, size, 0);
}

static uint8_t _mew_pubkey_buf[64];
static void __mew_send_pubkey(void) {
	uint32_t i;
	uint32_t* data32 = (uint32_t*) _mew_pubkey_buf;

	uint32_t data_len = mew_p256_ecdh_get_session_pubkey((char*) _mew_pubkey_buf);
	for (i=0; i<(data_len / sizeof(uint32_t)); i++) {
		data32[i] = big2little32(data32[i]);
	}

	send_ok_packet(_mew_pubkey_buf, data_len);
}

static void __mew_receive_pubkey(void) {
	uint32_t i;
	uint32_t* data32 = (uint32_t*) _mew_comm_payload;
	uint8_t priv_key[32];
	uint8_t shared_key[32];

	mew_p256_ecdh_get_session_privkey((char*)priv_key);

	if (_mew_comm_payload_length != 64) return;
	for (i=0; i<(64 / sizeof(uint32_t)); i++) {
		data32[i] = big2little32(data32[i]);
	}

	if (!P256_ecdh_shared_secret(shared_key, _mew_comm_payload, priv_key)) {
		uint8_t pub_key[64];
		data32 = (uint32_t*) pub_key;
		mew_p256_ecdh_get_session_pubkey((char*)pub_key);
		for (i=0; i<(64 / sizeof(uint32_t)); i++) {
			data32[i] = big2little32(data32[i]);
		}

		mew_debug_print("Can't create a shared key! Bad data was received.");
		mew_debug_print("Local pubkey:");
		mew_debug_print_hex((const char*)pub_key, 64);
//		mew_debug_print("Remote pubkey:");
//		mew_debug_print_hex((const char*)_mew_comm_payload, 64);


//		data32 = (uint32_t*) priv_key;
//		for (i=0; i<(32 / sizeof(uint32_t)); i++) {
//			data32[i] = big2little32(data32[i]);
//		}
		mew_debug_print("Local privkey:");
		mew_debug_print_hex((const char*)priv_key, 32);
		return;
	}

	mew_p256_ecdh_set_session_sharedkey((char*)shared_key);
	mew_debug_print_hex((const char*)shared_key, 32);
}

unsigned int mew_comm_handler(void) {
//	uint8_t data[255];
//	uint32_t data_len = 0;

    if (_mew_comm_last_error > 0) {
        mew_debug_print("ERROR:");
        mew_debug_print_hex((const char*)&_mew_comm_last_error, sizeof(uint32_t));
        mew_debug_print_hex((const char*)&current_checksum, sizeof(uint32_t));
        mew_debug_print_hex((const char*)&_mew_comm_payload_crc32, sizeof(uint32_t));
        mew_debug_print_hex((const char*)_mew_comm_buffer, _mew_comm_buffer_counter);
        
        __mew_pp_reset();
        
        return 0;
    }

    if (_mew_comm_payload_present == 1) {
    	switch (_mew_comm_command) {
    	case MEW_COMM_GET_DEVICE_SESSION_KEY:
    		__mew_send_pubkey();
    		break;
    	case MEW_COMM_SEND_HOST_SESSION_KEY:
    		__mew_receive_pubkey();
    		break;
    	};

//        mew_debug_print("OK");
//        mew_debug_print_hex((const char*)_mew_comm_payload, _mew_comm_payload_length);

        __mew_pp_reset();
    }
    
    return 0;
}
