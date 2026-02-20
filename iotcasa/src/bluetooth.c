/*
 * bluetooth.c
 *
 *  Created on: 28-Jan-2026
 *      Author: User
 */



/* --- BLE Stack (Si91x / RSI APIs) --- */
#include <casa_log.h>
#include "rsi_ble_apis.h"
#include "rsi_bt_common_apis.h"
#include "rsi_bt_common.h"
#include "rsi_common_apis.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"

/* --- Low Level Drivers --- */
#include "sl_si91x_driver.h"

#include "bluetooth.h"
#include "casa_module.h"
#include "casa_msg_parser.h"

/******************************************************
*               BLE Constants
******************************************************/

#define BLE_ATT_REC_SIZE 500
#define NO_OF_VAL_ATT    5

//! BLE attribute service types uuid values
#define RSI_BLE_CHAR_SERV_UUID   0x2803
#define RSI_BLE_CLIENT_CHAR_UUID 0x2902

//! BLE characteristic service uuid

#define RSI_BLE_NEW_SERVICE_UUID 0xBAAD
#define RSI_BLE_ATTRIBUTE_1_UUID 0xF00D
//! local device name

//! MTU size for the link
#define RSI_BLE_MTU_SIZE 240

//! attribute properties
#define RSI_BLE_ATT_PROPERTY_READ   0x02
#define RSI_BLE_ATT_PROPERTY_WRITE  0x08
#define RSI_BLE_ATT_PROPERTY_NOTIFY 0x10

//! Configuration bitmap for attributes
#define RSI_BLE_ATT_MAINTAIN_IN_HOST BIT(0)

#define RSI_BLE_ATT_CONFIG_BITMAP (RSI_BLE_ATT_MAINTAIN_IN_HOST)

//! application event list
#define RSI_BLE_CONN_EVENT                    0x01
#define RSI_BLE_DISCONN_EVENT                 0x02
#define RSI_BLE_GATT_WRITE_EVENT              0x03
#define RSI_BLE_READ_REQ_EVENT                0x04
#define RSI_BLE_GATT_ERROR                    0x05

const osThreadAttr_t ble_thread_attributes = {
  .name       = "BLE_thread",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 8000,
  .priority   = osPriorityNormal,
  .tz_module  = 0,
  .reserved   = 0,
};

typedef struct rsi_ble_att_list_s {
  uuid_t char_uuid;
  uint16_t handle;
  uint16_t len;
  uint16_t max_value_len;
  uint8_t char_val_prop;
  void *value;
} rsi_ble_att_list_t;

typedef struct rsi_ble_s {
  uint8_t DATA[BLE_ATT_REC_SIZE];
  uint16_t DATA_ix;
  uint16_t att_rec_list_count;
  rsi_ble_att_list_t att_rec_list[NO_OF_VAL_ATT];
} rsi_ble_t;

rsi_ble_t att_list;

//! global parameters list
static volatile uint32_t ble_app_event_map;
static uint8_t remote_dev_bd_addr[6] = { 0 };
static rsi_ble_event_conn_status_t conn_event_to_app;
static rsi_ble_event_disconnect_t disconn_event_to_app;
static rsi_ble_event_write_t app_ble_write_event;
static uint16_t rsi_ble_att1_val_hndl;
static rsi_ble_read_req_t app_ble_read_event;
uint8_t str_remote_address[18] = { '\0' };
osSemaphoreId_t ble_main_task_sem;

uint16_t ble_data_counter = 0; // This replaces the missing 'offset' member
// Using static to persist across multiple BLE packets
static int brace_depth = 0;
extern casa_context_t casa_ctx;


/******************************************************
 *                   BLE Function Definitions
 ******************************************************/


void stop_ble_service(void)
{
  int32_t status;

  // 1. Attempt to stop advertising.
  // Note: 0x4E0C often occurs if already stopped; we log but continue.
  status = rsi_ble_stop_advertising();
  if (status != RSI_SUCCESS) {
    LOG_WARN("BLE", "Stop advertising status: 0x%lx", status);
  }

  // 2. Clear Callbacks - prevents firmware from jumping to NULL/invalid addresses
  rsi_ble_gap_register_callbacks(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

  // 3. De-initialize the BT Stack
  status = rsi_bt_deinit();
  if (status != RSI_SUCCESS) {
    LOG_ERROR("BLE", "BT Deinit failed: 0x%lx", status);
    return;
  }

  // 4. Safe RTOS Cleanup
  // Instead of checking count, check if the handle exists.
  if (ble_main_task_sem != NULL) {
    osSemaphoreDelete(ble_main_task_sem);
    ble_main_task_sem = NULL; // Crucial: prevent dangling pointer
  }

  LOG_INFO("BLE", "Execution: SUCCESS - BLE Module Stopped");
}

void rsi_ble_send_json_response(const char *json_payload)
{
  if (json_payload == NULL) {
    LOG_ERROR("BLE", "Send Error: Null Buffer");
    return;
  }

  uint16_t payload_len = (uint16_t)strlen(json_payload);
  int32_t status = rsi_ble_gatt_read_response(conn_event_to_app.dev_addr,
                                              0,        // reserved flags
                                              app_ble_read_event.handle,
                                              0,        // offset
                                              payload_len,
                                              (uint8_t *)json_payload);

  if (status != RSI_SUCCESS) {
    LOG_ERROR("BLE", "Send Failed! Status: 0x%lX", (uint32_t)status);
  } else {
    LOG_INFO("BLE", "Sent %d bytes successfully.", payload_len);
  }
}

void rsi_gatt_add_attribute_to_list(rsi_ble_t *p_val,
                                    uint16_t handle,
                                    uint16_t data_len,
                                    uint8_t *data,
                                    uuid_t uuid,
                                    uint8_t char_prop)
{
  if ((p_val->DATA_ix + data_len) >= BLE_ATT_REC_SIZE) {
      LOG_ERROR("BLE", "No data memory for att rec values");
    return;
  }

  p_val->att_rec_list[p_val->att_rec_list_count].char_uuid     = uuid;
  p_val->att_rec_list[p_val->att_rec_list_count].handle        = handle;
  p_val->att_rec_list[p_val->att_rec_list_count].len           = data_len;
  p_val->att_rec_list[p_val->att_rec_list_count].max_value_len = data_len;
  p_val->att_rec_list[p_val->att_rec_list_count].char_val_prop = char_prop;
  memcpy(p_val->DATA + p_val->DATA_ix, data, data_len);
  p_val->att_rec_list[p_val->att_rec_list_count].value = p_val->DATA + p_val->DATA_ix;
  p_val->att_rec_list_count++;
  p_val->DATA_ix += p_val->att_rec_list[p_val->att_rec_list_count].max_value_len;

  return;
}

rsi_ble_att_list_t *rsi_gatt_get_attribute_from_list(rsi_ble_t *p_val, uint16_t handle)
{
  uint16_t i;
  for (i = 0; i < p_val->att_rec_list_count; i++) {
    if (p_val->att_rec_list[i].handle == handle) {
      return &(p_val->att_rec_list[i]);
    }
  }
  return 0;
}

static void rsi_ble_add_char_serv_att(void *serv_handler,
                                      uint16_t handle,
                                      uint8_t val_prop,
                                      uint16_t att_val_handle,
                                      uuid_t att_val_uuid)
{
  rsi_ble_req_add_att_t new_att = { 0 };

  //! preparing the attribute service structure
  new_att.serv_handler       = serv_handler;
  new_att.handle             = handle;
  new_att.att_uuid.size      = 2;
  new_att.att_uuid.val.val16 = RSI_BLE_CHAR_SERV_UUID;
  new_att.property           = RSI_BLE_ATT_PROPERTY_READ;

  //! preparing the characteristic attribute value
  new_att.data_len = 6;
  new_att.data[0]  = val_prop;
  rsi_uint16_to_2bytes(&new_att.data[2], att_val_handle);
  rsi_uint16_to_2bytes(&new_att.data[4], att_val_uuid.val.val16);

  //! Add attribute to the service
  rsi_ble_add_attribute(&new_att);

  return;
}

static void rsi_ble_add_char_val_att(void *serv_handler,
                                     uint16_t handle,
                                     uuid_t att_type_uuid,
                                     uint8_t val_prop,
                                     uint8_t *data,
                                     uint8_t data_len,
                                     uint8_t config_bitmap)
{
  rsi_ble_req_add_att_t new_att = { 0 };

  //! preparing the attributes
  new_att.serv_handler  = serv_handler;
  new_att.handle        = handle;
  new_att.config_bitmap = config_bitmap;
  memcpy(&new_att.att_uuid, &att_type_uuid, sizeof(uuid_t));
  new_att.property = val_prop;

  //! preparing the attribute value
  if (data != NULL)
    memcpy(new_att.data, data, data_len);

  new_att.data_len = data_len;
  //! add attribute to the service
  rsi_ble_add_attribute(&new_att);

  if (((config_bitmap & RSI_BLE_ATT_MAINTAIN_IN_HOST) == 1) || (data_len > 20)) {
    if (data != NULL)
      rsi_gatt_add_attribute_to_list(&att_list, handle, data_len, data, att_type_uuid, val_prop);
  }

  //! check the attribute property with notification
  if (val_prop & RSI_BLE_ATT_PROPERTY_NOTIFY) {
    //! if notification property supports then we need to add client characteristic service.

    //! preparing the client characteristic attribute & values
    memset(&new_att, 0, sizeof(rsi_ble_req_add_att_t));
    new_att.serv_handler       = serv_handler;
    new_att.handle             = handle + 1;
    new_att.att_uuid.size      = 2;
    new_att.att_uuid.val.val16 = RSI_BLE_CLIENT_CHAR_UUID;
    new_att.property           = RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE;
    new_att.data_len           = 2;

    //! add attribute to the service
    rsi_ble_add_attribute(&new_att);
  }

  return;
}

static uint32_t rsi_ble_add_simple_chat_serv(void)
{
  uuid_t new_uuid                       = { 0 };
  rsi_ble_resp_add_serv_t new_serv_resp = { 0 };
  uint8_t data1[100]                    = { 1, 0 };

  //! adding new service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_NEW_SERVICE_UUID;
  rsi_ble_add_service(new_uuid, &new_serv_resp);

  //! adding characteristic service attribute to the service
  new_uuid.size      = 2;
  new_uuid.val.val16 = RSI_BLE_ATTRIBUTE_1_UUID;
  rsi_ble_add_char_serv_att(new_serv_resp.serv_handler,
                            new_serv_resp.start_handle + 1,
                            RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY,
                            new_serv_resp.start_handle + 2,
                            new_uuid);

  //! adding characteristic value attribute to the service
  rsi_ble_att1_val_hndl = new_serv_resp.start_handle + 2;
  new_uuid.size         = 2;
  new_uuid.val.val16    = RSI_BLE_ATTRIBUTE_1_UUID;
  rsi_ble_add_char_val_att(new_serv_resp.serv_handler,
                           new_serv_resp.start_handle + 2,
                           new_uuid,
                           RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY,
                           data1,
                           sizeof(data1),
                           RSI_BLE_ATT_CONFIG_BITMAP);

  return 0;
}

static void rsi_ble_app_init_events()
{
  ble_app_event_map = 0;
  return;
}

static void rsi_ble_app_set_event(uint32_t event_num)
{
  if (event_num < 32) {
    ble_app_event_map |= BIT(event_num);
  }
  osSemaphoreRelease(ble_main_task_sem);
  return;
}

static void rsi_ble_app_clear_event(uint32_t event_num)
{
  if (event_num < 32) {
    ble_app_event_map &= ~BIT(event_num);
  }
  return;
}

static int32_t rsi_ble_app_get_event(void)
{
  uint32_t ix;

  for (ix = 0; ix < 32; ix++) {
    if (ble_app_event_map & (1 << ix)) {
      return ix;
    }
  }

  return (-1);
}

static void rsi_ble_on_connect_event(rsi_ble_event_conn_status_t *resp_conn)
{
  memcpy(&conn_event_to_app, resp_conn, sizeof(rsi_ble_event_conn_status_t));
//  LOG_INFO("BLE", "BLE Connect Event");
  rsi_ble_app_set_event(RSI_BLE_CONN_EVENT);
}

static void rsi_ble_on_disconnect_event(rsi_ble_event_disconnect_t *resp_disconnect, uint16_t reason)
{
  UNUSED_PARAMETER(reason); //This statement is added only to resolve compilation warning, value is unchanged
  memcpy(&disconn_event_to_app, resp_disconnect, sizeof(rsi_ble_event_disconnect_t));
//  LOG_INFO("BLE", "BLE Disconnect Event");
  rsi_ble_app_set_event(RSI_BLE_DISCONN_EVENT);
}

void rsi_ble_on_enhance_conn_status_event(rsi_ble_event_enhance_conn_status_t *resp_enh_conn)
{
  conn_event_to_app.dev_addr_type = resp_enh_conn->dev_addr_type;
  memcpy(conn_event_to_app.dev_addr, resp_enh_conn->dev_addr, RSI_DEV_ADDR_LEN);
  conn_event_to_app.status = resp_enh_conn->status;
//  LOG_DEBUG("BLE", "BLE Enhanced Connection Status Event");
  rsi_ble_app_set_event(RSI_BLE_CONN_EVENT);
}

static void rsi_ble_on_gatt_write_event(uint16_t event_id, rsi_ble_event_write_t *rsi_ble_write)
{
  UNUSED_PARAMETER(event_id); //This statement is added only to resolve compilation warning, value is unchanged
  memcpy(&app_ble_write_event, rsi_ble_write, sizeof(rsi_ble_event_write_t));
//  LOG_DEBUG("BLE", "Data received from mobile app");
  rsi_ble_app_set_event(RSI_BLE_GATT_WRITE_EVENT);
}

static void rsi_ble_on_read_req_event(uint16_t event_id, rsi_ble_read_req_t *rsi_ble_read_req)
{
  UNUSED_PARAMETER(event_id); //This statement is added only to resolve compilation warning, value is unchanged
  memcpy(&app_ble_read_event, rsi_ble_read_req, sizeof(rsi_ble_read_req_t));
//  LOG_DEBUG("BLE", "Read request from mobile app");
  rsi_ble_app_set_event(RSI_BLE_READ_REQ_EVENT);
}

static void rsi_ble_gatt_error_event(uint16_t resp_status, rsi_ble_event_error_resp_t *rsi_ble_gatt_error)
{
  UNUSED_PARAMETER(resp_status); //This statement is added only to resolve compilation warning, value is unchanged
  memcpy(remote_dev_bd_addr, rsi_ble_gatt_error->dev_addr, 6);
//  LOG_ERROR("BLE", "BLE GATT Error Event");
  rsi_ble_app_set_event(RSI_BLE_GATT_ERROR);
}


sl_status_t casa_ble_init(void)
{
  sl_status_t status;
  uint8_t adv[31] = { 2, 1, 6 };

  // 1. Setup GATT Service
  rsi_ble_add_simple_chat_serv();

  // 2. Register Callbacks (GAP & GATT)
  rsi_ble_gap_register_callbacks(NULL, rsi_ble_on_connect_event, rsi_ble_on_disconnect_event,
                                 NULL, NULL, NULL, rsi_ble_on_enhance_conn_status_event,
                                 NULL, NULL, NULL);

  rsi_ble_gatt_register_callbacks(NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                  rsi_ble_on_gatt_write_event, NULL, NULL,
                                  rsi_ble_on_read_req_event, NULL, rsi_ble_gatt_error_event,
                                  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

  // 3. Setup Events and OS primitives
  ble_main_task_sem = osSemaphoreNew(1, 0, NULL);
  if (ble_main_task_sem == NULL) {
      LOG_ERROR("BLE", "Failed to create ble_main_task_sem");
      return FAIL;
  }
  rsi_ble_app_init_events();

  // 4. Set Local Name and Advertising Data
  rsi_bt_set_local_name((uint8_t *)casa_ctx.hostname);
  adv[3] = strlen(casa_ctx.hostname) + 1;
  adv[4] = 9;
  strcpy((char *)&adv[5], casa_ctx.hostname);
  rsi_ble_set_advertise_data(adv, strlen(casa_ctx.hostname) + 5);

  // 5. Start Advertising
  status = rsi_ble_start_advertising();
  if (status == RSI_SUCCESS) {
      LOG_INFO("BLE", "BLE Advertising Started");
  } else {
      LOG_ERROR("BLE", "BLE Advertising failed");
      return FAIL;
  }
  return status;
}

void casa_ble_process(void)
{
  int32_t event_id;
  sl_status_t status;
  //! checking for events list
  event_id = rsi_ble_app_get_event();
  if (event_id == -1) {
    osSemaphoreAcquire(ble_main_task_sem, osWaitForever);
    return;
  }

  switch (event_id) {
    case RSI_BLE_CONN_EVENT: {
      //! event invokes when connection was completed

      //! clear the served event
      brace_depth = 0;
      ble_data_counter = 0;
      rsi_ble_app_clear_event(RSI_BLE_CONN_EVENT);
      rsi_6byte_dev_address_to_ascii(str_remote_address, conn_event_to_app.dev_addr);
      LOG_INFO("BLE", "Module connected to address : %s", str_remote_address);

      status = rsi_ble_mtu_exchange_event((uint8_t *)conn_event_to_app.dev_addr, RSI_BLE_MTU_SIZE);
      if (status != RSI_SUCCESS) {
          LOG_ERROR("BLE", "MTU CMD status: 0x%lX", status);
      }

    } break;

    case RSI_BLE_DISCONN_EVENT: {
      //! event invokes when disconnection was completed
      //! clear the served event
      brace_depth = 0;
      ble_data_counter = 0;
      rsi_ble_app_clear_event(RSI_BLE_DISCONN_EVENT);
      LOG_WARN("BLE", "Module Disconnected");

      // Restart advertising so the device remains discoverable
      do {
        status = rsi_ble_start_advertising();
      } while (status != RSI_SUCCESS);
      LOG_INFO("BLE", "Restarted Advertising...");
    } break;
    case RSI_BLE_GATT_WRITE_EVENT: {
      rsi_ble_app_clear_event(RSI_BLE_GATT_WRITE_EVENT);

      // Check if the write is on the correct attribute handle
      if ((*(uint16_t *)app_ble_write_event.handle) == rsi_ble_att1_val_hndl) {
          if (casa_ctx.registration == NULL) {
                    LOG_ERROR("BLE", "Registration context unavailable; ignoring BLE write");
            rsi_ble_gatt_write_response(conn_event_to_app.dev_addr, 0);
            break;
          }
        uint16_t length = app_ble_write_event.length;
        uint8_t *incoming_data = app_ble_write_event.att_value;

        // We use static variables to keep track across multiple packet events
        static int brace_depth = 0;

        // Disable interrupts/inputs like in your ESP32 code
        casa_ctx.switch_interrupts = DISABLE;

        for (int i = 0; i < length; i++) {
          uint8_t c = incoming_data[i];

          // 1. Filter out whitespace/junk
          if (c == '\n' || c == '\r' || c == '\t') continue;

          // 2. Start of JSON detection
          if (c == '{') {
            // If we see a '{' but the counter is stuck from a previous failed attempt, reset.
            if (brace_depth == 0 && casa_ctx.registration->ble_data_counter > 0) {
               printf("\r\n[BLE] Resyncing Buffer...\r\n");
               casa_ctx.registration->ble_data_counter = 0;
               memset(casa_ctx.registration->ble_buffer, 0, BLE_MAX_REQ_SIZE);
            }
            brace_depth++;
          }

          // 3. Collection Logic
          if (brace_depth > 0) {
            if (casa_ctx.registration->ble_data_counter < (BLE_MAX_REQ_SIZE - 1)) {
                // Store character using the persistent counter
                casa_ctx.registration->ble_buffer[casa_ctx.registration->ble_data_counter++] = c;
            } else {
                printf("\r\n[BLE] ERROR: Buffer Overflow (%d bytes)\r\n", BLE_MAX_REQ_SIZE);
                casa_ctx.registration->ble_data_counter = 0;
                brace_depth = 0;
                break;
            }
          }

          // 4. End of JSON detection
          if (c == '}') {
            if (brace_depth > 0) brace_depth--;

            // If depth reaches 0, we have a complete JSON object
            if (brace_depth == 0 && casa_ctx.registration->ble_data_counter > 0) {
              casa_ctx.registration->ble_buffer[casa_ctx.registration->ble_data_counter] = '\0'; // Null terminate
              casa_ctx.registration->buffer_status = true;

              printf("\r\n[BLE] Full JSON Received (%d bytes):\r\n%s\r\n", casa_ctx.registration->ble_data_counter, (char *)casa_ctx.registration->ble_buffer);

              // Reset counter for the NEXT complete JSON string
              casa_ctx.registration->ble_data_counter = 0;

              // Note: Do NOT memset here if you need to parse the buffer in another task
              // Parsing should happen now or in a separate task
            }
          }
        }

        casa_ctx.switch_interrupts = ENABLE;

        // Send response back to mobile app to acknowledge the write
        rsi_ble_gatt_write_response(conn_event_to_app.dev_addr, 0);
      }
    } break;
    case RSI_BLE_READ_REQ_EVENT: {
      rsi_ble_app_clear_event(RSI_BLE_READ_REQ_EVENT);
      printf("Read Request handled.\n");
    } break;

    case RSI_BLE_GATT_ERROR: {

      //! clear the served even
      rsi_ble_app_clear_event(RSI_BLE_GATT_ERROR);
    } break;
    default: {
    }
  }
}

