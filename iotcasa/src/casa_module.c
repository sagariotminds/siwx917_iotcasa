/*
 * casa_module.c
 *
 *  Created on: 29-Jan-2026
 *      Author: User
 */

#include <casa_log.h>
#include "casa_module.h"
#include "mqtt_handler.h"
#include "wifi.h"
#include "registration.h"

#include "sl_status.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "sl_wifi_types.h"

#include "sl_wifi_types.h"
#include "sl_si91x_driver.h"
#include "sl_wifi_callback_framework.h"

static const char *TAG = "CASA";
casa_context_t casa_ctx = {0};
extern char mqtt_pub_topic[MQTT_TOPIC_LEN];
extern char mqtt_pub_lastWill_topic[LASTWILL_TOPIC_LEN];
extern casa_wifi_status_t casa_wifi_status;
extern sl_wifi_device_configuration_t sl_wifi_triple_mode_configuration;
extern float fw_version;


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

void send_secure_device_resp(long long requestid)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "reqId", requestid);
    cJSON_AddNumberToObject(root, "optCode", DEVICE_SECURE_MODE);
    cJSON_AddNumberToObject(root, "sec", casa_ctx.secure_device);
    cJSON_AddStringToObject(root, "dId", casa_ctx.uniqueid);
    cJSON_AddStringToObject(root, "uId", casa_ctx.userid);

    char* json = cJSON_Print(root);
    cJSON_Delete(root);
    cJSON_Minify(json);

    LOG_INFO(TAG,"send secure device resp : %s",(char *)json);
    Mqtt_publish(mqtt_pub_topic, json);
    free(json);
}

void construct_device_status_resp(long long requestid)
{
    char float_val[5]={'\0'};
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "reqId", requestid);
    cJSON_AddNumberToObject(root, "optCode", LASTWILL_MSG);
    cJSON_AddStringToObject(root, "deviceId", casa_ctx.uniqueid);
    cJSON_AddStringToObject(root, "userId", casa_ctx.userid);
    cJSON_AddStringToObject(root, "locId", casa_ctx.location);
    cJSON_AddNumberToObject(root, "status", MQTT_ONLINE);
    cJSON_AddNumberToObject(root, "sec", casa_ctx.secure_device);
    cJSON_AddStringToObject(root, "ip", (char *) casa_wifi_status.ip_str);
    cJSON_AddStringToObject(root, "ssid", (char *)casa_wifi_status.ssid);
    cJSON_AddNumberToObject(root, "ss", casa_wifi_status.rssi);
    snprintf(float_val,5,"%.1f",casa_ctx.hardware_ver);
    cJSON_AddRawToObject(root, "hwVersion", float_val);
    memset(float_val, '\0', 5);
    snprintf(float_val,5,"%.1f",fw_version);
    cJSON_AddRawToObject(root, "fwVersion", float_val);
    cJSON_AddStringToObject(root, "hwBatch", casa_ctx.hw_batch);

    cJSON_AddStringToObject(root, "swType", "SiWX917_IOTCASA");
    cJSON_AddStringToObject(root, "DOF", "15032026");
    cJSON_AddNumberToObject(root, "memory", xPortGetFreeHeapSize());

    char* json = cJSON_Print(root);
    cJSON_Delete(root);
    cJSON_Minify(json);
    LOG_INFO(TAG,"construct device status resp JSON : %s",(char *)json);
    Mqtt_publish(mqtt_pub_lastWill_topic, json);
    free(json);
}

