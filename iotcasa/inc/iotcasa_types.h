/*
 * iotcasa_types.h
 *
 *  Created on: 04-Feb-2026
 *      Author: User
 */

#ifndef IOTCASA_INCLUDE_IOTCASA_TYPES_H_
#define IOTCASA_INCLUDE_IOTCASA_TYPES_H_
// Include the standard libraries once here
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sl_status.h"
#include "cmsis_os2.h"
#include "FreeRTOSConfig.h"
#include "sl_utility.h"
#include "sl_board_configuration.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_sleeptimer.h"
#include "cJSON.h"

#include <casa_log.h>

#define DELAY_10S    10            /* Task delay 10ms */
#define DELAY_50S    50            /* Task delay 50ms */
#define DELAY_100S   100           /* Task delay 100ms */
#define DELAY_150S   150           /* Task delay 150ms */
#define DELAY_200S   200           /* Task delay 200ms */
#define DELAY_500S   500           /* Task delay 500ms */
#define DELAY_1000S  1000          /* Task delay 1000ms */
#define DELAY_2000S  2000          /* Task delay 2000ms */
#define DELAY_3000S  3000          /* Task delay 3000ms */
#define DELAY_5000S  5000          /* Task delay 5000ms */
#define DELAY_50000S 50000         /* Task delay 50000ms */

#define  FAIL            0    /* Data parse fail */
#define  PARSE_SUCCESS   1    /* Data parse success */

#define ENABLE  1
#define DISABLE 0

#define MANUAL          "Manual"        /* switch operation via done manually info */
#define MOBILE          "mobile"        /* switch operation via done through mobile info */

#define SUCCESS         "success"       /* for success operations check */
#define FAILED          "failed"        /* for failed operations check */

#define CODE_SUCCESS    "0"             /* for success operations check */
#define CODE_FAILED     "-1"            /* for Cloud connect failed operations check */

//#define CONNECTED    true
//#define DISCONNECTED false

#endif /* IOTCASA_INCLUDE_IOTCASA_TYPES_H_ */
