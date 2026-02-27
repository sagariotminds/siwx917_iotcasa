/***************************************************************************/ /**
 * @file
 * @brief Top level application functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/***************************************************************************/ /**
 * Initialize application.
 ******************************************************************************/

#include "wifi.h"
#include "casa_log.h"
#include "bluetooth.h"
#include "casa_module.h"
#include "gpio_control.h"
#include "registration.h"
#include "mqtt_handler.h"
#include "eeprom_manage.h"
#include "iotcasa_types.h"
#include "device_control.h"
#include "de_registration.h"

#include "FreeRTOS.h"
#include "task.h"

extern casa_context_t casa_ctx;

const osThreadAttr_t casa_app_thread_attributes = {
  .name       = "casa_app",
  .stack_size = 4000,
  .priority   = osPriorityRealtime,
};

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  UNUSED_PARAMETER(xTask);\
  printf("STACK OVERFLOW: %s\n", pcTaskName);
  while (1) {
      // Breakpoint here for debugging.
    }
}

void print_overall_app_memory(const char *checkpoint_name)
{
  // Total heap available in FreeRTOSConfig.h
  size_t total_heap        = configTOTAL_HEAP_SIZE;
  size_t currently_free    = xPortGetFreeHeapSize();
  size_t minimum_ever_left = xPortGetMinimumEverFreeHeapSize();
  size_t used_ram          = total_heap - currently_free;

  LOG_INFO("MEM", "========= APP MEMORY: %s =========", checkpoint_name);
  LOG_INFO("MEM", "Total Allocated Heap : %u bytes", (unsigned int)total_heap);
  LOG_INFO("MEM", "Current Used RAM     : %u bytes", (unsigned int)used_ram);
  LOG_INFO("MEM", "Current Free RAM     : %u bytes", (unsigned int)currently_free);
  LOG_INFO("MEM", "Peak RAM Usage (Max) : %u bytes", (unsigned int)(total_heap - minimum_ever_left));
  LOG_INFO("MEM", "============================================");
}

void casa_app_process_action(void *argument)
{
    UNUSED_PARAMETER(argument);
    while(1) {

      switch (casa_ctx.current_operation)
      {
          case REGISTRATION:
          {
              casa_registration();
              break;
          }
          case DEREGISTRATION:
          {
              casa_deregistration();
              break;
          }
          case STATUS_UPDATE:
          {
              casa_device_status_update();
              break;
          }
          case FOTA_UPDATE:
          {
              break;
          }
          case WIFI_CRED_CHANGE:
          {
              break;
          }
          case CASA_IDLE:
          {
              break;
          }
          default:
          {
              break;
          }
      }
      osDelay(50);    /* Delay the main Loop execution for 50 milliseconds */
    }
}

/**************** App Init ****************/
void app_init(void)
{
  LOG_INFO("APP", "========== app_init started ==========");
  /* Print memory snapshot at startup */
  print_overall_app_memory("STARTUP");
  log_mem_snapshot("app init - before start");
  casa_ctx.current_operation = CASA_IDLE;
  set_device_id_and_hostname();
  gpio_init();
  nvm3_eeprom_init();

  if (casa_ctx.reg_status == REGISTRATION_DONE) {
      wifi_sta("YOUR_AP_SSID", "YOUR_AP_PASSPHRASE");
      constuct_secure_mqtt_sub_pub_topic();
      construct_mqtt_lastWill_pub_topic();
      construct_mqtt_device_log_pub_topic();
      mqtt_connection_task_create();
  } else {
      casa_ctx.current_operation = REGISTRATION;
  }

  if (osThreadNew(casa_app_process_action, NULL, &casa_app_thread_attributes) != NULL) {
      LOG_INFO("APP", "casa app task created.");
  } else {
      LOG_ERROR("APP", "casa app task not created.");
  }
}
