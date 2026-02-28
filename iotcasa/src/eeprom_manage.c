/*
 * eeprom_manage.c
 *
 *  Created on: 06-Feb-2026
 *      Author: User
 */


#include <casa_log.h>
#include <gpio_control.h>
#include "nvm3_default.h"
#include "nvm3_default_config.h"
#include "sl_si91x_driver_gpio.h"

#include "eeprom_manage.h"
#include "casa_module.h"
#include "device_control.h"

#define NVM3_DEFAULT_HANDLE nvm3_defaultHandle

extern float fw_version;
extern casa_context_t casa_ctx;
extern sl_si91x_gpio_pin_config_t load_gpio_cfg[];
extern device_control_context_t device_control;

// Group 0x00: Device Registration & Identity
#define NVM3_KEY_REG_STATUS    0x0001
#define NVM3_KEY_UNIQUE_ID     0x0002
#define NVM3_KEY_HOST_NAME     0x0003
#define NVM3_KEY_WIFI_SSID     0x0004
#define NVM3_KEY_WIFI_PASSWORD 0x0005
#define NVM3_KEY_USERID        0x0006
#define NVM3_KEY_LOCATION_ID   0x0007
#define NVM3_KEY_REG_MODE      0x0008

#define NVM3_KEY_HW_VER        0x0101
#define NVM3_KEY_HW_BATCH      0x0102

#define NVM3_KEY_ENDPOINT_BASE  0x0200
#define NVM3_KEY_ENDPOINT(n)    (NVM3_KEY_ENDPOINT_BASE + n)

// Group 0x03: State Info
#define NVM3_KEY_SW_STATE      0x0301
#define NVM3_KEY_SECURE        0x0302

// FOTA Group (0x04xx)
#define NVM3_KEY_FOTA_STATUS   0x0401
#define NVM3_KEY_FOTA_URL      0x0402
#define NVM3_KEY_OLD_FW_VER    0x0403

#define NVM3_KEY_TIMER_BASE    0x0500
#define NVM3_KEY_TIMER(n)      (NVM3_KEY_TIMER_BASE + n)

static bool nvm3_read_u8_key(nvm3_ObjectKey_t key, uint8_t *value)
{
    uint32_t object_type = 0;
    size_t object_len = 0;
    Ecode_t err = nvm3_getObjectInfo(nvm3_defaultHandle, key, &object_type, &object_len);

    if (err != ECODE_NVM3_OK || object_type != NVM3_OBJECTTYPE_DATA || object_len != sizeof(uint8_t)) {
        return false;
    }

    return (nvm3_readData(nvm3_defaultHandle, key, value, sizeof(uint8_t)) == ECODE_NVM3_OK);
}

static bool nvm3_read_i32_key(nvm3_ObjectKey_t key, int32_t *value)
{
    uint32_t object_type = 0;
    size_t object_len = 0;
    Ecode_t err;

    if (value == NULL) {
        return false;
    }

    err = nvm3_getObjectInfo(nvm3_defaultHandle, key, &object_type, &object_len);
    if (err != ECODE_NVM3_OK || object_type != NVM3_OBJECTTYPE_DATA || object_len != sizeof(int32_t)) {
        return false;
    }

    return (nvm3_readData(nvm3_defaultHandle, key, value, sizeof(int32_t)) == ECODE_NVM3_OK);
}

//static bool nvm3_read_u32_key(nvm3_ObjectKey_t key, uint32_t *value)
//{
//    uint32_t object_type = 0;
//    size_t object_len = 0;
//    Ecode_t err;
//
//    if (value == NULL) {
//        return false;
//    }
//
//    err = nvm3_getObjectInfo(nvm3_defaultHandle, key, &object_type, &object_len);
//    if (err != ECODE_NVM3_OK || object_type != NVM3_OBJECTTYPE_DATA || object_len != sizeof(uint32_t)) {
//        return false;
//    }
//
//    return (nvm3_readData(nvm3_defaultHandle, key, value, sizeof(uint32_t)) == ECODE_NVM3_OK);
//}

static bool nvm3_read_float_key(nvm3_ObjectKey_t key, float *value)
{
    uint32_t object_type = 0;
    size_t object_len = 0;
    Ecode_t err;

    if (value == NULL) {
        return false;
    }

    err = nvm3_getObjectInfo(nvm3_defaultHandle, key, &object_type, &object_len);
    if (err != ECODE_NVM3_OK || object_type != NVM3_OBJECTTYPE_DATA || object_len != sizeof(float)) {
        return false;
    }

    return (nvm3_readData(nvm3_defaultHandle, key, value, sizeof(float)) == ECODE_NVM3_OK);
}

static bool nvm3_read_string_key(nvm3_ObjectKey_t key, char *dst, size_t dst_len)
{
    uint32_t object_type = 0;
    size_t object_len = 0;
    size_t read_len = 0;
    Ecode_t err;

    if (dst == NULL || dst_len == 0) {
        return false;
    }

    dst[0] = '\0';

    err = nvm3_getObjectInfo(nvm3_defaultHandle, key, &object_type, &object_len);
    if (err != ECODE_NVM3_OK || object_type != NVM3_OBJECTTYPE_DATA || object_len == 0) {
        return false;
    }

    read_len = (object_len < (dst_len - 1)) ? object_len : (dst_len - 1);
    err = nvm3_readData(nvm3_defaultHandle, key, dst, read_len);
    if (err != ECODE_NVM3_OK) {
        dst[0] = '\0';
        return false;
    }

    dst[read_len] = '\0';
    return true;
}

static void nvm3_write_u8_key(nvm3_ObjectKey_t key, uint8_t value)
{
    nvm3_writeData(nvm3_defaultHandle, key, &value, sizeof(uint8_t));
}

static void nvm3_write_i32_key(nvm3_ObjectKey_t key, int32_t value)
{
    nvm3_writeData(nvm3_defaultHandle, key, &value, sizeof(int32_t));
}

static void nvm3_write_float_key(nvm3_ObjectKey_t key, float value)
{
    nvm3_writeData(nvm3_defaultHandle, key, &value, sizeof(float));
}

static void nvm3_write_string_key(nvm3_ObjectKey_t key, const char *src)
{
    if (src == NULL) {
        return;
    }

    if (src[0] == '\0') {
        nvm3_deleteObject(nvm3_defaultHandle, key);
        return;
    }

    nvm3_writeData(nvm3_defaultHandle, key, (const uint8_t *)src, strlen(src) + 1);
}


void get_eeprom_device_reg_info(void)
{
    memset(casa_ctx.ssid, 0, sizeof(casa_ctx.ssid));
    memset(casa_ctx.password, 0, sizeof(casa_ctx.password));
    memset(casa_ctx.userid, 0, sizeof(casa_ctx.userid));
    memset(casa_ctx.location, 0, sizeof(casa_ctx.location));

    if (!nvm3_read_u8_key(NVM3_KEY_REG_STATUS, &casa_ctx.reg_status)) {
        casa_ctx.reg_status = (uint8_t)!REGISTRATION_DONE;
        nvm3_write_u8_key(NVM3_KEY_REG_STATUS, casa_ctx.reg_status);
        LOG_WARN("NVS", "Execution: Reg Status not available yet, defaulted to NOT_DONE");
    }

    nvm3_read_string_key(NVM3_KEY_WIFI_SSID, casa_ctx.ssid, sizeof(casa_ctx.ssid));
    nvm3_read_string_key(NVM3_KEY_WIFI_PASSWORD, casa_ctx.password, sizeof(casa_ctx.password));
    nvm3_read_string_key(NVM3_KEY_USERID, casa_ctx.userid, sizeof(casa_ctx.userid));
    nvm3_read_string_key(NVM3_KEY_LOCATION_ID, casa_ctx.location, sizeof(casa_ctx.location));

    // UID / Hostname fallback to generated defaults if NVM data is not present.
    if (casa_ctx.uniqueid[0] == '\0') {
        nvm3_read_string_key(NVM3_KEY_UNIQUE_ID, casa_ctx.uniqueid, sizeof(casa_ctx.uniqueid));
    }
    if (casa_ctx.hostname[0] == '\0') {
        nvm3_read_string_key(NVM3_KEY_HOST_NAME, casa_ctx.hostname, sizeof(casa_ctx.hostname));
    }

    // 3. Keep registration state strict: only complete if required cloud fields are present.
    if (casa_ctx.reg_status == REGISTRATION_DONE) {
        if (strlen(casa_ctx.ssid) == 0 || strlen(casa_ctx.password) == 0 || strlen(casa_ctx.userid) == 0) {
            casa_ctx.reg_status = (uint8_t)!REGISTRATION_DONE;
            nvm3_write_u8_key(NVM3_KEY_REG_STATUS, casa_ctx.reg_status);
            LOG_WARN("NVS", "Execution: Reg status reset. Missing SSID/PWD/User in NVM");
        }
    }

    LOG_INFO("NVS", "Device Reg Info: Stat:%d, UID:%s, Host:%s, SSID:%s, User:%s LocId:%s",
             casa_ctx.reg_status, casa_ctx.uniqueid, casa_ctx.hostname, casa_ctx.ssid, casa_ctx.userid, casa_ctx.location);
}

void set_eeprom_device_reg_info(void)
{
    bool has_required_fields = (strlen(casa_ctx.ssid) > 0 && strlen(casa_ctx.password) > 0 && strlen(casa_ctx.userid) > 0);

    if (casa_ctx.reg_status == REGISTRATION_DONE && !has_required_fields) {
        LOG_WARN("NVM", "Execution: Prevented save with empty SSID/PWD/User. Forcing reg_status to NOT_DONE");
        casa_ctx.reg_status = (uint8_t)!REGISTRATION_DONE;
    }

    nvm3_write_u8_key(NVM3_KEY_REG_STATUS, casa_ctx.reg_status);

    nvm3_write_string_key(NVM3_KEY_UNIQUE_ID, casa_ctx.uniqueid);
    nvm3_write_string_key(NVM3_KEY_HOST_NAME, casa_ctx.hostname);
    nvm3_write_string_key(NVM3_KEY_WIFI_SSID, casa_ctx.ssid);
    nvm3_write_string_key(NVM3_KEY_WIFI_PASSWORD, casa_ctx.password);
    nvm3_write_string_key(NVM3_KEY_USERID, casa_ctx.userid);
    nvm3_write_string_key(NVM3_KEY_LOCATION_ID, casa_ctx.location);

    LOG_INFO("NVM", "Execution: SUCCESS - Device Reg Info Saved (Stat:%d SSID:%u User:%u Loc:%u)",
             casa_ctx.reg_status,
             (unsigned int)strlen(casa_ctx.ssid),
             (unsigned int)strlen(casa_ctx.userid),
             (unsigned int)strlen(casa_ctx.location));
}

void get_eeprom_device_HW_details(void)
{
    if (!nvm3_read_float_key(NVM3_KEY_HW_VER, &casa_ctx.hardware_ver)) {
        casa_ctx.hardware_ver = 0.0f;
    }

    nvm3_read_string_key(NVM3_KEY_HW_BATCH, casa_ctx.hw_batch, sizeof(casa_ctx.hw_batch));
    LOG_INFO("NVM", "Execution: SUCCESS - HW Details Loaded (Ver: %.2f | Batch: %s)", casa_ctx.hardware_ver, casa_ctx.hw_batch);
}

void set_eeprom_device_HW_details(void)
{
    float my_float_value = HW_VERSION;

    nvm3_write_float_key(NVM3_KEY_HW_VER, my_float_value);
    nvm3_write_string_key(NVM3_KEY_HW_BATCH, HW_BATCH);
    LOG_INFO("NVM", "Execution: SUCCESS - HW Details Saved (Ver: %.2f | Batch: %s)", my_float_value, HW_BATCH);
}

void set_timer_control(int node, int state)
{
    nvm3_write_i32_key(NVM3_KEY_TIMER(node), (int32_t)state);
    LOG_INFO("NVM", "Execution: SUCCESS - Timer Node %d State Saved: %d (Key: 0x%04X)", node, state, NVM3_KEY_TIMER(node));
}

int get_timer_control(int node)
{
    int32_t state = 0;

    if (!nvm3_read_i32_key(NVM3_KEY_TIMER(node), &state)) {
        LOG_WARN("NVM", "Execution: Timer Node %d not found, returning 0", node);
        return 0;
    }

    LOG_INFO("NVM", "Execution: SUCCESS - Timer Node %d loaded: %d", node, state);
    return (int)state;
}

void save_device_status(int index)
{
    casa_ctx.switch_interrupts = DISABLE;
    int32_t val = device_control.endpoint_info[index].set_value;

    // Save to NVM3 using Endpoint ID (e.g., Index 0 -> Key 0x0201)
    nvm3_write_i32_key(NVM3_KEY_ENDPOINT(device_control.endpoint_info[index].endpoint), val);

    if (casa_ctx.reg_dereg_flag == 0) casa_ctx.switch_interrupts = ENABLE;
    LOG_INFO("NVM", "Execution: SUCCESS - Saved Index %d [Value: %ld] Endpoint :%d", index, val, device_control.endpoint_info[index].endpoint);
}

void get_eeprom_device_state_info(void)
{
    for (int i = 1; i <= NO_OF_ENDPOINTS; i++) {
        int32_t saved_val = 0;

        // Read state from NVM3
        bool has_saved_state = nvm3_read_i32_key(NVM3_KEY_ENDPOINT(i), &saved_val);

        // If data exists, apply to GPIO; otherwise, default to OFF (0)
        if (has_saved_state && saved_val == 1) {
            sl_gpio_driver_set_pin(&load_gpio_cfg[i].port_pin); // ON
        } else {
            sl_gpio_driver_clear_pin(&load_gpio_cfg[i].port_pin); // OFF
        }

        LOG_INFO("NVM", "Execution: SUCCESS - Restored Index %d to %s", i, (has_saved_state && saved_val == 1) ? "ON" : "OFF");
    }
    casa_ctx.switch_interrupts = ENABLE;
}


void clear_dev_reg_preferences(void)
{
    // Reset Integer Statuses to 0 (Key 0x0001, 0x0002)
    nvm3_write_u8_key(NVM3_KEY_REG_STATUS, 0);
    nvm3_write_u8_key(NVM3_KEY_REG_MODE, 0);

    // Delete String Objects to wipe memory (Keys 0x0003 - 0x0008)
    nvm3_deleteObject(nvm3_defaultHandle, NVM3_KEY_UNIQUE_ID);
    nvm3_deleteObject(nvm3_defaultHandle, NVM3_KEY_HOST_NAME);
    nvm3_deleteObject(nvm3_defaultHandle, NVM3_KEY_WIFI_SSID);
    nvm3_deleteObject(nvm3_defaultHandle, NVM3_KEY_WIFI_PASSWORD);
    nvm3_deleteObject(nvm3_defaultHandle, NVM3_KEY_USERID);
    nvm3_deleteObject(nvm3_defaultHandle, NVM3_KEY_LOCATION_ID);

    // Reset Switch and Secure States (Keys 0x0301, 0x0302)
    nvm3_write_u8_key(NVM3_KEY_SW_STATE, 0);
    nvm3_write_u8_key(NVM3_KEY_SECURE, 0);

    // Industrial Standard: Repack to reclaim flash space immediately
    nvm3_repack(nvm3_defaultHandle);

    LOG_INFO("NVM", "Execution: SUCCESS - Keys 0x0001-0x0302 have been cleared/deleted.");
}


//void set_eeprom_fota_update_info(uint8_t stat)
//{
//    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_FOTA_STATUS, &stat, sizeof(uint8_t));
//    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_FOTA_URL, &casa_ctx.url_src, sizeof(int32_t));
//    if (stat == FOTA_DOWN_SUCCESS) nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_OLD_FW_VER, &fw_version, sizeof(float));
//    casa_ctx.fota_status = stat;
//    LOG_INFO("NVM", "Execution: SUCCESS - FOTA Info Saved (Stat: %d | Key: 0x0401)", stat);
//}
//
//void get_eeprom_fota_update_info(void)
//{
//    nvm3_readData(nvm3_defaultHandle, NVM3_KEY_FOTA_STATUS, &casa_ctx.fota_status, sizeof(uint8_t));
//    nvm3_readData(nvm3_defaultHandle, NVM3_KEY_FOTA_URL, &casa_ctx.url_src, sizeof(int32_t));
//    nvm3_readData(nvm3_defaultHandle, NVM3_KEY_OLD_FW_VER, &casa_ctx.old_frmwre_ver, sizeof(float));
//    LOG_INFO("NVM", "Execution: SUCCESS - FOTA Status: %d, Old FW: %.1f", casa_ctx.fota_status, casa_ctx.old_frmwre_ver);
//}
//
//void save_new_wifi_credentials(void)
//{
//    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_WIFI_SSID, (uint8_t *)casa_ctx.wifi_cred_changing->new_ssid, strlen(casa_ctx.wifi_cred_changing->new_ssid) + 1);
//    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_WIFI_PASSWORD, (uint8_t *)casa_ctx.wifi_cred_changing->new_pwd, strlen(casa_ctx.wifi_cred_changing->new_pwd) + 1);
//    nvm3_readData(nvm3_defaultHandle, NVM3_KEY_WIFI_SSID, casa_ctx.ssid, sizeof(casa_ctx.ssid));
//    nvm3_readData(nvm3_defaultHandle, NVM3_KEY_WIFI_PASSWORD, casa_ctx.password, sizeof(casa_ctx.password));
//    LOG_INFO("NVM", "Execution: SUCCESS - WiFi Credentials Updated (Keys: 0x0004, 0x0005)");
//}

void set_eeprom_switch_state_info(uint8_t stat)
{
    nvm3_write_u8_key(NVM3_KEY_SW_STATE, stat);
    casa_ctx.switch_op_state = stat;
    LOG_INFO("NVM", "Execution: SUCCESS - Switch State Saved: %d (Key: 0x0301)", stat);
}

void get_eeprom_switch_state_info(void)
{
    if (!nvm3_read_u8_key(NVM3_KEY_SW_STATE, &casa_ctx.switch_op_state)) {
        casa_ctx.switch_op_state = 0;
    }

    LOG_INFO("NVM", "Execution: SUCCESS - Switch Op Status: %d", casa_ctx.switch_op_state);
}

void set_eeprom_secure_device_info(uint8_t sec)
{
    nvm3_write_u8_key(NVM3_KEY_SECURE, sec);
    casa_ctx.secure_device = sec;
    LOG_INFO("NVM", "Execution: SUCCESS - Secure Mode Saved: %d (Key: 0x0302)", sec);
}

void get_eeprom_secure_device_info(void)
{
    if (!nvm3_read_u8_key(NVM3_KEY_SECURE, &casa_ctx.secure_device)) {
        casa_ctx.secure_device = 0;
    }

    LOG_INFO("NVM", "Execution: SUCCESS - Secure Status: %d", casa_ctx.secure_device);
}

void NVS_init(void) {
    int idx = 0;
    while(idx < 3) {
        if (nvm3_initDefault() != ECODE_NVM3_OK) {
            if(idx >= 2) {
                nvm3_eraseAll(nvm3_defaultHandle);
                if (nvm3_initDefault() != ECODE_NVM3_OK) {
                    LOG_ERROR("NVM", "NVM3 init failed");
                    return;
                  }
            }
            if (nvm3_deinitDefault() != ECODE_NVM3_OK) {
                LOG_ERROR("NVM", "NVM3 close failed");
            }
        } else {
            return;
        }
        idx++;
        osDelay(1000);
    }
}

void nvm3_eeprom_init()
{

  /* Init NVM3 */
//  if (nvm3_initDefault() != ECODE_NVM3_OK) {
//    LOG_ERROR("NVM", "NVM3 init failed");
//    return;
//  }

  NVS_init();
  #if (HW_FLASH == 1)
      set_eeprom_device_HW_details();
//      set_eeprom_error_log_history();
  #endif
  get_eeprom_device_state_info();
  get_eeprom_device_HW_details();
  get_eeprom_switch_state_info();
  get_eeprom_secure_device_info();
  get_eeprom_device_reg_info();
}
