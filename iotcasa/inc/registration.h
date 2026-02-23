/*
 * registration.h
 *
 *  Created on: 04-Feb-2026
 *      Author: User
 */

#ifndef IOTCASA_INCLUDE_REGISTRATION_H_
#define IOTCASA_INCLUDE_REGISTRATION_H_

#include "iotcasa_types.h"

#define  PROTOCOL                  "wifi-casa"                     /* protocol name of CASA module */
#define  ACTION_REG                "registration"                  /* registration action string */
#define  DEVICE_TYPE               "actuator"                      /* device type of CASA module */
#define  POWER_SOURCE              "mains"                         /* CASA device power source */
#define  MANUFACTURER              "IoTMinds"                      /* Manufacturer name of the casa module */
#define  NODETYPE_SWITCH           "switch"                        /* Node type of CASA module */
#define  CLOUD_REG_MQTT_TOPIC      "/device/registration"          /* Cloud registration mqtt topic */

#define  AUTHTOK_LEN               1000                             /* Authentication token maximum length */
#define  ENDPOINT_LEN              35                              /* Endpoint data maximum length */
#define  DISC_TOK_LEN              50                              /* Discovery token maximum length */
#define  REG_BUFFER_SIZE           1000                            /* Registration buffer size maximum length */
#define  DISCOVERY_JSON_LEN        1500                            /* Discovery json maximum length */
#define  PERIODIC_TIMER            500                             /* 1000ms = 1 seconds, so 500ms = 0.5 seconds */
#define  CLOUD_CODE_OK             200                             /* value for checking Cloud has recived data and sends success 200 response*/
#define  REG_FINAL_RESP_LEN        200                             /* Registration final response maximum length */
#define  REGISTRATION_DONE         1                               /* Registration done status */
#define  SWITCH_ENABLE_TIME        10                              /* Time to enable switches. 1 unit = 500 ms ex: 10 = 5 seconds */
#define  REG_TIMEOUT               240                             /* Maximum Registration time 1 unit = 500 ms ex: 240 = 120 seconds*/
#define  REG_REQ_RCVD_RESP_LEN     200                             /* Registration request recived response maximum length */
#define  NWK_STS_RESP_LEN          200                             /* Network Status Response maximum length */
#define  CLOUD_REG_RESP_TIME       120                             /* Cloud registration response time 1 unit = 500 ms; ex: 40 = 20 seconds */

#define  NET_ACTION                "nwk_con_update"


/* casa registration context structure */
typedef struct registration_context_s {

    long long          requestId;                                   /* received request ID for registration*/
    uint8_t            current_op;                                  /* current registration operation switch loop*/
    uint8_t            buffer_status;                               /* structure buffer status (sturcture filled or not) recived request from the mobile app via udp/ble */
    uint32_t           time_out;                                    /* couter for checking the timeout */
    uint16_t           ble_data_counter;                            /* counter to check ble data size from consecutive ble packets received from mobile app */
    char               ble_buffer[REG_BUFFER_SIZE];                 /* BLE data buffer */
//    char               udp_buffer[REG_BUFFER_SIZE];                 /* UDP data buffer */
    char               discovery_token[DISC_TOK_LEN];               /* Discovery token buffer */
    char               authtoken[AUTHTOK_LEN];                      /* Authentication token buffer */
    char               casa_device_details[DISCOVERY_JSON_LEN];     /* casa device details at the time of Discovery */
    uint16_t           cloud_mqtt_reg_status;                       /* cloud mqtt send data. waiting for data recive status flag wrt cloud */
    uint8_t            cloud_mqtt_fota_status;                      /* cloud mqtt send data. waiting for data recive status flag wrt cloud */
    uint16_t           reg_fota_status;                             /* Cloud mqtt sends the FOTA url if the SW update is available at the time of registration.*/
} registration_context_t;

/* Registration process operations enum variables for switch case */
typedef enum
{
    REGISTRATION_INIT = 1,
    REG_REQ_PARSE,
    REG_WIFI_MQTT_INIT,
    REG_DEVICE_DETAILS_SHARING,
    REGISTRATION_PROCESS,
    REGISTRATION_FINAL,
    WAIT,
    REGISTRATION_FOTA_UPDATE_CHECK
} registration_current_operation_t;

void casa_registration(void);
void reg_req_recvd_response(char response[REG_REQ_RCVD_RESP_LEN]);
void wifi_status_response(char response[NWK_STS_RESP_LEN]);
void discovery_device_details_json(void);
void create_discovery_resp_to_mobile(char buf[REG_FINAL_RESP_LEN], const char *status_resp, const char *msg);
#endif /* IOTCASA_INCLUDE_REGISTRATION_H_ */
