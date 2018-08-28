#include "../debug/debug.h"
#include "bluetooth.h"

static const mew_bt_at_command _mew_at_is_default_name = { 0, 0, "AT+NAME", "+NAME:JDY-08" };
static const mew_bt_at_command _mew_at_is_set_name = { 0, 0, "AT+NAMEMeW Pro", "+OK" };

static const mew_bt_at_command _mew_at_is_cofigured_name = { 0, 0, "AT+NAME", "MeW Pro" };
static const mew_bt_at_command _mew_at_is_transparent = { 0, 0, "AT+HOSTEN0", "+OK" };

void mew_bluetooth_config(void) {
    uint32_t errcode = mew_bluetooth_execute_at(&_mew_at_is_cofigured_name);
    switch (errcode) {
        case MEW_BT_CS_OK:
            mew_bluetooth_execute_at(&_mew_at_is_transparent);
            break;
        case MEW_BT_CS_ERROR:
            mew_bluetooth_execute_at(&_mew_at_is_set_name);
            mew_bluetooth_execute_at(&_mew_at_is_transparent);
#ifdef INTEGRATION_TESTS
            mew_debug_print("First-time bluetooth config ok.");
#endif
            break;
        
    }
}
