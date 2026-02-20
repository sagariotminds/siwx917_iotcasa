/*
 * bluetooth.h
 *
 *  Created on: 28-Jan-2026
 *      Author: User
 */

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#define BLE_MAX_REQ_SIZE          2000      /* BLE data maximum request recived size */

void stop_ble_service(void);
sl_status_t casa_ble_init(void);
void rsi_ble_send_json_response(const char *json_payload);
void casa_ble_process(void);

#endif /* BLUETOOTH_H_ */
