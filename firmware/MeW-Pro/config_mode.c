#include "config_mode.h"

u8 mew_conf_mode_buffer[MEW_CM_INPUT_BUFFER_SIZE];
u32 mew_conf_mode_buffer_counter = 0;

struct mew_cm_command_header* command_field = (struct mew_cm_command_header*) mew_conf_mode_buffer;
u8* data_field = (u8*) (mew_conf_mode_buffer + sizeof(struct mew_cm_command_header));
struct mew_cm_command_header command_ret;

volatile u32 cm_timer_counter = 0;
struct settings_record sett_r;

void cm_timer_proc(void) {
    cm_timer_counter++;
    if (cm_timer_counter > MEW_CM_CLEANER_TIMER) {
        cm_timer_counter = 0;
        mew_conf_mode_buffer_counter = 0;
    }
}

void __cm_buf_reset(void) {
    memset(mew_conf_mode_buffer, 0, MEW_CM_INPUT_BUFFER_SIZE);
    mew_conf_mode_buffer_counter = 0;
}

u32 __get_data_len(void) {
    return (mew_conf_mode_buffer_counter > sizeof(struct mew_cm_command_header)) ? (mew_conf_mode_buffer_counter - sizeof(struct mew_cm_command_header)) : 0;
}

void __error_send(void) {
    command_ret.command    = MEW_CM_RET_ERROR;
    command_ret.data_crc32 = 0;
    command_ret.data_len   = 0;
    cdcacm_send_chars((u8*) &command_ret, sizeof(struct mew_cm_command_header));
    __cm_buf_reset();
}

void __ping_send(void) {
    command_ret.command    = MEW_CM_RET_PING;
    command_ret.data_crc32 = 0;
    command_ret.data_len   = 0;
    cdcacm_send_chars((u8*) &command_ret, sizeof(struct mew_cm_command_header));
    __cm_buf_reset();
}

void __data_send(u32 cmd, u8* data, u32 len) {
    u32 datablock = 0;
    s32 count = len;
    
    command_ret.command    = cmd;
    command_ret.data_crc32 = (data != NULL) ? crc_gen((u32*) data, len / sizeof(u32)) : 0;
    command_ret.data_len   = len;
    cdcacm_send_chars((u8*) &command_ret, sizeof(struct mew_cm_command_header));
    
    if (data != NULL) {
        while(1) {
            cdcacm_send_chars((u8*) (data + datablock), (count > MEW_CM_OUT_MAX_PACKET_SIZE) ? MEW_CM_OUT_MAX_PACKET_SIZE : count);
            datablock += MEW_CM_OUT_MAX_PACKET_SIZE;
            count -= MEW_CM_OUT_MAX_PACKET_SIZE;
            if (count <= 0) break;
        }
    }
    
    __cm_buf_reset(); 
}

u32 check_crc(u32 crc32, u32* data, u16 len) {
    u32 calc_crc = crc_gen(data, len / sizeof(u32));
    if (calc_crc == crc32) return MEW_CRC_RET_OK; else return MEW_CRC_RET_FAIL;
}

u32 mew_cm_push_raw(u8* raw, u8 len) {
    timer_set_counter(TIM2, 0);
    cm_timer_counter = 0;
    
    if (len >= MEW_CM_INPUT_BUFFER_SIZE) return MEW_CM_ERR_TOO_MANY_DATA;
    if ((mew_conf_mode_buffer_counter + len) >= MEW_CM_INPUT_BUFFER_SIZE) {
        __cm_buf_reset();
        return MEW_CM_ERR_TOO_MANY_DATA;
    }
    
    memcpy(mew_conf_mode_buffer + mew_conf_mode_buffer_counter, raw, len);
    mew_conf_mode_buffer_counter += len;
    
    return MEW_CM_OK;
}

void mew_cm_poll(void) {
    if (mew_conf_mode_buffer_counter >= sizeof(struct mew_cm_command_header)) {
        debug_print_hex(mew_conf_mode_buffer, mew_conf_mode_buffer_counter);
        switch (command_field->command) {
            case MEW_CM_PING:
                debug_print("PING RECEIVE");
                __ping_send();
                break;
            case MEW_CM_IDENT:
                
                break;
            case MEW_CM_READ_PASSWD:
                
                break;
            case MEW_CM_WRITE_PASSWD:
                
                break;
            case MEW_CM_CONFIG_READ:
                if (__get_data_len() >= sizeof(u32)) {                    
                    if (check_crc(command_field->data_crc32, (u32*) data_field, sizeof(u32)) == MEW_CRC_RET_FAIL) {
                        __data_send(MEW_CM_RET_CRC_FAIL, NULL, 0);
                    } else {
                        if (mewcrypt_read_settings(&sett_r, *((u32*)data_field)) == MEW_CRYPT_OK) {
                            __data_send(MEW_CM_RET_OK, (u8*) &sett_r, sizeof(struct settings_record));
                        } else {
                            __data_send(MEW_CM_RET_DATA_ACCESS_FAIL, NULL, 0);
                        }
                    }
                    __cm_buf_reset();
                }
                break;
            case MEW_CM_CONFIG_WRITE:
                
                break;
            default:
                debug_print("DATA ERROR");
                //debug_print_hex(mew_conf_mode_buffer, mew_conf_mode_buffer_counter);
                __error_send();
        }
    }
}







