/*
 * registration.c
 *
 *  Created on: 04-Feb-2026
 *      Author: User
 */

#include "sl_si91x_driver_gpio.h"

#include "registration.h"
#include "casa_module.h"
#include "bluetooth.h"
#include "eeprom_manage.h"
#include "casa_msg_parser.h"
#include "mqtt_handler.h"
#include "wifi.h"

#define  REG_FINAL_RESP_LEN        200                             /* Registration final response maximum length */
#define  REG_REQ_RCVD_RESP_LEN     200                             /* Registration request recived response maximum length */
#define  NWK_STS_RESP_LEN          200                             /* Network Status Response maximum length */


// Macros
#define TIMER_DELAY_500MS        500
#define TIMER_DELAY_5000MS       5000
#define MAX_TICK_COUNT           10


extern bool mqtt_connection_check;
extern bool ssl_connection_error;
extern casa_wifi_status_t casa_wifi_status;
extern bool device_status_report;

extern casa_context_t casa_ctx;
extern sl_si91x_gpio_pin_config_t led_gpio_cfg[];

// --- Global Variables ---
static sl_sleeptimer_timer_handle_t registration_periodic_timer;
static sl_sleeptimer_timer_handle_t cloud_reg_resp_wait_timer;
static sl_sleeptimer_timer_handle_t reg_fota_periodic_timer;

float fw_version = 3.7;

// --- Internal Timer Callback (Interrupt Context) ---
void register_data_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)data, (void)handle;
  LOG_INFO("REG", "Waiting for registration request from mobile app: %ld\r\n", casa_ctx.registration->time_out);
  if (casa_ctx.registration->time_out == SWITCH_ENABLE_TIME) {
      casa_ctx.switch_interrupts = ENABLE;
      casa_ctx.reg_dereg_flag = false;
  }

  sl_gpio_driver_toggle_pin(&led_gpio_cfg[0].port_pin);
//  UDP_receive_callback();

  if (casa_ctx.registration->buffer_status == true) {
      casa_ctx.registration->current_op = REG_REQ_PARSE;
      if (sl_sleeptimer_stop_timer(&registration_periodic_timer) == SL_STATUS_OK) {
          LOG_INFO("REG", "Timer Stopped due to data receive");
      }
  } else if (casa_ctx.registration->time_out == REG_TIMEOUT) {
      LOG_WARN("REG", "Registration timeout");
//      Insert_operation_history(REG_TIMEOUT_CODE);
      casa_ctx.registration->current_op = REGISTRATION_FINAL;
      if (sl_sleeptimer_stop_timer(&registration_periodic_timer) == SL_STATUS_OK) {
          LOG_INFO("REG", "Timer Stopped due to timeout");
      }
  }
  casa_ctx.registration->time_out++;
}


void cloud_reg_fota_status_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)data, (void)handle;
  LOG_INFO("REG", "Waiting for FOTA update check from cloud: %ld", casa_ctx.registration->time_out);

  if (casa_ctx.registration->cloud_mqtt_fota_status == 1) {
      casa_ctx.registration->current_op  = REGISTRATION_FINAL;
      if (sl_sleeptimer_stop_timer(&reg_fota_periodic_timer) == SL_STATUS_OK) {
//          LOG_INFO("REG", "Timer Stopped due to FOTA data receive");
      }
  } else if (casa_ctx.registration->time_out == CLOUD_REG_RESP_TIME || mqtt_connection_check == false ) {
//      Insert_operation_history(REG_CLOUD_RESP_TIMEOUT);
      casa_ctx.registration->current_op = REGISTRATION_FINAL;
      if (sl_sleeptimer_stop_timer(&reg_fota_periodic_timer) == SL_STATUS_OK) {
          LOG_INFO("REG", "MQTT SSL ceritificate FOTA resp time out");
      }
  }
//    ESP_LOGI(TAG, "fota state callback [APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
  casa_ctx.registration->time_out++;
}


void cloud_reg_status_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  (void)data, (void)handle;
  LOG_INFO("REG","Waiting for registration resp from cloud: %ld", casa_ctx.registration->time_out);
  if (casa_ctx.registration->cloud_mqtt_reg_status != 0) {
      casa_ctx.registration->current_op = REGISTRATION_PROCESS;
      if (sl_sleeptimer_stop_timer(&cloud_reg_resp_wait_timer) == SL_STATUS_OK) {
          LOG_INFO("REG", "Timer Stopped due to data receive");
      }
  } else if (casa_ctx.registration->time_out == CLOUD_REG_RESP_TIME || mqtt_connection_check == false) {
//      Insert_operation_history(REG_CLOUD_RESP_TIMEOUT);
      casa_ctx.registration->current_op = REGISTRATION_FINAL;
      if (sl_sleeptimer_stop_timer(&cloud_reg_resp_wait_timer) == SL_STATUS_OK) {
          LOG_INFO("REG", "Timer Stopped due to timeout");
      }
  }
//  LOG_INFO("REG","cloud states callback[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
  casa_ctx.registration->time_out++;
}


void stop_provisioning_timer(void)
{
  if (sl_sleeptimer_stop_timer(&registration_periodic_timer) == SL_STATUS_OK) {
    LOG_DEBUG("REG", "Timer Manually Stopped");
  }
}


bool registration_init(void)
{
    casa_ctx.switch_interrupts = DISABLE;
    casa_ctx.reg_dereg_flag = true;
    casa_ctx.registration->buffer_status = false;
    casa_ctx.registration->ble_data_counter = 0;
    memset(casa_ctx.registration->ble_buffer, 0, sizeof(casa_ctx.registration->ble_buffer));

    if (casa_ctx.reg_status != REGISTRATION_DONE) {
//        ble_init();
        casa_ble_init();
//        Insert_operation_history(INITIALIZE_AP_BLE);
        LOG_INFO("REG", "Initializing BLE");
        return true;
    } else {
        LOG_INFO("REG", "Device is already registered");
        return false;
    }
}

void casa_registration(void)
{
    char reg_req_rcv[REG_REQ_RCVD_RESP_LEN]     =  { 0 };    /* registration request recive buffer */
    char resp_network_stat[NWK_STS_RESP_LEN]    =  { 0 };    /* Network status response buffer */
    char reg_final_response[REG_FINAL_RESP_LEN] =  { 0 };    /* registration final response buffer */

    if (casa_ctx.registration == NULL) {
//        ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
        casa_ctx.registration = (registration_context_t*) malloc (sizeof(registration_context_t));
        if (casa_ctx.registration == NULL) {
            LOG_ERROR("REG", "Malloc failed");
            casa_ctx.current_operation = CASA_IDLE;
            return;
        } else {
            LOG_DEBUG("REG", "Malloc succeeded");
            memset(casa_ctx.registration, 0 , sizeof(registration_context_t));
            get_eeprom_device_HW_details();
            set_device_id_and_hostname();
            casa_ctx.registration->current_op = REGISTRATION_INIT;
            casa_ctx.registration->ble_data_counter  = 0;
        }
    }

    switch (casa_ctx.registration->current_op)
    {
        case REGISTRATION_INIT:
        {
            if ( registration_init() == false ) {
                casa_ctx.registration->current_op = REGISTRATION_FINAL;
            }

            casa_ctx.registration->time_out = 0;
            sl_status_t status = sl_sleeptimer_start_periodic_timer_ms( &registration_periodic_timer, PERIODIC_TIMER,
                                                                         register_data_callback, NULL,  0, 0);
            if (status == SL_STATUS_OK) {
                LOG_INFO("REG", "BLE receiver callback Timer Started");
             }
            casa_ctx.registration->current_op = WAIT;
            break;
        }

        case REG_REQ_PARSE:                       /* Registration request parser amd received data acknowlegement sent via UDP,BLE */
        {
            reg_req_recvd_response(reg_req_rcv);
            if (strlen(casa_ctx.registration->ble_buffer) != 0) {
                rsi_ble_send_json_response(reg_req_rcv);
                if (casa_message_parser(casa_ctx.registration->ble_buffer, strlen(casa_ctx.registration->ble_buffer))) {
                    casa_ctx.registration->current_op  = REG_WIFI_MQTT_INIT;
                    break;
                }
            }

            casa_ctx.registration->current_op  = REGISTRATION_FINAL;
            break;
        }

        case REG_WIFI_MQTT_INIT:                            /* WIFI initialized, if fails sending fail response via BLE, MQTT. */
        {                                                   /* If Wifi init success MQTT cloud initialization will happen. */
//            if (!wifi_sta(casa_ctx.ssid, casa_ctx.password)) {
          if (!wifi_sta("YOUR_AP_SSID", "YOUR_AP_PASSPHRASE")) {
                wifi_status_response(resp_network_stat);
                if (strlen(casa_ctx.registration->ble_buffer) != 0) {
                    rsi_ble_send_json_response(resp_network_stat);
                    osDelay(2000);
                } else {
//                    reg_resp_handler(resp_network_stat);
                }
                casa_ctx.registration->current_op = REGISTRATION_FINAL;
                break;
            }

            constuct_secure_mqtt_sub_pub_topic();
            construct_mqtt_lastWill_pub_topic();
            construct_mqtt_device_log_pub_topic();
            casa_ctx.registration->current_op = WAIT;

            if ( mqtt_app_start() && casa_ctx.mqtt_ssl_connection == true) {
               casa_ctx.registration->current_op = REG_DEVICE_DETAILS_SHARING;
               mqtt_connection_task_create();
            } else if (casa_ctx.mqtt_ssl_connection == false  && mqtt_connection_check == true){
               casa_ctx.registration->current_op = REGISTRATION_FOTA_UPDATE_CHECK;
            } else {
                casa_ctx.registration->current_op = REGISTRATION_FINAL;
            }
            break;
        }

        case REGISTRATION_FOTA_UPDATE_CHECK:
        {
            if (casa_ctx.mqtt_ssl_connection == false) {
                casa_ctx.registration->time_out = 0;
                sl_status_t status = sl_sleeptimer_start_periodic_timer_ms( &reg_fota_periodic_timer, PERIODIC_TIMER,
                                                                            cloud_reg_fota_status_callback, NULL,  0, 0);
                if (status == SL_STATUS_OK) {
                    LOG_INFO("REG", "Registration response callback Timer Started");
                }

                casa_ctx.registration->current_op = WAIT;
            } else {
                casa_ctx.registration->current_op = REGISTRATION_FINAL;
            }
            break;
        }

        case REG_DEVICE_DETAILS_SHARING:
        {
          if (casa_ctx.mqtt_ssl_connection == true && mqtt_connection_check == true) {
              discovery_device_details_json();
              Mqtt_publish(CLOUD_REG_MQTT_TOPIC, casa_ctx.registration->casa_device_details);
              casa_ctx.registration->time_out = 0;

              sl_status_t status = sl_sleeptimer_start_periodic_timer_ms( &cloud_reg_resp_wait_timer, PERIODIC_TIMER,
                                                                          cloud_reg_status_callback, NULL,  0, 0);
              if (status == SL_STATUS_OK) {
                  LOG_INFO("REG", "Registration response callback Timer Started");
               }
              casa_ctx.registration->current_op = WAIT;
          } else {
              casa_ctx.registration->current_op = REGISTRATION_FINAL;
          }
          break;
        }

        case REGISTRATION_PROCESS:       /* Cloud sends the registration response. Depending on cloud response, */
        {                                /* success or fail response is sent to mobile app via UDP or BLE */
            if (casa_ctx.registration->cloud_mqtt_reg_status == CLOUD_CODE_OK) {
//                LOG_INFO("REG", "Registration is success.");
//                gpio_set_level(WIFI_STATUS_LED_GPIO, LOW);
                casa_ctx.reg_status = REGISTRATION_DONE;
                create_discovery_resp_to_mobile(reg_final_response, CODE_SUCCESS, SUCCESS);

                if (strlen(casa_ctx.registration->ble_buffer) != 0) {
                    rsi_ble_send_json_response(reg_final_response);
                    osDelay(DELAY_2000S);
                } else {
//                    reg_resp_handler(reg_final_response);
//                    osDelay(DELAY_2000S);
                }

                set_eeprom_device_reg_info();
                device_status_report = 0;
//                mqtt_connection_task_create();
                casa_ctx.registration->current_op = REGISTRATION_FINAL;
            } else {
                LOG_ERROR("REG", "Registration failed resp from cloud.");
//                Insert_operation_history(RCVD_REG_FAIL_RESP);
                casa_ctx.registration->current_op = REGISTRATION_FINAL;
            }
            break;
        }

        case REGISTRATION_FINAL:
        {
            if(casa_ctx.reg_status != REGISTRATION_DONE)
            {
                create_discovery_resp_to_mobile(reg_final_response, CODE_FAILED, FAILED);

                if (strlen(casa_ctx.registration->ble_buffer) != 0) {
                    rsi_ble_send_json_response(reg_final_response);
                    osDelay(DELAY_2000S);
//                } else {
//                    reg_resp_handler(reg_final_response);
                }
                mqtt_app_close();
                osDelay(DELAY_2000S);
//                udp_close();
//                wifi_close();
//                gpio_set_level(WIFI_STATUS_LED_GPIO, HIGH);
                mqtt_connection_check = false;
            } else if (casa_ctx.reg_status == REGISTRATION_DONE) {
//                sl_net_down(SL_NET_WIFI_AP_INTERFACE);
//                sl_wifi_stop_ap(SL_WIFI_AP_INTERFACE);
            }

            printf("*****************************reg final****************************************\r\n");
            osDelay(DELAY_2000S);
//            stop_ble_service();
//            if(casa_ctx.reg_status == REGISTRATION_DONE) {
//                udp_close();
//            }
            casa_ctx.switch_interrupts = ENABLE;
            casa_ctx.reg_dereg_flag = false;
            ssl_connection_error = false;
            if(casa_ctx.registration->reg_fota_status == 1) {
//                casa_ctx.fota_status = FOTA_INIT;
                casa_ctx.switch_interrupts = DISABLE;
                casa_ctx.current_operation = FOTA_UPDATE;
            } else {
                casa_ctx.current_operation = CASA_IDLE;
            }
            free(casa_ctx.registration);
            casa_ctx.registration = NULL;
            casa_ctx.Reset_reason = 0;
//            LOG_INFO("REG","End [APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
            break;
        }


        case WAIT:
        {
            break;
        }

        default:
        {
          break;
        }
    }
}

void reg_req_recvd_response(char response[REG_REQ_RCVD_RESP_LEN])
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "requestId", casa_ctx.registration->requestId);
    cJSON_AddStringToObject(root, "action", "action.device.reg_req_ack");
    cJSON_AddStringToObject(root, "msg", "reg request received");
    cJSON_AddStringToObject(root, "code", "0");
    char* json = cJSON_Print(root);
    cJSON_Delete(root);
    cJSON_Minify(json);
    strcpy(response,json);
    free(json);
    LOG_INFO("REG", "Received ack: %s", (char *)response);
}

void wifi_status_response(char response[NWK_STS_RESP_LEN])
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "action", NET_ACTION);
    cJSON_AddStringToObject(root, "deviceId", casa_ctx.uniqueid);
    cJSON_AddStringToObject(root, "msg", FAILED);
    cJSON_AddStringToObject(root, "code", CODE_FAILED);
    char* json = cJSON_Print(root);
    cJSON_Delete(root);
    cJSON_Minify(json);
    strcpy(response,json);
    free(json);
    LOG_INFO("REG", "wifi status response: %s",(char *)response);
}

void discovery_device_details_json(void)
{
//    volatile int live_status;                   /* current status of the casa end_points */
    uint8_t gpio_level = 0;
    char nodeid[ENDPOINT_LEN] = {'\0'};
    char float_val[5]={'\0'};

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "requestId", casa_ctx.registration->requestId);
    cJSON_AddStringToObject(root, "action", ACTION_REG);
    cJSON_AddStringToObject(root, "ownedBy", casa_ctx.userid);
    cJSON_AddStringToObject(root, "eBy", casa_ctx.event_by);
    cJSON_AddStringToObject(root, "eById", casa_ctx.event_by_id);
    cJSON_AddStringToObject(root, "locId", casa_ctx.location);
    cJSON_AddStringToObject(root, "sn", "xxxxxxxxxxxxxx");
    cJSON_AddStringToObject(root, "token", casa_ctx.registration->discovery_token);
    cJSON_AddStringToObject(root, "authorization", casa_ctx.registration->authtoken);
    cJSON_AddStringToObject(root, "protocol", PROTOCOL);
    cJSON_AddStringToObject(root, "deviceId", casa_ctx.uniqueid);
    cJSON_AddStringToObject(root, "deviceIP", (char *) casa_wifi_status.ip_str);
    cJSON_AddStringToObject(root, "ssid", (char *)casa_wifi_status.ssid);
    cJSON_AddNumberToObject(root, "signalStrength", casa_wifi_status.rssi);
    cJSON_AddStringToObject(root, "deviceType", DEVICE_TYPE);
    cJSON_AddStringToObject(root, "powerSource", POWER_SOURCE);
    cJSON_AddStringToObject(root, "manufacturer", MANUFACTURER);
    cJSON_AddStringToObject(root, "model", MODEL);
    snprintf(float_val,5,"%.1f",casa_ctx.hardware_ver);
    cJSON_AddRawToObject(root, "hwVersion", float_val);
    cJSON_AddStringToObject(root, "hwBatch", casa_ctx.hw_batch);
    snprintf(float_val,5,"%.1f",fw_version);
    cJSON_AddRawToObject(root, "fwVersion", float_val);
    cJSON_AddNumberToObject(root, "numNodes", NO_OF_ENDPOINTS);

    cJSON *payload = cJSON_CreateObject();
    cJSON *nodes = cJSON_CreateArray();
    for(int idx = 1; idx <= NO_OF_ENDPOINTS; idx++) {
//        live_status = gpio_get_level(gpio_loads[idx-1]);
        sl_gpio_driver_get_pin(&led_gpio_cfg[idx].port_pin, &gpio_level);
        snprintf(nodeid, ENDPOINT_LEN, "%s_%d", casa_ctx.uniqueid, idx);
        cJSON *node = cJSON_CreateObject();
        cJSON_AddStringToObject(node, "nodeId", nodeid);
        cJSON_AddStringToObject(node, "nodeType", NODETYPE_SWITCH);
        cJSON *traits = cJSON_CreateObject();
        cJSON *onoff = cJSON_CreateObject();
        cJSON_AddNumberToObject(onoff, "value", gpio_level);
        cJSON_AddItemToObject(traits, "OnOff", onoff);
        cJSON_AddItemToObject(node, "traits", traits);
        cJSON_AddItemToArray(nodes, node);
        memset(nodeid, '\0', ENDPOINT_LEN);
    }
    cJSON_AddItemToObject(payload, "nodes", nodes);
    cJSON_AddItemToObject(root, "payload", payload);

    char* json = cJSON_Print(root);
    cJSON_Delete(root);
    cJSON_Minify(json);
    strcpy(casa_ctx.registration->casa_device_details,json);
    free(json);
//    LOG_INFO_LARGE("REG", (char *)casa_ctx.registration->casa_device_details);
    LOG_INFO("REG", "Device discovery details prepared");
    LOG_INFO_LARGE("REG", (char *)casa_ctx.registration->casa_device_details);
}

void create_discovery_resp_to_mobile(char buf[REG_FINAL_RESP_LEN], const char *status_resp, const char *msg)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "requestId", casa_ctx.registration->requestId);
    cJSON_AddStringToObject(root, "action", "action.device.reg_resp");
    cJSON_AddStringToObject(root, "deviceId", casa_ctx.uniqueid);
    cJSON_AddStringToObject(root, "msg", msg);
    cJSON_AddStringToObject(root, "code", status_resp);

    char* json = cJSON_Print(root);
    cJSON_Delete(root);
    cJSON_Minify(json);
    strcpy(buf,json);
    free(json);
    LOG_INFO("REG","Response to mobile: %s",(char *)buf);
}
