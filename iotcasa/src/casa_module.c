/*
 * casa_module.c
 *
 *  Created on: 29-Jan-2026
 *      Author: User
 */

#include <casa_log.h>
#include "casa_module.h"

#include "sl_status.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "sl_wifi_types.h"

#include "sl_wifi_types.h"
#include "sl_si91x_driver.h"
#include "sl_wifi_callback_framework.h"

casa_context_t casa_ctx = {0};
extern sl_wifi_device_configuration_t sl_wifi_triple_mode_configuration;

void set_device_id_and_hostname() {
    sl_status_t status;
    sl_mac_address_t mac_addr;

    status = sl_wifi_init(&sl_wifi_triple_mode_configuration, NULL, sl_wifi_default_event_handler);
    if (status != SL_STATUS_OK) {
        LOG_ERROR("APP", "Wi-Fi Initialization Failed, Error Code : 0x%lX", status);
      return;
    }

    // 1. Get MAC Address (The "EFUSE" equivalent in Si91x)
    status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_addr);

    if (status != SL_STATUS_OK) {
        LOG_ERROR("MAC", "Failed to get MAC address: 0x%lX", status);
        return;
    }

    // 2. Print Board MAC
    LOG_INFO("MAC", "Board MAC address: %02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr.octet[0], mac_addr.octet[1], mac_addr.octet[2],
           mac_addr.octet[3], mac_addr.octet[4], mac_addr.octet[5]);

    // 3. Create UniqueID (Matches your ESP logic: "00" + last 3 bytes of MAC)
    snprintf(casa_ctx.uniqueid, UNIQEID_LEN, "00%02X%02X%02X",
             mac_addr.octet[3], mac_addr.octet[4], mac_addr.octet[5]);
    LOG_INFO("MAC", "UniqueId :: %s", casa_ctx.uniqueid);

    // 4. Create Hostname (MODEL + UniqueID)
    snprintf(casa_ctx.hostname, HOSTNAME_LEN, "%s_%s", MODEL, casa_ctx.uniqueid);
    LOG_INFO("MAC", "HostName :: %s", casa_ctx.hostname);
}
