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
#define KEY_COUNTER   1
#define KEY_STRING    2
#define KEY_FLOAT     3

extern float fw_version;
extern casa_context_t casa_ctx;
extern sl_si91x_gpio_pin_config_t led_gpio_cfg[];
extern device_control_context_t device_control;

uint32_t persistent_counter = 0;
char persistent_string[64];
float persistent_float = 0.0f;


// --- Your Original Functions ---

//void NVS_init(void) {
//    int idx = 0;
//    while(idx < 3) {
//        Ecode_t err = nvm3_open(nvm3_defaultHandle, nvm3_defaultInit);
//        if (err != ECODE_NVM3_OK) {
//            if(idx >= 2) {
//                nvm3_eraseAll(nvm3_defaultHandle);
//                nvm3_open(nvm3_defaultHandle, nvm3_defaultInit);
//            }
//        } else {
//            return;
//        }
//        idx++;
//        osDelay(1000);
//    }
//}

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


void get_eeprom_device_reg_info(void)
{
    sl_status_t status;

    // 1. Read Registration Status (Integer)
    status = nvm3_readData(nvm3_defaultHandle, NVM3_KEY_REG_STATUS, &casa_ctx.reg_status, sizeof(casa_ctx.reg_status));

    if (status != SL_STATUS_OK) {
        LOG_ERROR("NVS", "Execution: FAILED to read Reg Status (0x%lx)", status);
        casa_ctx.reg_status = !REGISTRATION_DONE;
    }

    // 2. Read Strings
    // We pass the size of our destination buffers to prevent memory overflow
    nvm3_readData(nvm3_defaultHandle, NVM3_KEY_UNIQUE_ID, casa_ctx.uniqueid, sizeof(casa_ctx.uniqueid));
    nvm3_readData(nvm3_defaultHandle, NVM3_KEY_HOST_NAME, casa_ctx.hostname, sizeof(casa_ctx.hostname));
    nvm3_readData(nvm3_defaultHandle, NVM3_KEY_WIFI_SSID, casa_ctx.ssid, sizeof(casa_ctx.ssid));
    nvm3_readData(nvm3_defaultHandle, NVM3_KEY_WIFI_PASSWORD, casa_ctx.password, sizeof(casa_ctx.password));
    nvm3_readData(nvm3_defaultHandle, NVM3_KEY_USERID, casa_ctx.userid, sizeof(casa_ctx.userid));
    nvm3_readData(nvm3_defaultHandle, NVM3_KEY_LOCATION_ID, casa_ctx.location, sizeof(casa_ctx.location));

    // 3. Logic: If Registration was not done, check if we have enough info to complete it
    if (casa_ctx.reg_status != REGISTRATION_DONE) {
        LOG_WARN("NVS", "Execution: Reg not done, checking acquired strings...");

        if (strlen(casa_ctx.uniqueid) > 0 && strlen(casa_ctx.ssid) > 0 && strlen(casa_ctx.password) > 0) {
            casa_ctx.reg_status = REGISTRATION_DONE;
            nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_REG_STATUS, &casa_ctx.reg_status, sizeof(int32_t));
            LOG_INFO("NVS", "Execution: SUCCESS - Registration auto-completed");
        }
    } else {
        // 4. Logic: Sanity check - if status says DONE but all strings are empty, reset status
        if (strlen(casa_ctx.uniqueid) == 0 && strlen(casa_ctx.ssid) == 0) {
            casa_ctx.reg_status = !REGISTRATION_DONE;
            nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_REG_STATUS, &casa_ctx.reg_status, sizeof(int32_t));
            LOG_ERROR("NVS", "Execution: Status mismatch! Strings empty. Resetting status.");
        }
    }

    // Final Log with Full Data (Using your Total Line Color logic)
    LOG_INFO("NVS", "Device Reg Info: Stat:%d, UID:%s, Host:%s, SSID:%s, User:%s", casa_ctx.reg_status, casa_ctx.uniqueid, casa_ctx.hostname, casa_ctx.ssid, casa_ctx.userid);
}

void set_eeprom_device_reg_info(void)
{
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_REG_STATUS, (uint8_t *)&casa_ctx.reg_status, sizeof(int32_t));
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_UNIQUE_ID, (uint8_t *)casa_ctx.uniqueid, strlen(casa_ctx.uniqueid) + 1);
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_HOST_NAME, (uint8_t *)casa_ctx.hostname, strlen(casa_ctx.hostname) + 1);
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_WIFI_SSID, (uint8_t *)casa_ctx.ssid, strlen(casa_ctx.ssid) + 1);
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_WIFI_PASSWORD, (uint8_t *)casa_ctx.password, strlen(casa_ctx.password) + 1);
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_USERID, (uint8_t *)casa_ctx.userid, strlen(casa_ctx.userid) + 1);
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_LOCATION_ID, (uint8_t *)casa_ctx.location, strlen(casa_ctx.location) + 1);
    LOG_INFO("NVM", "Execution: SUCCESS - Device Reg Info Saved");
}

void get_eeprom_device_HW_details(void)
{
    nvm3_readData(nvm3_defaultHandle, NVM3_KEY_HW_VER, &casa_ctx.hardware_ver, sizeof(float));
    nvm3_readData(nvm3_defaultHandle, NVM3_KEY_HW_BATCH, casa_ctx.hw_batch, sizeof(casa_ctx.hw_batch));
    LOG_INFO("NVM", "Execution: SUCCESS - HW Details Loaded (Ver: %.2f | Batch: %s)", casa_ctx.hardware_ver, casa_ctx.hw_batch);
}

void set_eeprom_device_HW_details(void)
{
    float my_float_value = HW_VERSION;
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_HW_VER, (uint8_t *)&my_float_value, sizeof(float));
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_HW_BATCH, (uint8_t *)HW_BATCH, strlen(HW_BATCH) + 1);
    LOG_INFO("NVM", "Execution: SUCCESS - HW Details Saved (Ver: %.2f | Batch: %s)", my_float_value, HW_BATCH);
}

void set_timer_control(int node, int state)
{
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_TIMER(node), (uint8_t *)&state, sizeof(int32_t));
    LOG_INFO("NVM", "Execution: SUCCESS - Timer Node %d State Saved: %d (Key: 0x%04X)", node, state, NVM3_KEY_TIMER(node));
}

int get_timer_control(int node)
{
    int32_t state = 0;
    // Read directly using the calculated key number
    sl_status_t status = nvm3_readData(nvm3_defaultHandle, NVM3_KEY_TIMER(node), &state, sizeof(int32_t));

    if (status != SL_STATUS_OK) {
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
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_ENDPOINT(index + 1), (uint8_t *)&val, sizeof(int32_t));

    if (casa_ctx.reg_dereg_flag == 0) casa_ctx.switch_interrupts = ENABLE;
    LOG_INFO("NVM", "Execution: SUCCESS - Saved Index %d [Value: %ld]", index, val);
}

void get_eeprom_device_state_info(void)
{
    int32_t saved_val = 0;
    for (int i = 0; i < NO_OF_ENDPOINTS; i++) {
        // Read state from NVM3
        sl_status_t status = nvm3_readData(nvm3_defaultHandle, NVM3_KEY_ENDPOINT(i + 1), &saved_val, sizeof(int32_t));

        // If data exists, apply to GPIO; otherwise, default to OFF (0)
        if (status == SL_STATUS_OK && saved_val == 1) {
            sl_gpio_driver_set_pin(&led_gpio_cfg[i].port_pin); // ON
        } else {
            sl_gpio_driver_clear_pin(&led_gpio_cfg[i].port_pin); // OFF
        }

        LOG_INFO("NVM", "Execution: SUCCESS - Restored Index %d to %s", i, (saved_val == 1) ? "ON" : "OFF");
    }
    casa_ctx.switch_interrupts = ENABLE;
}


void clear_dev_reg_preferences(void)
{
    int32_t reset_val = 0;

    // Reset Integer Statuses to 0 (Key 0x0001, 0x0002)
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_REG_STATUS, &reset_val, sizeof(int32_t));
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_REG_MODE, &reset_val, sizeof(int32_t));

    // Delete String Objects to wipe memory (Keys 0x0003 - 0x0008)
    nvm3_deleteObject(nvm3_defaultHandle, NVM3_KEY_UNIQUE_ID);
    nvm3_deleteObject(nvm3_defaultHandle, NVM3_KEY_HOST_NAME);
    nvm3_deleteObject(nvm3_defaultHandle, NVM3_KEY_WIFI_SSID);
    nvm3_deleteObject(nvm3_defaultHandle, NVM3_KEY_WIFI_PASSWORD);
    nvm3_deleteObject(nvm3_defaultHandle, NVM3_KEY_USERID);
    nvm3_deleteObject(nvm3_defaultHandle, NVM3_KEY_LOCATION_ID);

    // Reset Switch and Secure States (Keys 0x0301, 0x0302)
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_SW_STATE, &reset_val, sizeof(int32_t));
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_SECURE, &reset_val, sizeof(int32_t));

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
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_SW_STATE, &stat, sizeof(uint8_t));
    casa_ctx.switch_op_state = stat;
    LOG_INFO("NVM", "Execution: SUCCESS - Switch State Saved: %d (Key: 0x0301)", stat);
}

void get_eeprom_switch_state_info(void)
{
    nvm3_readData(nvm3_defaultHandle, NVM3_KEY_SW_STATE, &casa_ctx.switch_op_state, sizeof(uint8_t));
    LOG_INFO("NVM", "Execution: SUCCESS - Switch Op Status: %d", casa_ctx.switch_op_state);
}

void set_eeprom_secure_device_info(uint8_t sec)
{
    nvm3_writeData(nvm3_defaultHandle, NVM3_KEY_SECURE, &sec, sizeof(uint8_t));
    casa_ctx.secure_device = sec;
    LOG_INFO("NVM", "Execution: SUCCESS - Secure Mode Saved: %d (Key: 0x0302)", sec);
}

void get_eeprom_secure_device_info(void)
{
    nvm3_readData(nvm3_defaultHandle, NVM3_KEY_SECURE, &casa_ctx.secure_device, sizeof(uint8_t));
    LOG_INFO("NVM", "Execution: SUCCESS - Secure Status: %d", casa_ctx.secure_device);
}




const osThreadAttr_t nvm3_thread_attributes = {
  .name       = "nvm",
  .stack_size = 3072,
  .priority   = osPriorityLow,
};


void clear_counter(void)
{
  persistent_counter = 0;
  nvm3_writeData(NVM3_DEFAULT_HANDLE, KEY_COUNTER, &persistent_counter, sizeof(persistent_counter));
  LOG_DEBUG("NVM", "Counter cleared (value set to 0)");
}

void delete_counter(void)
{
  nvm3_deleteObject(NVM3_DEFAULT_HANDLE, KEY_COUNTER);
  persistent_counter = 0;
  nvm3_writeData(NVM3_DEFAULT_HANDLE, KEY_COUNTER, &persistent_counter, sizeof(persistent_counter));
  LOG_DEBUG("NVM", "Counter cleared and reset to 0");
}

void clear_string_null(void)
{
  /* Delete the string key */
  nvm3_deleteObject(NVM3_DEFAULT_HANDLE, KEY_STRING);

  /* Store empty string back */
  nvm3_writeData(NVM3_DEFAULT_HANDLE,KEY_STRING,"",0);

  LOG_DEBUG("NVM", "String cleared and set to NULL");
}

/******************************************************
 * EEPROM
 ******************************************************/
void save_all(void)
{
  nvm3_writeData(NVM3_DEFAULT_HANDLE, KEY_COUNTER,
                 &persistent_counter, sizeof(persistent_counter));
  nvm3_writeData(NVM3_DEFAULT_HANDLE, KEY_STRING,
                 persistent_string, strlen(persistent_string));
  nvm3_writeData(NVM3_DEFAULT_HANDLE, KEY_FLOAT,
                 &persistent_float, sizeof(persistent_float));
}

void load_all(void)
{
  uint32_t type;
  size_t len;
  Ecode_t err;

  /* Counter */
  err = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, KEY_COUNTER, &type, &len);
  if (err == ECODE_NVM3_OK && type == NVM3_OBJECTTYPE_DATA)
    nvm3_readData(NVM3_DEFAULT_HANDLE, KEY_COUNTER, &persistent_counter, sizeof(persistent_counter));
  else
    persistent_counter = 0;

  /* String */
  err = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, KEY_STRING, &type, &len);
  if (err == ECODE_NVM3_OK && type == NVM3_OBJECTTYPE_DATA) {
    nvm3_readData(NVM3_DEFAULT_HANDLE, KEY_STRING, persistent_string, len);
    persistent_string[len] = '\0';
  } else {
    strcpy(persistent_string, "IOTCASA");
  }

  /* Float */
  err = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, KEY_FLOAT, &type, &len);
  if (err == ECODE_NVM3_OK && type == NVM3_OBJECTTYPE_DATA)
    nvm3_readData(NVM3_DEFAULT_HANDLE, KEY_FLOAT, &persistent_float, sizeof(persistent_float));
  else
    persistent_float = 1.23f;

  /* Save defaults if first boot */
  save_all();

  LOG_INFO("NVM", "Counter = %lu", persistent_counter);
  LOG_INFO("NVM", "String  = %s", persistent_string);
  LOG_INFO("NVM", "Float   = %.2f", persistent_float);
}

//static void nvm3_application_start(void *argument)
//{
//  UNUSED_PARAMETER(argument);
//
//  LOG_INFO("NVM", "=== SiWx917 EEPROM ===");
//
//
//  /* Init NVM3 */
//  if (nvm3_initDefault() != ECODE_NVM3_OK) {
//    LOG_ERROR("NVM", "NVM3 init failed");
//    return;
//  }
//
//  load_all();
//
//  while (1) {
//    osDelay(5000);
//    persistent_counter++;
//    persistent_float += 1.5f;
//    strcpy(persistent_string, "SiWx917");
//    save_all();
//    LOG_DEBUG("NVM", "Saved → %lu, %s, %.2f",
//           persistent_counter, persistent_string, persistent_float);
//  }
//}

void nvm3_eeprom_init()
{

  /* Init NVM3 */
  if (nvm3_initDefault() != ECODE_NVM3_OK) {
    LOG_ERROR("NVM", "NVM3 init failed");
    return;
  }
//  osThreadNew(nvm3_application_start, NULL, &nvm3_thread_attributes);
}
