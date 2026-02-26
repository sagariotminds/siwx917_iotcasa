/*
 * device_control.c
 *
 *  Created on: 12-Feb-2026
 *      Author: User
 */

#include "device_control.h"
#include "gpio_control.h"
#include "mqtt_handler.h"
#include "eeprom_manage.h"
#include "wifi.h"

static const char *TAG = "Device Control";

extern casa_context_t casa_ctx;
extern sl_si91x_gpio_pin_config_t load_gpio_cfg[];
extern char mqtt_pub_topic[MQTT_TOPIC_LEN];
extern time_based_control_t gpio_timer[NO_OF_ENDPOINTS];
extern bool timer_ctrl_status[NO_OF_ENDPOINTS];
extern bool mqtt_connection_check;
extern bool internet_status;
extern casa_wifi_status_t casa_wifi_status;
device_control_context_t device_control = { 0 };

void casa_device_status_update(void)
{
  if (casa_ctx.reg_status == REGISTRATION_DONE) {
      int endpoint = device_control.endpoint_info[0].endpoint - 1;
      if(mqtt_connection_check == true ) {
          device_control.call_type = 0;
          if(gpio_timer[endpoint].active == true) {
              gpio_timer[endpoint].active = false;
              set_timer_control(endpoint + 1,0);
              if( timer_ctrl_status[endpoint]== true ) {
                  strcpy(device_control.msg_from, TIMER_CONTROL);
                  send_timer_resp(0,2, endpoint + 1,3);
                  LOG_ERROR(TAG, "Endpoint : %d timer successfully completed %lld sec ",endpoint + 1,gpio_timer[endpoint].duration_ticks);
              } else {
                  send_timer_resp(0,2, endpoint + 1,4);
                  LOG_ERROR(TAG, "Endpoint : %d timer cancelled",endpoint + 1);
              }
              timer_ctrl_status[endpoint]= false;
          }
        } else if(gpio_timer[endpoint].active == true){
            gpio_timer[endpoint].active = false;
            if( timer_ctrl_status[endpoint]== true ) {
                LOG_ERROR(TAG, "Endpoint : %d timer successfully completed %lld sec ",endpoint + 1,gpio_timer[endpoint].duration_ticks);
            } else {
                LOG_ERROR(TAG, "Endpoint : %d timer cancelled",endpoint + 1);
            }
        }
        if(internet_status == true) {
            construct_status_update_json();
        }
    }


    for(int idx = 0; idx < device_control.endpoints_counter; idx++) {
        save_device_status(idx);
    }

    if (casa_ctx.registration  != NULL) {
        casa_ctx.current_operation = REGISTRATION;        /*If the device is under registration, giving control to main loop registration */
    } else if (casa_ctx.de_registration  != NULL) {
        casa_ctx.current_operation = DEREGISTRATION;      /*If the device is under Deregistration, giving control to main loop Deregistration */
    } else {
        casa_ctx.current_operation = CASA_IDLE;
    }

    if (casa_ctx.reg_dereg_flag == 1) {
        if (casa_ctx.reg_status == REGISTRATION_DONE) {
            casa_ctx.current_operation = DEREGISTRATION;
        } else {
            casa_ctx.current_operation = REGISTRATION;
        }
    }
}

void control_endpoint(void)
{
    for (int idx = 0; idx < device_control.endpoints_counter; idx++) {
        if(device_control.endpoint_info[idx].set_value) {
            sl_gpio_driver_set_pin(&load_gpio_cfg[(device_control.endpoint_info[idx].endpoint )].port_pin);
        } else {
            sl_gpio_driver_clear_pin(&load_gpio_cfg[(device_control.endpoint_info[idx].endpoint)].port_pin);
        }
    }
    vTaskDelay(DELAY_50S / portTICK_PERIOD_MS);
    return;
}

void construct_status_update_json(void)
{
    char enpoint[ENDPOINT_LEN] = { '\0' };
    uint8_t gpio_level = 0;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "requestId", device_control.requestId);
    cJSON_AddNumberToObject(root, "optCode", DEVICE_CONTROL_RESP);
    cJSON_AddStringToObject(root, "action", "statusUpdate");
    cJSON_AddStringToObject(root, "deviceId", casa_ctx.uniqueid);
    cJSON_AddStringToObject(root, "deviceIP", (char *) casa_wifi_status.ip_str);
    cJSON_AddStringToObject(root, "triggerTyp", device_control.msg_from);
    cJSON_AddStringToObject(root, "ownedBy", casa_ctx.userid);

    if (!strcmp(device_control.msg_from, MANUAL_CONTROL) || !strcmp(device_control.msg_from, TIMER_CONTROL)) {
        cJSON_AddStringToObject(root, "eBy", device_control.msg_from);
        cJSON_AddStringToObject(root, "eById", casa_ctx.userid);
    } else {
        cJSON_AddStringToObject(root, "eBy", casa_ctx.event_by);
        cJSON_AddStringToObject(root, "eById", casa_ctx.event_by_id);
    }
    cJSON_AddNumberToObject(root, "cType", device_control.call_type);
    cJSON_AddNumberToObject(root, "isGroup", device_control.isgroup);

    cJSON *payloadArray = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "payload", payloadArray);
    for (int idx = 0; idx < device_control.endpoints_counter; idx++) {

        cJSON *firstObject = cJSON_CreateObject();
        cJSON_AddItemToArray(payloadArray, firstObject);
        cJSON_AddStringToObject(firstObject, "nodeType", NODETYPE_SWITCH);

        cJSON *traitsObject = cJSON_CreateObject();
        cJSON_AddItemToObject(firstObject, "traits", traitsObject);

        cJSON *onOffObject = cJSON_CreateObject();
        cJSON_AddItemToObject(traitsObject, "OnOff", onOffObject);

        if(device_control.call_type) {
            sprintf(enpoint, "%s_%d", casa_ctx.uniqueid, idx+1);
            sl_gpio_driver_get_pin(&load_gpio_cfg[idx+1].port_pin, &gpio_level);
            cJSON_AddNumberToObject(onOffObject, "value", gpio_level);
        }else {
            sprintf(enpoint, "%s_%d", casa_ctx.uniqueid, device_control.endpoint_info[idx].endpoint);
            //cJSON_AddNumberToObject(onOffObject, "value", device_control.endpoint_info[idx].set_value);
            sl_gpio_driver_get_pin(&load_gpio_cfg[device_control.endpoint_info[idx].endpoint].port_pin, &gpio_level);
            cJSON_AddNumberToObject(onOffObject, "value", gpio_level);
        }
        cJSON_AddStringToObject(firstObject, "nodeId", enpoint);

    }
    char* json = cJSON_Print(root);
    cJSON_Delete(root);
    cJSON_Minify(json);
//    LOG_INFO(TAG, "Device status update : %s",(char *)json);
    Mqtt_publish(mqtt_pub_topic, json);
    free(json);
}
