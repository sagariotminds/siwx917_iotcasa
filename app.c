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

/* --- System & RTOS --- */

#include <casa_log.h>
#include <gpio_control.h>
#include "sl_wifi.h"
#include "sl_wifi_types.h"
#include "sl_si91x_driver.h"
#include "sl_wifi_callback_framework.h"

#include "sl_si91x_driver.h"

#include "iotcasa_types.h"
#include "casa_module.h"
#include "bluetooth.h"
#include "wifi.h"
#include "registration.h"
#include "de_registration.h"
#include "eeprom_manage.h"
#include "mqtt_handler.h"

extern osThreadAttr_t ble_thread_attributes;
extern osThreadAttr_t thread_attributes;
extern sl_wifi_device_configuration_t sl_wifi_triple_mode_configuration;
extern casa_context_t casa_ctx;

#include "FreeRTOS.h"
#include "task.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  UNUSED_PARAMETER(xTask);
    printf("STACK OVERFLOW: %s\n", pcTaskName);
    while(1);  // Breakpoint here
}

void print_overall_app_memory(const char* checkpoint_name) {
    // Total heap available in FreeRTOSConfig.h
    size_t total_heap = configTOTAL_HEAP_SIZE;
    size_t currently_free = xPortGetFreeHeapSize();
    size_t minimum_ever_left = xPortGetMinimumEverFreeHeapSize();
    size_t used_ram = total_heap - currently_free;

    LOG_INFO("MEM", "========= APP MEMORY: %s =========", checkpoint_name);
    LOG_INFO("MEM", "Total Allocated Heap : %u bytes", (unsigned int)total_heap);
    LOG_INFO("MEM", "Current Used RAM     : %u bytes", (unsigned int)used_ram);
    LOG_INFO("MEM", "Current Free RAM     : %u bytes", (unsigned int)currently_free);
    LOG_INFO("MEM", "Peak RAM Usage (Max) : %u bytes", (unsigned int)(total_heap - minimum_ever_left));
    LOG_INFO("MEM", "============================================");
}

void casa_app_process_action(void *argument);

const osThreadAttr_t casa_app_thread_attributes = {
  .name       = "casa_app",
  .stack_size = 8000,
  .priority   = osPriorityLow,
};

/**************** App Init ****************/
void app_init(const void *unused)
{
  UNUSED_PARAMETER(unused);
  LOG_INFO("APP", "========== app_init started ==========");

  sl_status_t status;

  status = sl_wifi_init(&sl_wifi_triple_mode_configuration, NULL, sl_wifi_default_event_handler);
  if (status != SL_STATUS_OK) {
      LOG_ERROR("APP", "Wi-Fi Initialization Failed, Error Code : 0x%lX", status);
    return;
  }

  /* Print memory snapshot at startup */
  print_overall_app_memory("STARTUP");
  casa_ctx.current_operation = CASA_IDLE;
//  osThreadNew(casa_app_process_action, NULL, &casa_app_thread_attributes);

  if (osThreadNew(casa_app_process_action, NULL, &casa_app_thread_attributes) != NULL) {
      LOG_INFO("MQTT", "casa app task created.");
  } else {
      LOG_ERROR("MQTT", "casa app task not created.");
  }
  set_device_id_and_hostname();

  gpio_init();
  nvm3_eeprom_init();
  get_eeprom_device_state_info();
  get_eeprom_device_reg_info();

//  wifi_sta("YOUR_AP_SSID", "YOUR_AP_PASSPHRASE");

  casa_ctx.current_operation = REGISTRATION;
}

void casa_app_process_action(void *argument)
{
  UNUSED_PARAMETER(argument);
  printf("This is app process action loop.\r\n");
  while(1) {
        printf("This is app process action loop.\r\n");
        casa_ble_process();

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
      //          casa_device_status_update();
                break;
            }
            case FOTA_UPDATE:
            {
      //          fota_firmware_update();
                break;
            }
            case WIFI_CRED_CHANGE:
            {
      //          wifi_credentials_change();
      //          free(casa_ctx.wifi_cred_changing);
      //          casa_ctx.wifi_cred_changing = NULL;
      //          casa_ctx.current_operation = CASA_IDLE;
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
        osDelay(100);    /* Delay the main Loop execution for 50 milliseconds */
  }

}
