/*
 * casa_msg_parser.h
 *
 *  Created on: 03-Feb-2026
 *      Author: User
 */

#ifndef IOTCASA_INCLUDE_CASA_MSG_PARSER_H_
#define IOTCASA_INCLUDE_CASA_MSG_PARSER_H_

#include "iotcasa_types.h"
#include "cJSON.h"

/* Operation codes recived for respective operations */
typedef enum
{
    UDP_BLE_REGISTRATION      = 103,
    CLOUD_REG_MQTT_RESP       = 104,
    DE_REGISTRATION_MSG       = 105,
    CONTROL_MSG               = 106,
    FOTA                      = 107,
    CLOUD_DEREG_MQTT_RESP     = 109,
    FOTA_MSG                  = 108,
    CHANGE_WIFI_CRED          = 110,
    WIFI_SIGNAL_STRENGTH_MSG  = 111,
    GET_ONLINE_OFFLINE_STATUS = 113,
    SWITCH_OPERATION_CHANGE   = 114,
    SECURE_DEVICE             = 115,
    GET_DEVICE_STATUS         = 116,
    ONLINE_RESP               = 121,
    TIMER_CTRL                = 124,
    TIMER_DEL                 = 125,
    HW_DATA_EDIT              = 130,
    CASA_SUPPORT              = 131,
    UDP_PING                  = 301,
    UDP_STATUS_UPDATE         = 302,
    UDP_ACK                   = 303,
} json_parser_current_operation_t;

/**
 * @brief     Parse casa message to process data received from the cloud MQTT.
 *            This function delegates control to respective functionalities/operations.
 * @param[in] casa_req Casa request data.
 * @param[in] Req_length Length of casa request data.
 * @return    1 (Success), 0 (Fail)
 */
int casa_message_parser(char *casa_req, int Req_length);

/**
 * @brief     Parse casa message to process data received for parsing registration function via BLE/UDP.
 * @param[in] json_parse  Casa request data.
 * @return    1 (Success), 0 (Fail)
 */
int registration_udp_ble_parser(cJSON *json_parse);



#endif /* IOTCASA_INCLUDE_CASA_MSG_PARSER_H_ */
