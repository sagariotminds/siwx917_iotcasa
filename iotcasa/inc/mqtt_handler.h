/*
 * mqtt_handler.h
 *
 *  Created on: 06-Feb-2026
 *      Author: User
 */

#ifndef MQTT_HEADLER_H_
#define MQTT_HEADLER_H_

#include "iotcasa_types.h"

#define  FOTA_TLS_TOPIC_SUB              "/set/fota_check"          /* Fota tls topic subscription for unsecure fota   */
#define  FOTA_TLS_TOPIC_PUB              "/report/fota_check"       /* Fota tls topic publish for unsecure fota */

#define  MQTT_TOPIC_LEN                  200                        /* MQTT topic buffer maximum length */
#define  LASTWILL_TOPIC_LEN              45                         /* MQTT last will topic maximum length */
#define  SUPPORT_TOPIC_LEN               50                         /* MQTT Support topic maximum length */
#define  LOG_TPOIC_LEN                   85                         /* device log publish topic length */
#define  DEVICE_STATUS_JSON_LEN          250                        /* Device status buffer length */
#define  FOTA_UPDATE_AVAILABLE_JSON_LEN  200                        /* Fota update available json maximum length */
#define  MQTT_TOPIC_LEN                  200                        /* MQTT topic buffer maximum length */


void mqtt_app_close(void);
bool Mqtt_publish(const char *Topic, const char *string);
bool mqtt_secure_config(bool input);
bool mqtt_app_start(void);
void mqtt_reconnection_check(void *arg);
void mqtt_connection_task_create(void);
void mqtt_start();


/**
 * @brief     This function constructs the publishing and subscription topics for IoTCasa
 * @param[in] None
 * @return    None
 */
void constuct_secure_mqtt_sub_pub_topic(void);

/**
 * @brief     This function constructs the Support publishing topic for IoTCasa
 * @param[in] Void
 * @return    Void
 */
void construct_mqtt_support_pub_topic(void);

/**
 * @brief     This function constructs the last will publishing topic for IoTCasa
 * @param[in] None
 * @return    None
 */
void construct_mqtt_lastWill_pub_topic(void);

/**
 * @brief     Publish Device log message topic.
 * @param[in] None
 * @return    None
 */
void construct_mqtt_device_log_pub_topic(void);

/**
 * @brief     Construct MQTT last will message JSON.
 * @param[in] status_dev Indicates MQTT offline or online status.
 * @param[in] json Last will data buffer array pointer.
 * @return    None
 */
void construct_mqtt_lastwill_msg(int status_dev, char json[DEVICE_STATUS_JSON_LEN]);

void unsecure_fota_check(void);
void send_timer_resp(long long requestid, int type, int node, int status);

#endif /* MQTT_HEADLER_H_ */
