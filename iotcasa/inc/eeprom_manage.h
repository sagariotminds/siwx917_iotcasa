/*
 * eeprom_manage.h
 *
 *  Created on: 06-Feb-2026
 *      Author: User
 */

#ifndef EEPROM_MANAGE_H_
#define EEPROM_MANAGE_H_

#include "iotcasa_types.h"


/**
 * @brief     Retrieve EEPROM data during device bootup
 *            (device state info and registration info).
 * @param[in] None
 * @return    None
 */
void eeprom_init(void);

/**
 * @brief     Initialize Non-Volatile Storage (NVS).
 * @param[in] None
 * @return    None
 */
void NVS_init(void);

/**
 * @brief     Open the Non-Volatile Storage (NVS) handle.
 * @param[in] namespace File name to store the values.
 * @return    None
 */
void open_nvs_handle(char *namespace);

/**
 * @brief     Commit written value. After setting any values, nvs_commit()
 *            must be called to ensure changes are written to flash storage.
 *            Close the Non-Volatile Storage (NVS) handle.
 * @param[in] None
 * @return    None
 */
void close_nvs_handle(void);

/**
 * @brief     Get EEPROM data based on Integer, Float, or String.
 * @param[in] key    Name of the key.
 * @param[in] value  EEPROM value stored in value.
 * @param[in] type   Data type (string, float, or int).
 * @return    Integer indicating the status of the operation.
 */
int get_eeprom_data(char *key, void *value, uint8_t type);

/**
 * @brief     Set device registration data information such as registration status,
 *            WiFi credentials (SSID, password), user ID, and location from the EEPROM.
 * @param[in] None
 * @return    None
 */
void set_eeprom_device_reg_info(void);

/**
 * @brief     Retrieve device registration data information such as registration status,
 *            WiFi credentials (SSID, password), user ID, and location from the EEPROM.
 * @param[in] None
 * @return    None
 */
void get_eeprom_device_reg_info(void);

/**
 * @brief     Set CASA device hardware version and batch number.
 * @param[in] None
 * @return    None
 */
void set_eeprom_device_HW_details(void);

/**
 * @brief     Get the CASA device hardware version and batch number.
 * @param[in] None
 * @return    None
 */
void get_eeprom_device_HW_details(void);

/**
 * @brief     Save the device endpoint control status.
 * @param[in] index Index of the endpoint macro.
 * @return    None
 */
void save_device_status(int index);

/**
 * @brief     Retrieve device endpoint status information from the EEPROM using preferences.
 *            Set the endpoints to their previous states using digitalWrite based on the retrieved information.
 * @param[in] None
 * @return    None
 */
void get_eeprom_device_state_info(void);

/**
 * @brief     Clear device registration preferences.
 * @param[in] None
 * @return    None
 */
void clear_dev_reg_preferences(void);

/**
 * @brief     Set device FOTA update information such as FOTA status.
 * @param[in] stat FOTA state variable.
 * @return    None
 */
void set_eeprom_fota_update_info(uint8_t stat);

/**
 * @brief     Get device FOTA update information such as FOTA status.
 * @param[in] None
 * @return    None
 */
void get_eeprom_fota_update_info(void);

/**
 * @brief     Handle successful change of CASA WiFi credentials.
 *            New WiFi credentials are saved in memory.
 * @param[in] None
 * @return    None
 */
void save_new_wifi_credentials(void);

/**
 * @brief     Set the CASA switch state operation for toggle or normal functionality received from the cloud.
 * @param[in] state Toggle or normal operation.
 * @return    None
 */
void set_eeprom_switch_state_info(uint8_t stat);

/**
 * @brief     Get and set the CASA switch state operation for toggle or normal functionality.
 * @param[in] None
 * @return    None
 */
void get_eeprom_switch_state_info(void);

/**
 * @brief     Set the bell switch secure while deregistering the end device manually.
 * @param[in] sec Toggle or normal operation.
 * @return    None
 */
void set_eeprom_secure_device_info(uint8_t sec);

/**
 * @brief     Get and set the bell switch secure while deregistering the end device manually.
 * @param[in] None
 * @return    None
 */
void get_eeprom_secure_device_info(void);

/**
 * @brief     Set the timer control state for a specific node.
 * @details   This function saves the state (on/off) of the timer for the specified node in EEPROM.
 * @param[in] node   Node ID for which the timer state is being set.
 * @param[in] state  The state of the timer (0 = off, 1 = on).
 * @return    None
 */
void set_timer_control(int node,int state);

/**
 * @brief     Get the timer control state for a specific node.
 * @details   This function loads the state (on/off) of the timer for the specified node from EEPROM.
 * @param[in] node   Node ID for which the timer state is being retrieved.
 * @return    0 (off) or 1 (on) depending on the stored state.
 */
int get_timer_control(int node);

void get_eeprom_device_HW_details(void);
void get_eeprom_device_HW_details(void);
void nvm3_eeprom_init();

#endif /* EEPROM_MANAGE_H_ */
