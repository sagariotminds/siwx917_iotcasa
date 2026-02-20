/*
 * manual_control.h
 *
 *  Created on: 03-Feb-2026
 *      Author: User
 */

#ifndef IOTCASA_INCLUDE_MANUAL_CONTROL_H_
#define IOTCASA_INCLUDE_MANUAL_CONTROL_H_


#include "iotcasa_types.h"


#define  MANUAL_CONTROL     "Manual"
#define  TIMER_CONTROL      "Timer"
#define  DEVICE_CONTROL     "Device"

#define MULTI_CLICK_REQUIRED     10
#define MULTI_CLICK_WINDOW_MS    3500
#define SHORT_PRESS_THRESHOLD_MS 1000


#define  MANUAL_CTL_MODE_JSON_LEN    200      /* manual control mode change json responce length */
#define  HIGH      1
#define  LOW       0

typedef struct {
  bool active;            // Timer active status
  bool state;             // GPIO control state: true for ON, false for OFF
  long long start_time;     // Timer start time in clock ticks
  long long duration_ticks; // Timer duration in clock ticks
} time_based_control_t;


/**
 * @brief     Control the nodes which are pressed manually according to the
 *            toggle or normal operation and fill the device control structure for status update.
 * @param[in] index of the Endpoint to be controlled.
 * @return    None
 */
void manual_switch_handling(uint8_t index);

/**
 * @brief      Enable registration/deregistration via 10 button/switch clicks.
 * @param[in]  None
 * @return     None
 */
void reg_dereg_button(void);

/**
 * @brief     Handle the push button, record the 10 clicks and long press.
 * @param[in] Index of values for push button.
 * @return    None
 */
void push_button(uint8_t index);

/**
 * @brief     Handle the switches and count the number of switch clicks.
 * @param[in] pvParameters Pointer to parameters (void *).
 * @return    None
 */
void switches_handling_task(void *argument);

/**
 * @brief     Initialize GPIO pin configurations for buttons,
 *            CASA load, and LED.
 * @param[in] None
 * @return    None
 */
void gpio_init(void);


#endif /* IOTCASA_INCLUDE_MANUAL_CONTROL_H_ */
