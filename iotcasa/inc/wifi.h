/*
 * wifi.h
 *
 *  Created on: 28-Jan-2026
 *      Author: User
 */

#ifndef WIFI_H_
#define WIFI_H_

#include "sl_wifi.h"
#include "sl_net.h"
#include "sl_constants.h"
#include "sl_net_wifi_types.h"
#include "sl_net_default_values.h"
#include "sl_wifi_callback_framework.h"
#include "sl_net_si91x_integration_handler.h"
#include "sl_net_ping.h"

/* --- BSD Sockets (Si91x Unified API) --- */
#include "socket.h"
#include "sl_si91x_socket.h"
#include "sl_si91x_socket_utility.h"
#include "sl_si91x_socket_support.h"
#include "sl_si91x_socket_constants.h"

#define WIFI_MAXIMUM_RETRY          30       /* Maximum Wifi check loop count per half second*/

/* Global structure to hold WiFi status for all files to use */
typedef struct {
    char ip_str[16];      // String format: "192.168.1.100"
    char ssid[33];        // Connected SSID name
    int32_t rssi;         // Signal strength in dBm
    bool is_connected;    // Flag for other tasks (MQTT, etc.)
} casa_wifi_status_t;

void wifi_close(void);
bool wifi_sta(const char *ssid, const char *password);
bool station_wifi_detailes_get(sl_net_wifi_client_profile_t *profile);
void station_wifi_status_clear(void);
void wifi_sta_monitor_task(void *argument);
void sta_mode_application(void *argument);
void ap_mode_application(void);

#endif /* WIFI_H_ */
