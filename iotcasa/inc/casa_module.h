/*
 * casa_module.h
 *
 *  Created on: 29-Jan-2026
 *      Author: User
 */

#ifndef IOTCASA_INCLUDE_CASA_MODULE_H_
#define IOTCASA_INCLUDE_CASA_MODULE_H_

#include "iotcasa_types.h"
#include "registration.h"
#include "de_registration.h"

#define NO_OF_ENDPOINTS 2               /* Number of endpoints supported by the CASA device */
#define HW_FLASH        1
#define MODEL           "ICR2S"
#define HW_VERSION      1.2             /* Hardware version of CASA module */
#define HW_BATCH        "23122025"      /* Hardware batch of CASA module */
#define HOSTNAME_LEN    20
#define UNIQEID_LEN     10
#define LOC_LEN         100             /* User Location maximum length */
#define USRID_LEN       50              /* User ID maximum length */
#define EVENT_BY_LEN    30              /* length of user name */
#define OP_HISTORY_SIZE 15              /* Device operations history length */
#define HW_BATCH_LEN    10              /* Hardware batch length */
#define SSID_LEN        32              /* WIFI router ssid name maximum length */
#define PSWD_LEN        64              /* WIFI router password maximum length */

typedef struct casa_context_s {
    char                        uniqueid[UNIQEID_LEN];  /* device uniqueID buffer data info */
    char                        hostname[HOSTNAME_LEN]; /* device hostname buffer data info */
    uint8_t                     current_operation;      /* device current operation status */
    uint8_t                     reg_status;             /* registraion status data info */
    char                        ssid[SSID_LEN];         /* WIFI SSID bufferdata info */
    char                        password[PSWD_LEN];     /* WIFI password buffer data info */
    char                        userid[USRID_LEN];      /* userID buffer data info */
    char                        location[LOC_LEN];      /* Location ID data info */
    uint8_t                     fota_status;            /* cheaks fota status; success or fail */
    float                       old_frmwre_ver;         /* To store the old firmware version */
    bool                        mqtt_ssl_connection;    /* MQTT connection type (secure or unsecure)*/
    uint8_t                     switch_op_state;        /* Switch toggle = 1; Normal = 0;   */
    char                        event_by[EVENT_BY_LEN]; /* Stores Name of the the person who registers the device at the point of registration */
    char                        event_by_id[USRID_LEN]; /* Stores the device specific userid */
    uint8_t                     reg_dereg_flag;         /* This flag Indicates that casa is in registration or deregistration mode if it is 1 else not */
    bool                        switch_interrupts;      /* Switch interrupts to enable or disable with respect to functionalities of the device */
    uint8_t                     secure_device;          /* Manul de-registration time if secure mode enable end device is not de-register
                                                           else device is de-registerd. default secure mode is 0  */
    uint8_t                     Reset_reason;           /* Get Reset reason code */
    float                       hardware_ver;           /* To store the hardware version */
    char                        hw_batch[HW_BATCH_LEN]; /* TO store the Hardware Batch number*/
    uint8_t                     url_src;                /* Mobile or Cloud initiate the FOTA update by putting source value. 0- GCP (default), 1- AWS. */

    registration_context_t*     registration;           /* device registration structure pointer of type registration_context_t */
    de_registration_context_t*  de_registration;        /* device de_registration structure pointer of type de_registration_context_t */
//    fota_context_t*             fota_update;            /* device fota update structure pointer of type fota_context_t */
//    wifi_cred_change_t*         wifi_cred_changing;     /* device wifi credentials change structure pointer of type wifi_cred_change_t */

} casa_context_t;


/* enum variables for checking the current device operation in loop switch case */
typedef enum
{
    REGISTRATION = 1,
    STATUS_UPDATE,
    DEREGISTRATION,
    WIFI_CRED_CHANGE,
    REG_STATUS,
    DE_REG_STATUS,
    FOTA_UPDATE,
    MQTT_UNSECURE,
    CASA_IDLE
} casa_current_operation_t;

typedef enum
{
    DEVICE_CONTROL_RESP = 208,
    MQTT_STABILITY = 209,
    WIFI_CRED_CHANGE_RESP_OPCODE  = 213,
    WIFI_SIG_STRENGTH_RESP_OPCODE,
    SSL_FOTA_UPDATE_REQ_OPCODE,
    ONLINE_OFFLINE_STATUS,
    LASTWILL_MSG,
    SENSING_MODE_CHANGE_CODE,
    DEVICE_SECURE_MODE,
    DEVICE_LOG = 221,
    TIMER_RESP = 225,
    TIMER_DELETE
} casa_resp_optcode_t;


void set_device_id_and_hostname();
void send_secure_device_resp(long long requestid);
void construct_device_status_resp(long long requestid);

#endif /* IOTCASA_INCLUDE_CASA_MODULE_H_ */
