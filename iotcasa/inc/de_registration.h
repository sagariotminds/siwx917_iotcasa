/*
 * de_registration.h
 *
 *  Created on: 13-Feb-2026
 *      Author: User
 */

#ifndef DE_REGISTRATION_H_
#define DE_REGISTRATION_H_

#include "iotcasa_types.h"
#include "registration.h"

#define  DEREG_ACTION                "deregistration"                /* De-registration action name */
#define  DE_REG_MQTT_PUB_TOPIC       "/device/deregistration"        /* De-registration MQTT Publish Topic */
#define  CLOUD_INVALID_DEVID_MSG     "Invalid device id provided"    /* description message from cloud */

#define  INVALID_USER_ID             401                             /* Invalid user id provided */
#define  INVALID_DEVICE_ID           402                             /* Invalid device id provided */
#define  INVALID_AUTH_TOKEN          403                             /* InValid auth token */
#define  SECURE_MODE_FAILURE         404                             /* Device delete failed due to set to secure mode */
#define  CLOUD_ERROR                 500                             /* Error code of Cloud response*/

#define  DEREG_SOURCE_LEN            10                              /* Length to store "manual" or "mobile" deregistration */
#define  CLOUD_DESC_MSG_LEN          100                             /* description message length from cloud */
#define  DEREG_AUTHTOK_LEN           1100                            /* length to store the de registration details json */
#define  DEREG_DETAILS_JSON_LEN      1500                            /* length to store the de registration details json */

#define  CLOUD_DEREG_RESP_TICKER     500                             /* Timer to check the de reg resp from cloud for every 500 milli seconds */
#define  CLOUD_DE_REG_RESP_TIMEOUT   40                              /* current = 20 secends, device de registration resp(cloud) timeout */

/* Device De-registration process vairables, created using malloc() and will be cleared after the deregistration process */
typedef struct de_registration_context_s {
    uint8_t       current_op;                                         /* Current De registration operation */
    char          authtoken[DEREG_AUTHTOK_LEN];                       /* Authorization token */
    char          dereg_details[DEREG_DETAILS_JSON_LEN];              /* De registration json buffer */
    char          dereg_from[DEREG_SOURCE_LEN];                       /* Buffer to store how the De registration is happening(manual or mobile) */
    char          cloud_description_msg[CLOUD_DESC_MSG_LEN];          /* Buffer to store cloud description message*/
    uint16_t      cloud_mqtt_dereg_status;                            /* Cloud mqtt de reg status */
    uint16_t      time_out;                                           /* De Registration time out */
    long long     requestId;                                          /* Received request id */
} de_registration_context_t;

/* Device De-registration process switch case enum variables */
typedef enum
{
    DE_REG_DETAILS_SHARING = 1,
    DE_REGISTRATION_PROCESS,
    DE_REGISTRATION_FINAL,
    WAIT_DEREG
} de_registration_current_operation_t;


/**
 * @brief     Exit the de-registration process due to failed scenarios.
 * @param[in] None
 * @return    None
 */
void exit_de_registration(void);

/**
 * @brief     Handle received data from the MQTT cloud or failure in the de-registration process.
 * @param[in] arg Pointer to data.
 * @return    None
 */
void cloud_dereg_status_callback(sl_sleeptimer_timer_handle_t *handle, void *data);

/**
 * @brief     Perform Casa deregistration process.
 * @param[in] None
 * @return    None
 */
void casa_deregistration(void);

/**
 * @brief     Construct device deregistration device details data to send to CASA cloud.
 * @param[in] None
 * @return    None
 */
void dereg_device_details_json(void);

#endif /* DE_REGISTRATION_H_ */
