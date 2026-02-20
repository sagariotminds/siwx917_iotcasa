/*
 * device_control.h
 *
 *  Created on: 12-Feb-2026
 *      Author: User
 */

#ifndef DEVICE_CONTROL_H_
#define DEVICE_CONTROL_H_

#include "iotcasa_types.h"
#include "casa_module.h"

#define CONTROL_MSG_FROM_LEN            10         /* Length from where the message is received (manual or mobile or Alexa or Google) */

/* Device endpoint info structure used as an array for mutltiple endpoints */
typedef struct endpoint_info_context_s {
    uint8_t   endpoint;                     /* Respective End point of the casa module */
    uint8_t   set_value;                    /* Respective End point value ON or OFF*/
} endpoint_info_context_t;

/* Device Control Structure */
typedef struct device_control_context_s {
    uint8_t                     endpoints_counter;                                /* recived endpoints itteration counter */
    uint8_t                     isgroup;                                          /* For group control setting it to 1 or else 0 */
    uint8_t                     call_type;                                        /* End device status update when offline to online = 1
                                                                                        (taken from Device and time trigger scene) Normal update = 0 */
    char                        msg_from[CONTROL_MSG_FROM_LEN];                   /* From where the message is received (manual or mobile or Alexa or Google)*/
    long long                   requestId;                                        /* Received request id for device control */
    endpoint_info_context_t     endpoint_info[NO_OF_ENDPOINTS];                   /* Creating the endpoint info sturcture array */
} device_control_context_t;



void casa_device_status_update(void);
void construct_status_update_json(void);

#endif /* DEVICE_CONTROL_H_ */
