/*
 * mqtt_handler.c
 *
 *  Created on: 06-Feb-2026
 *      Author: User
 */



#include "sl_constants.h"
#include "sl_mqtt_client.h"
#include "sl_wifi.h"
#include "sl_net.h"
#include "sl_net_dns.h"
#include "sl_net_si91x.h"
#include "sl_net_wifi_types.h"
#include "sl_net_default_values.h"

#include "mqtt_handler.h"
#include "gpio_control.h"
#include "casa_module.h"
#include "casa_msg_parser.h"

static const char *TAG = "MQTTS";

#define MQTT_BROKER_IP        "10.13.195.72"
#define MQTT_BROKER_PORT      8883
#define IOT_MQTT_HOST         "agile-lime-alpaca.rmq6.cloudamqp.com"
#define CLIENT_PORT           1
#define CLIENT_ID             "SIWX917_MQTT_TLS_CLIENT"

#define TOPIC_TO_BE_SUBSCRIBED "test"
#define QOS_OF_SUBSCRIPTION    SL_MQTT_QOS_LEVEL_1

#define PUBLISH_TOPIC          "send"
#define PUBLISH_MESSAGE_BASE   "Message Counter: "
#define QOS_OF_PUBLISH_MESSAGE SL_MQTT_QOS_LEVEL_0

#define IS_DUPLICATE_MESSAGE  0
#define IS_MESSAGE_RETAINED   0
#define IS_CLEAN_SESSION      1

#define LAST_WILL_TOPIC       "device/lwt"
#define LAST_WILL_MESSAGE     "Device disconnected"
#define QOS_OF_LAST_WILL      1
#define IS_LAST_WILL_RETAINED 1

#define ENCRYPT_CONNECTION     1
#define SEND_CREDENTIALS       1
#define CERTIFICATE_INDEX      1
#define KEEP_ALIVE_INTERVAL    60
#define MQTT_CONNECT_TIMEOUT   5000
#define MQTT_KEEPALIVE_RETRIES 3
#define MQTT_RX_PARSE_BUF_LEN  2048

#define USERNAME "ihkkclcq:ihkkclcq"
#define PASSWORD "iE6uMQPeFgvHDic4kJpEoyboCsPDr1rs"

char mqtt_pub_topic[MQTT_TOPIC_LEN] = {'\0'};                                          /* MQTT publish topic data buffer */
char mqtt_sub_topic[MQTT_TOPIC_LEN] = {'\0'};                                          /* MQTT subscribe topic data buffer */
char mqtt_pub_lastWill_topic[LASTWILL_TOPIC_LEN] = {'\0'};                             /* MQTT publish last will topic data buffer */
char mqtt_pub_support_topic[SUPPORT_TOPIC_LEN] = {'\0'};                               /* MQTT publish support topic data buffer */
char mqtt_sub_support_topic[SUPPORT_TOPIC_LEN] = {'\0'};                               /* MQTT subscribe support topic data buffer */
char device_log_topic_pub[LOG_TPOIC_LEN] = {'\0'};                                     /* device log publish topic data buffer */
char lastwill_buf[DEVICE_STATUS_JSON_LEN] = {'\0'};                                    /* Last will Offline and online updated JSON*/
char fota_tls_topic_pub[MQTT_TOPIC_LEN] = {'\0'};                                      /* Fota MQTT publish topic data buffer */
char fota_tls_topic_sub[MQTT_TOPIC_LEN] = {'\0'};                                      /* Fota MQTT subscibe topic data buffer */

/* MQTT callbacks may run in Si91x async RX context; defer heavy work to app task. */
static volatile bool mqtt_subscribe_pending = false;

static volatile bool mqtt_rx_message_pending = false;
static volatile uint16_t mqtt_rx_message_len = 0;
static char mqtt_rx_parse_buf[MQTT_RX_PARSE_BUF_LEN] = {0};

bool device_status_report = 1;
extern casa_context_t casa_ctx;

bool mqtt_connection_check = false;
bool ssl_connection_error = false;
extern bool internet_status;
sl_mqtt_client_t mqtt_client;
extern float fw_version;

const unsigned char mycacert[] =
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";

/******************************************************
 *               Thread Attributes
 ******************************************************/
// Task Handle (Equivalent to mqttConnectHandle)
osThreadId_t mqttConnectHandle = NULL;

// Attributes based on your provided structure
const osThreadAttr_t mqtt_thread_attributes = {
  .name       = "mqtt_reconnection_check",
  .stack_size = 8192,           // Increased slightly to handle TLS + Stack overhead
  .priority   = osPriorityNormal,
};


/******************************************************
 *               Globals
 ******************************************************/
sl_mqtt_client_t client;
sl_mqtt_client_credentials_t *client_credentials = NULL;
sl_mqtt_client_configuration_t mqtt_client_config = {0};
sl_mqtt_client_last_will_message_t last_will_message = {0};
sl_mqtt_broker_t mqtt_broker_config = {0};


/******************************************************
 *               MQTT MESSAGE CALLBACK
 ******************************************************/
void mqtt_message_callback(void *client, sl_mqtt_client_message_t *message, void *context)
{
  UNUSED_PARAMETER(client);
  UNUSED_PARAMETER(context);

//  LOG_INFO("MQTT", "TOPIC=%.*s", message->topic_length, message->topic);
//  LOG_INFO("MQTT", "DATA=%.*s", message->content_length, message->content);
  if (message == NULL || message->content == NULL || message->content_length == 0) {
//     LOG_WARN("MQTT", "Received empty MQTT payload");
     return;
   }

   if (message->content_length >= MQTT_RX_PARSE_BUF_LEN) {
//     LOG_ERROR("MQTT", "MQTT payload too large for parser buffer (%u)", (unsigned int)message->content_length);
     return;
   }

//   char mqtt_json_buf[MQTT_RX_PARSE_BUF_LEN] = {0};
//   memcpy(mqtt_json_buf, message->content, message->content_length);
//   mqtt_json_buf[message->content_length] = '\0';
   if (mqtt_rx_message_pending) {
       return;
     }

//   if (!casa_message_parser(mqtt_json_buf, (int)message->content_length)) {
//     LOG_ERROR("MQTT", "MQTT payload parser failed");
//   }
   memcpy(mqtt_rx_parse_buf, message->content, message->content_length);
   mqtt_rx_parse_buf[message->content_length] = '\0';
   mqtt_rx_message_len = message->content_length;
   mqtt_rx_message_pending = true;

}

/******************************************************
 *               MQTT EVENT HANDLER
 ******************************************************/

void mqtt_event_handler(void *client, sl_mqtt_client_event_t event,
                        void *event_data, void *context)
{
  UNUSED_PARAMETER(event_data);
  UNUSED_PARAMETER(context);
  UNUSED_PARAMETER(client);

  switch (event) {
    case SL_MQTT_CLIENT_CONNECTED_EVENT:
//      LOG_INFO("MQTT", "MQTT event Connected");
      mqtt_connection_check = 1;
      mqtt_subscribe_pending = true;
      casa_ctx.mqtt_ssl_connection = 1;

      // Subscribe with callback
//      sl_mqtt_client_subscribe(client, (uint8_t *)mqtt_sub_topic, strlen(mqtt_sub_topic), QOS_OF_SUBSCRIPTION, 0, mqtt_message_callback, NULL);
      break;

    case SL_MQTT_CLIENT_MESSAGE_PUBLISHED_EVENT:
//      LOG_INFO("MQTT", "Message Published");
      break;

    case SL_MQTT_CLIENT_SUBSCRIBED_EVENT:
//      LOG_INFO("MQTT", "Subscribed successfully");
      break;

    case SL_MQTT_CLIENT_DISCONNECTED_EVENT:
//      LOG_INFO("MQTT", "MQTT Disconnected");
      mqtt_connection_check = 0;
      mqtt_subscribe_pending = false;
      casa_ctx.mqtt_ssl_connection = 0;
      break;

    case SL_MQTT_CLIENT_ERROR_EVENT:
//      LOG_ERROR("MQTT", "MQTT Error");
      mqtt_connection_check = 0;
      mqtt_subscribe_pending = false;
      casa_ctx.mqtt_ssl_connection = 0;
      break;

    default:
      break;
  }
}

/******************************************************
 *             MQTT  Application
 ******************************************************/

// Global config structures for Si917
void mqtt_app_close(void)
{
    sl_status_t status;
    status = sl_mqtt_client_disconnect(&mqtt_client, 5000);
    if (status == SL_STATUS_OK) {
        LOG_ERROR("MQTT", "Disconnect failed: 0x%lx", status);
        sl_mqtt_client_deinit(&mqtt_client); // Important: Frees the hardware socket
        mqtt_connection_check = false;
        LOG_INFO("MQTT", "MQTT Session Closed");
    }
//    if (mqttConnectHandle != NULL) {
//       status = osThreadTerminate(mqttConnectHandle);
//
//      if (status == SL_STATUS_OK) {
//          LOG_INFO("MQTT", "MQTT Thread deleted successfully.");
//          mqttConnectHandle = NULL; // Reset handle to prevent double-deletion
//      } else {
//          LOG_ERROR("MQTT", "Failed to delete MQTT Thread: %d", status);
//      }
//    }
}

bool Mqtt_publish(const char *Topic, const char *string)
{
  if ((Topic == NULL) || (string == NULL) || !mqtt_connection_check) {
      LOG_WARN("MQTT", "Publish skipped: invalid params or MQTT disconnected");
      return false;
  }
  sl_mqtt_client_message_t publish_message;
  publish_message.qos_level            = QOS_OF_PUBLISH_MESSAGE;
  publish_message.is_retained          = IS_MESSAGE_RETAINED;
  publish_message.is_duplicate_message = IS_DUPLICATE_MESSAGE;
  publish_message.topic                = (uint8_t *)Topic;
  publish_message.topic_length         = strlen(Topic);
  publish_message.content              = (uint8_t *)string;
  publish_message.content_length       = strlen(string);

//  return sl_mqtt_client_publish(&mqtt_client, &publish_message, 0, NULL);
  sl_status_t status = sl_mqtt_client_publish(&mqtt_client, &publish_message, 0, NULL);
  if (status != SL_STATUS_OK) {
      LOG_WARN("MQTT", "Publish failed: 0x%lx", status);
      return false;
  }

  return true;
}

bool mqtt_secure_config(bool input)
{
  mqtt_app_close();

  sl_status_t status;
  mqtt_connection_check = false; // Reset flag on start

  if(input){
    status = sl_net_set_credential( SL_NET_TLS_SERVER_CREDENTIAL_ID(CERTIFICATE_INDEX), SL_NET_SIGNING_CERTIFICATE, mycacert, sizeof(mycacert) - 1);
    if (status != SL_STATUS_OK) {
        LOG_ERROR("MQTT", "CA certificate load failed: 0x%lx", status);
        return false;
      }
    LOG_INFO("MQTT", "CA Cert Load: 0x%lx", status);
  }

  uint32_t len = sizeof(sl_mqtt_client_credentials_t) + strlen(USERNAME) + strlen(PASSWORD);

  client_credentials = malloc(len);
  if (client_credentials == NULL)
    return false;

  memset(client_credentials, 0, len);

  client_credentials->username_length = strlen(USERNAME);
  client_credentials->password_length = strlen(PASSWORD);

  memcpy(client_credentials->data, USERNAME, strlen(USERNAME));
  memcpy(client_credentials->data + strlen(USERNAME), PASSWORD, strlen(PASSWORD));

  status = sl_net_set_credential(SL_NET_MQTT_CLIENT_CREDENTIAL_ID(0), SL_NET_MQTT_CLIENT_CREDENTIAL, client_credentials, len);
  if (status != SL_STATUS_OK) {
    LOG_ERROR("MQTT", "MQTT credentials load failed: 0x%lx", status);
    return false;
  }

  mqtt_client_config.credential_id               = SL_NET_MQTT_CLIENT_CREDENTIAL_ID(0);
  mqtt_client_config.is_clean_session            = IS_CLEAN_SESSION;
  mqtt_client_config.client_id                   = (uint8_t *)CLIENT_ID;
  mqtt_client_config.client_id_length            = strlen(CLIENT_ID);
  mqtt_client_config.client_port                 = CLIENT_PORT;
  mqtt_client_config.tls_flags                   = 0;
  mqtt_broker_config.port                        = 1883;
  mqtt_broker_config.is_connection_encrypted     = 0;
  if(input){
      mqtt_client_config.tls_flags               = SL_MQTT_TLS_ENABLE | SL_MQTT_TLS_TLSV_1_2 | SL_MQTT_TLS_CERT_INDEX_1;
      mqtt_broker_config.port                    = 8883;
      mqtt_broker_config.is_connection_encrypted = 1;
  }
  mqtt_broker_config.connect_timeout             = MQTT_CONNECT_TIMEOUT;
  mqtt_broker_config.keep_alive_interval         = KEEP_ALIVE_INTERVAL;
  mqtt_broker_config.keep_alive_retries          = MQTT_KEEPALIVE_RETRIES;

  last_will_message.will_topic                   = (uint8_t *)LAST_WILL_TOPIC;
  last_will_message.will_topic_length            = strlen(LAST_WILL_TOPIC);
  last_will_message.will_message                 = (uint8_t *)LAST_WILL_MESSAGE;
  last_will_message.will_message_length          = strlen(LAST_WILL_MESSAGE);
  last_will_message.will_qos_level               = QOS_OF_LAST_WILL;
  last_will_message.is_retained                  = IS_LAST_WILL_RETAINED;

  sl_ip_address_t broker_ip = {0};

  // 1. Resolve Hostname to IP address
  LOG_INFO("MQTT", "Resolving Hostname: %s", IOT_MQTT_HOST);

  uint8_t dns_retry_count = 0;
  const uint8_t MAX_RETRIES = 3;
  do {
      dns_retry_count++;
      status = sl_net_dns_resolve_hostname(IOT_MQTT_HOST, 15000, SL_NET_DNS_TYPE_IPV4, &broker_ip);
      if (status == SL_STATUS_OK) {
          LOG_INFO("MQTT", "DNS Success on attempt %d", dns_retry_count);
          break; // Exit loop immediately on success
      }
      LOG_WARN("MQTT", "DNS Resolution Failed (Attempt %d): 0x%lx", dns_retry_count, status);
      if (dns_retry_count < MAX_RETRIES) {
            LOG_INFO("MQTT", "Retrying in 2 seconds...");
            osDelay(2000); // Wait before next attempt
      }

  } while(status != SL_STATUS_OK && dns_retry_count < MAX_RETRIES);

  if (status != SL_STATUS_OK) {
      LOG_ERROR("MQTT", "DNS failed after %d attempts. Check Network Connection.", MAX_RETRIES);
      return false;
  }

  // Print resolved IP for verification
  LOG_INFO("MQTT", "Resolved IP: %u.%u.%u.%u", broker_ip.ip.v4.bytes[0], broker_ip.ip.v4.bytes[1], broker_ip.ip.v4.bytes[2], broker_ip.ip.v4.bytes[3]);

  // 2. Manually map the resolved IP into your existing broker configuration
  mqtt_broker_config.ip.type = SL_IPV4;
  memcpy(&mqtt_broker_config.ip.ip.v4.value, &broker_ip.ip.v4.value, 4);

  // Init MQTT client
  sl_mqtt_client_init(&mqtt_client, mqtt_event_handler);

  // Connect to MQTT broker
  sl_mqtt_client_connect(&mqtt_client, &mqtt_broker_config, &last_will_message, &mqtt_client_config, 0);
  return true;

}

#define  MQTT_CONN_RETRY_LIMIT           30                         /* MQTT connection retry limit 30 -> 15 sec */


bool mqtt_app_start(void)
{
  if (!mqtt_secure_config(true)) {
      printf("mqtt_secure_config()\r\n");
          return false;
  }
  int retry_count = 0;
  while (retry_count < MQTT_CONN_RETRY_LIMIT) {
      if(mqtt_connection_check) {
          LOG_INFO("MQTT", "MQTT Connected");
          sl_mqtt_client_subscribe(&mqtt_client, (uint8_t *)mqtt_sub_topic, strlen(mqtt_sub_topic), QOS_OF_SUBSCRIPTION, 0, mqtt_message_callback, NULL);
          return true;
      } else {
          LOG_INFO("MQTT", "MQTT Trying to connect...");
          retry_count++;
          osDelay(500);
      }
  }
  return false;

}

void mqtt_reconnection_check(void *arg)
{
  (void)arg;
  uint32_t counter = 0;
  uint32_t slow_loop_timer = 0;
//  sl_mqtt_client_message_t publish_message;

  while(true) {
      // --- SECTION 1: FAST LOGIC (Executes every 50ms) ---

      // Handle Incoming Messages immediately
      if(mqtt_connection_check && mqtt_rx_message_pending) {
          printf("received JSON : %s\r\n", mqtt_rx_parse_buf);
          if (!casa_message_parser(mqtt_rx_parse_buf, (int)mqtt_rx_message_len)) {
              LOG_ERROR("MQTT", "MQTT payload parser failed");
          }
          mqtt_rx_parse_buf[0] = '\0';
          mqtt_rx_message_len = 0;
          mqtt_rx_message_pending = false;
      }

      // --- SECTION 2: SLOW LOGIC (Executes every 5000ms) ---

      if (slow_loop_timer >= 100) { // 100 iterations * 50ms = 5000ms
          slow_loop_timer = 0; // Reset the 5s timer

          // 1. Connection Management
          if(mqtt_connection_check == 0 && internet_status == 1) {
              mqtt_app_start();
          }

          if(mqtt_connection_check) {
              // 2. Handle Subscriptions
              if (mqtt_subscribe_pending) {
                  // Static topic subscription
                  sl_mqtt_client_subscribe(&mqtt_client, (uint8_t *)TOPIC_TO_BE_SUBSCRIBED,
                                          strlen(TOPIC_TO_BE_SUBSCRIBED), QOS_OF_SUBSCRIPTION,
                                          0, mqtt_message_callback, NULL);

                  // Dynamic topic subscription
                  sl_mqtt_client_subscribe(&mqtt_client, (uint8_t *)mqtt_sub_topic,
                                          strlen(mqtt_sub_topic), QOS_OF_SUBSCRIPTION,
                                          0, mqtt_message_callback, NULL);

                  mqtt_subscribe_pending = false;
                  LOG_INFO("MQTT", "Subscriptions processed");
              }

              // 3. Periodic Publishing
              char message_payload[128];
              snprintf(message_payload, sizeof(message_payload), "%s%lu", PUBLISH_MESSAGE_BASE, counter++);
              Mqtt_publish(PUBLISH_TOPIC, message_payload);

//              publish_message.qos_level            = QOS_OF_PUBLISH_MESSAGE;
//              publish_message.is_retained          = IS_MESSAGE_RETAINED;
//              publish_message.is_duplicate_message = IS_DUPLICATE_MESSAGE;
//              publish_message.topic                = (uint8_t *)PUBLISH_TOPIC;
//              publish_message.topic_length         = strlen(PUBLISH_TOPIC);
//              publish_message.content              = (uint8_t *)message_payload;
//              publish_message.content_length       = strlen(message_payload);
//
//              sl_mqtt_client_publish(&mqtt_client, &publish_message, 0, NULL);
          }
      }

//      if (slow_loop_timer >= 20) {
//          printf("mqtt_connection_check : %d, internet_status: %d\r\n",mqtt_connection_check, internet_status);
//      }

      // Small delay to drive the 50ms timing
      osDelay(50);
      slow_loop_timer++;
  }
}

//void mqtt_reconnection_check(void *arg)
//{
//  (void)arg;
//  while(true) {
//      // Continuous publishing loop
//        uint32_t counter = 0;
//        sl_mqtt_client_message_t publish_message;
//        while (1) {
//            if(mqtt_connection_check == 0 && internet_status == 1) {
//                mqtt_app_start();
//            }
//            if(mqtt_connection_check) {
//                if (mqtt_rx_message_pending) {
//                    printf("received JSON : %s\r\n",mqtt_rx_parse_buf);
//                    if (!casa_message_parser(mqtt_rx_parse_buf, (int)mqtt_rx_message_len)) {
//                        LOG_ERROR("MQTT", "MQTT payload parser failed");
//                    }
//                    mqtt_rx_parse_buf[0] = '\0';
//                    mqtt_rx_message_len = 0;
//                    mqtt_rx_message_pending = false;
//                }
//
//                if (mqtt_subscribe_pending) {
//                  int msg_id = sl_mqtt_client_subscribe(&mqtt_client,
//                                                        (uint8_t *)TOPIC_TO_BE_SUBSCRIBED,
//                                                        strlen(TOPIC_TO_BE_SUBSCRIBED),
//                                                        QOS_OF_SUBSCRIPTION,
//                                                        0,
//                                                        mqtt_message_callback,
//                                                        NULL);
//                  if (msg_id >= 0) {
//                    LOG_INFO("MQTT", "Subscribed successfully (msg_id=%d)", msg_id);
//                    mqtt_subscribe_pending = false;
//                  } else {
//                    LOG_WARN("MQTT", "Subscribe request failed (%d), retrying", msg_id);
//                  }
//
//                  msg_id = sl_mqtt_client_subscribe(&mqtt_client,
//                                                          (uint8_t *)mqtt_sub_topic,
//                                                          strlen(mqtt_sub_topic),
//                                                          QOS_OF_SUBSCRIPTION,
//                                                          0,
//                                                          mqtt_message_callback,
//                                                          NULL);
//                    if (msg_id >= 0) {
//                      LOG_INFO("MQTT", "Subscribed successfully (msg_id=%d)", msg_id);
//                      mqtt_subscribe_pending = false;
//                    } else {
//                      LOG_WARN("MQTT", "Subscribe request failed (%d), retrying", msg_id);
//                    }
//                }
//              char message_payload[128];
//              snprintf(message_payload, sizeof(message_payload), "%s%lu", PUBLISH_MESSAGE_BASE, counter++);
//
//              publish_message.qos_level            = QOS_OF_PUBLISH_MESSAGE;
//              publish_message.is_retained          = IS_MESSAGE_RETAINED;
//              publish_message.is_duplicate_message = IS_DUPLICATE_MESSAGE;
//              publish_message.topic                = (uint8_t *)PUBLISH_TOPIC;
//              publish_message.topic_length         = strlen(PUBLISH_TOPIC);
//              publish_message.content              = (uint8_t *)message_payload;
//              publish_message.content_length       = strlen(message_payload);
//
//              sl_mqtt_client_publish(&mqtt_client, &publish_message, 0, NULL);
//            }
//          osDelay(5000); // publish every 2 seconds
//        }
//  }
//
//}

void mqtt_connection_task_create(void)
{
    mqttConnectHandle = NULL;
    casa_ctx.mqtt_ssl_connection = true;

    mqttConnectHandle = osThreadNew(mqtt_reconnection_check, NULL, &mqtt_thread_attributes);
    if (mqttConnectHandle != NULL) {
        LOG_INFO("MQTT", "MQTT Connection checking task created.");
    } else {
        LOG_ERROR("MQTT", "MQTT Connection checking task not created.");
    }
}


void constuct_secure_mqtt_sub_pub_topic(void)
{
    memset(mqtt_sub_topic, '\0', MQTT_TOPIC_LEN);       /* Subscribing topic : "/user_id/set/location_id/gateway_id/device_id" */
    memset(mqtt_pub_topic, '\0', MQTT_TOPIC_LEN);       /* Publishing topic  : "/user_id/report/location_id/gateway_id/device_id" */
    snprintf(mqtt_sub_topic, MQTT_TOPIC_LEN, "/%s/set/%s/gateway_id/%s", casa_ctx.userid, casa_ctx.location, casa_ctx.uniqueid);
    snprintf(mqtt_pub_topic, MQTT_TOPIC_LEN, "/%s/report/%s/gateway_id/%s", casa_ctx.userid, casa_ctx.location, casa_ctx.uniqueid);
    LOG_INFO("MQTT", "MQTT CASA Device is subscribing to : %s", mqtt_sub_topic);
    LOG_INFO("MQTT", "MQTT CASA Device is publishing to: %s", mqtt_pub_topic);
}

void construct_mqtt_support_pub_topic(void)
{
//    set_device_id();
    memset(mqtt_pub_support_topic, '\0', SUPPORT_TOPIC_LEN);     /* Support publish topic : "/support/report/device_id" */
    memset(mqtt_sub_support_topic, '\0', SUPPORT_TOPIC_LEN);     /* Support Subscribing topic : "/support/set/device_id" */
    snprintf(mqtt_sub_support_topic, SUPPORT_TOPIC_LEN, "/support/set/%s", casa_ctx.uniqueid);
    snprintf(mqtt_pub_support_topic, SUPPORT_TOPIC_LEN, "/support/report/%s", casa_ctx.uniqueid);
    LOG_INFO("MQTT", "MQTT CASA Device Support publish topic : %s", mqtt_pub_support_topic);
    LOG_INFO("MQTT", "MQTT CASA Device Support Subscribing topic : %s", mqtt_sub_support_topic);
}
void construct_mqtt_lastWill_pub_topic(void)
{
    memset(mqtt_pub_lastWill_topic, '\0', LASTWILL_TOPIC_LEN);     /* Last will publish topic : "/device/device_id/status" */
    snprintf(mqtt_pub_lastWill_topic, LASTWILL_TOPIC_LEN, "/device/%s/status", casa_ctx.uniqueid);
    LOG_INFO("MQTT", "MQTT CASA Device Last will publish topic : %s", mqtt_pub_lastWill_topic);
}

void construct_mqtt_device_log_pub_topic(void)
{
    memset(device_log_topic_pub, '\0', LOG_TPOIC_LEN);             /* Device log publish topic : "/log/user_id/device_id/" */
    snprintf(device_log_topic_pub, LOG_TPOIC_LEN, "/log/%s/%s", casa_ctx.userid,casa_ctx.uniqueid);
    LOG_INFO("MQTT", "MQTT CASA Device log publish topic : %s", device_log_topic_pub);
}

void construct_unsecure_mqtt_sub_pub_topic(void)
{
    memset(fota_tls_topic_pub, '\0', MQTT_TOPIC_LEN);         /* SSL FOTA update Subscribing topic : "/device_id/set/fota_check" */
    memset(fota_tls_topic_sub, '\0', MQTT_TOPIC_LEN);         /* SSL FOTA update Publishing topic  : "/device_id/report/fota_check" */
    sprintf(fota_tls_topic_pub, "/%s%s", casa_ctx.uniqueid, FOTA_TLS_TOPIC_PUB);
    sprintf(fota_tls_topic_sub, "/%s%s", casa_ctx.uniqueid ,FOTA_TLS_TOPIC_SUB);
    LOG_INFO("MQTT", "CASA Device SSL FOTA update subscribing topic is : %s", fota_tls_topic_sub);
    LOG_INFO("MQTT", "CASA Device SSL FOTA update publishing topic is  : %s", fota_tls_topic_pub);
}

void construct_fota_update_request(char fota_avlb_buf[FOTA_UPDATE_AVAILABLE_JSON_LEN])
{
    char float_val[5]={'\0'};
    memset(fota_avlb_buf, '\0', FOTA_UPDATE_AVAILABLE_JSON_LEN);
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "reqId", 1268302);
    cJSON_AddNumberToObject(root, "optCode", SSL_FOTA_UPDATE_REQ_OPCODE);
    cJSON_AddStringToObject(root, "dId", casa_ctx.uniqueid);
    cJSON_AddStringToObject(root, "dMod", MODEL);
    snprintf(float_val,5,"%.1f",fw_version);
    cJSON_AddRawToObject(root, "cVer", float_val);
    snprintf(float_val,5,"%.1f",casa_ctx.hardware_ver);
    cJSON_AddRawToObject(root, "hwVer", float_val);

    char* json = cJSON_Print(root);
    cJSON_Delete(root);
    cJSON_Minify(json);
    strcpy(fota_avlb_buf,json);
    LOG_INFO("MQTT", "constructed fota update request: %s",(char *)json);
    free(json);
}

void unsecure_fota_check(void)
{
    char fota_avlb_buf[FOTA_UPDATE_AVAILABLE_JSON_LEN] = {'\0'};
    construct_unsecure_mqtt_sub_pub_topic();
    int msg_id = sl_mqtt_client_subscribe(&mqtt_client, (uint8_t *)fota_tls_topic_sub, strlen(fota_tls_topic_sub), QOS_OF_SUBSCRIPTION, 0, mqtt_message_callback, NULL);
    LOG_INFO("MQTT", "sent subscribe successful, msg_id=%d", msg_id);
    construct_fota_update_request(fota_avlb_buf);
    Mqtt_publish(fota_tls_topic_pub, fota_avlb_buf);
    memset(fota_avlb_buf, '\0', FOTA_UPDATE_AVAILABLE_JSON_LEN);
}

void send_timer_resp(long long requestid, int type, int node, int status)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "reqId", requestid);
    cJSON_AddStringToObject(root, "dId", casa_ctx.uniqueid);
    cJSON_AddNumberToObject(root, "nId", node);
    cJSON_AddStringToObject(root, "userId", casa_ctx.userid);

    const char *event_by = NULL;
    const char *event_by_id = NULL;

    if (status == 2 || status == 3 || status == 4) {

        if (status == 2) {
            event_by = DEVICE_CONTROL;
        } else if (status == 3) {
            event_by = TIMER_CONTROL;
        } else { // status == 4
            event_by = MANUAL_CONTROL;
            status = 1; // Adjust status for manual control
        }
        event_by_id = casa_ctx.userid;

    } else {
        event_by = casa_ctx.event_by;
        event_by_id = casa_ctx.event_by_id;
    }

    cJSON_AddStringToObject(root, "eBy", event_by);
    cJSON_AddStringToObject(root, "eById", event_by_id);

    if (type == 1) {
        cJSON_AddNumberToObject(root, "optCode", TIMER_RESP);
        cJSON_AddNumberToObject(root, "inSec", status);
    } else if(type ==2) {
        cJSON_AddNumberToObject(root, "optCode", TIMER_DELETE);
        cJSON_AddNumberToObject(root, "cBy", status);
    }

    char* json = cJSON_Print(root);
    cJSON_Delete(root);
    cJSON_Minify(json);
    LOG_INFO(TAG, "Timer status responce: %s",(char *)json);
    Mqtt_publish(mqtt_pub_topic, json);
    free(json);
}
