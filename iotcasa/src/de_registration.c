/*
 * de_registration.c
 *
 *  Created on: 13-Feb-2026
 *  Author: User
 */

#include "de_registration.h"
#include "casa_module.h"
#include "eeprom_manage.h"
#include "mqtt_handler.h"
#include "wifi.h"


static const char *TAG = "De-Reg";

extern casa_context_t casa_ctx;
extern bool mqtt_connection_check;
extern bool ssl_connection_error;
extern bool device_status_report;

static sl_sleeptimer_timer_handle_t de_registration_periodic_timer;

void exit_de_registration(void)
{
  LOG_ERROR(TAG, "Rcvd de-registration failed resp from cloud.");
  free(casa_ctx.de_registration);
  casa_ctx.de_registration = NULL;
  casa_ctx.current_operation = CASA_IDLE;
  casa_ctx.switch_interrupts = ENABLE;
  casa_ctx.reg_dereg_flag = 0;
}

char *dereg_device_details_json(void)
{
    // 1. Create Root Object
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        LOG_ERROR(TAG, "Execution: FAILED - Root allocation");
        return NULL;
    }

    cJSON_AddNumberToObject(root, "requestId",     (double)casa_ctx.de_registration->requestId);
    cJSON_AddStringToObject(root, "action",        DEREG_ACTION);
    cJSON_AddStringToObject(root, "reqFrom",       casa_ctx.de_registration->dereg_from);
    cJSON_AddStringToObject(root, "authorization", casa_ctx.de_registration->authtoken);
//    cJSON_AddStringToObject(root, "reqFrom",       "mobile");
//    cJSON_AddStringToObject(root, "authorization", "Manual");
    cJSON_AddStringToObject(root, "deviceId",      casa_ctx.uniqueid);
    cJSON_AddStringToObject(root, "locId",         casa_ctx.location);
    cJSON_AddStringToObject(root, "ownedBy",       casa_ctx.userid);
    cJSON_AddStringToObject(root, "eBy",           casa_ctx.event_by);
    cJSON_AddStringToObject(root, "eById",         casa_ctx.event_by_id);

    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
//    memset(casa_ctx.de_registration->dereg_details, 0, sizeof(casa_ctx.de_registration->dereg_details));
//    strncpy(casa_ctx.de_registration->dereg_details, json, sizeof(casa_ctx.de_registration->dereg_details) - 1);
////    LOG_INFO(TAG, "Dereg details generated: %s", casa_ctx.de_registration->dereg_details);
//    printf("Dereg details generated: %s\r\n", casa_ctx.de_registration->dereg_details);
//    free(json);
    if (json == NULL) {
            LOG_ERROR(TAG, "Execution: FAILED - JSON print allocation");
            return NULL;
        }

        LOG_INFO(TAG, "Dereg details generated len=%u", (unsigned int)strlen(json));
        return json;
}

void cloud_dereg_status_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{

  (void)data, (void)handle;
  LOG_INFO(TAG, "Waiting for cloud dereg resp: %d", casa_ctx.de_registration->time_out);
  if (casa_ctx.de_registration->cloud_mqtt_dereg_status != 0) {
      casa_ctx.de_registration->current_op = DE_REGISTRATION_PROCESS;
      if (sl_sleeptimer_stop_timer(&de_registration_periodic_timer) == SL_STATUS_OK) {
          LOG_INFO("REG", "Timer Stopped due to data receive");
      }
      return;
  } else if (casa_ctx.de_registration->time_out == CLOUD_DE_REG_RESP_TIMEOUT) {
      LOG_WARN(TAG, "De-registration cloud resp time out.");
//        Insert_operation_history(DEREG_CLOUD_RESP_TIMEOUT);
      exit_de_registration();
      if (sl_sleeptimer_stop_timer(&de_registration_periodic_timer) == SL_STATUS_OK) {
          LOG_INFO("REG", "Timer Stopped due to timeout");
      }
      return;
  }
//    LOG_INFO(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
  casa_ctx.de_registration->time_out++;
}

void casa_deregistration(void)
{
//  ESP_LOGI(TAG, "start [APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
  if (casa_ctx.de_registration == NULL) {
      casa_ctx.de_registration = (de_registration_context_t*) malloc (sizeof(de_registration_context_t));
      if (casa_ctx.de_registration == NULL) {
          LOG_ERROR(TAG, "Malloc failed for de-registration.");
//          Insert_operation_history(MALLOC_FAILED_DEREG);
//          construct_mqtt_device_log_msg(E_MALLOC,L_DEREGISTRATION);
          return;
      } else {
          LOG_INFO(TAG, "Malloc succeeded for de-registration.");
          memset(casa_ctx.de_registration, 0 , sizeof(de_registration_context_t));
          /* TODO we have to change the mobile to manual once the mobile app completed its functionality for manual de registration. */
          sprintf(casa_ctx.de_registration->dereg_from, "%s", "mobile");
          sprintf(casa_ctx.de_registration->authtoken, "%s", "Manual");
          casa_ctx.de_registration->current_op = DE_REG_DETAILS_SHARING;
          casa_ctx.switch_interrupts = DISABLE;
      }
  }


  switch(casa_ctx.de_registration->current_op)
  {
      case DE_REG_DETAILS_SHARING:  /* Publishes de-registration request to the cloud via MQTT */
      {                             /* Waits for the Cloud response */
          if ((!mqtt_connection_check) ) {
              LOG_ERROR(TAG, "No Internet to do de-registration.");
//              Insert_operation_history(NO_WIFI_DEREG);
              if (casa_ctx.secure_device == 0) {
                  casa_ctx.de_registration->current_op = DE_REGISTRATION_FINAL;
              } else {
                  exit_de_registration();
              }
              break;
          }
//          dereg_device_details_json();
//          Mqtt_publish(DE_REG_MQTT_PUB_TOPIC, casa_ctx.de_registration->dereg_details);
          char *dereg_json = dereg_device_details_json();
          if (dereg_json == NULL) {
              LOG_ERROR(TAG, "De-registration JSON generation failed.");
              break;
          }

          if (!Mqtt_publish(DE_REG_MQTT_PUB_TOPIC, dereg_json)) {
              free(dereg_json);
              LOG_WARN(TAG, "De-registration publish failed, will retry.");
              break;
          }

          free(dereg_json);
          sl_status_t status = sl_sleeptimer_start_periodic_timer_ms( &de_registration_periodic_timer, PERIODIC_TIMER,
                                                                      cloud_dereg_status_callback, NULL,  0, 0);
          if (status == SL_STATUS_OK) {
              LOG_INFO("REG", "Cloud de-registration response receiver callback Timer Started");
          }
          casa_ctx.de_registration->current_op = WAIT_DEREG;
          break;
      }

      case DE_REGISTRATION_PROCESS: /* Processes the Mqtt cloud response, wheather the device details removed */
      {                             /* from cloud or not, depending on it next operation is performed */
          switch (casa_ctx.de_registration->cloud_mqtt_dereg_status)
          {
            case CLOUD_CODE_OK:
            case CLOUD_ERROR :
            case INVALID_USER_ID:
            case INVALID_DEVICE_ID:
            {
                casa_ctx.de_registration->current_op = DE_REGISTRATION_FINAL;
                break;
            }
            case INVALID_AUTH_TOKEN:
            {
//                  Insert_operation_history(RCVD_DEREG_FAIL_RESP);
                exit_de_registration();
                break;
            }
            case SECURE_MODE_FAILURE:
            {
                exit_de_registration();
                break;
            }
          }
          break;
      }

      case DE_REGISTRATION_FINAL: /* Clears deregistration context, casa context, disconnect Mqtt,  */
      {                           /* disconnects Wifi, clears preferences and sets operation to registration */
        LOG_INFO(TAG, "De-registration is success.");
        free(casa_ctx.de_registration);
        casa_ctx.de_registration = NULL;
        casa_ctx.reg_status = 0;
        clear_dev_reg_preferences();
        memset(&casa_ctx, 0, sizeof(casa_context_t));
//        close_multicast_UPD();
        mqtt_app_close();
        wifi_close();
        casa_ctx.reg_dereg_flag = 0;
        mqtt_connection_check = false;
        ssl_connection_error = false;
        device_status_report = 0;
//          gpio_set_level(WIFI_STATUS_LED_GPIO, HIGH);
//          ESP_LOGI(TAG, "End [APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
//        for(int idx = 1; idx <= NO_OF_ENDPOINTS; idx++)
//        {
//            if(gpio_timer[idx-1].active == true){
//                set_timer_control(idx,0);
//                timer_ctrl_status[idx-1] = false;
//                gpio_timer[idx-1].active = false;
//            }
//        }
        casa_ctx.current_operation = REGISTRATION;
//        if(mqttConnectHandle != NULL){
//           vTaskDelete(mqttConnectHandle);
//           LOG_INFO(TAG, "MQTT task deleted success.\n");
//        }
        osDelay(DELAY_500S);
        casa_ctx.switch_interrupts = ENABLE;
        break;
      }

      case WAIT_DEREG:
      {
          //ideal case
          break;
      }
  }
}
