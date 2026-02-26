/*
 * manual_control.c
 *
 *  Created on: 03-Feb-2026
 *      Author: User
 */


#include "gpio_control.h"
#include "casa_module.h"
#include "mqtt_handler.h"
#include "device_control.h"

static const char *TAG = "GPIO";

static uint32_t btn_press_time_ms[NO_OF_ENDPOINTS+1];
static uint32_t btn_release_time_ms[NO_OF_ENDPOINTS+1];
static uint8_t  btn_long_press_active[NO_OF_ENDPOINTS+1];
static uint8_t  btn_click_count[NO_OF_ENDPOINTS+1];
static uint8_t  btn_edge_flag[NO_OF_ENDPOINTS+1] = {true};
static bool     btn_window_active[NO_OF_ENDPOINTS+1];
static uint8_t  btn_released_event[NO_OF_ENDPOINTS+1];
long int time_current = 0;                                          /* To get current ESP time ( in micro-seconds ) */
uint32_t multi_click_start_time_ms;

time_based_control_t gpio_timer[NO_OF_ENDPOINTS];
bool timer_ctrl_status[NO_OF_ENDPOINTS] = {false};

extern casa_context_t casa_ctx;
extern device_control_context_t device_control;

/* ================== GPIO CONFIG ================== */

/* -----------------------------------------------------------------------------------------------------
 * SiWx917 GPIO PORT & DOMAIN MAPPING (WiseConnect 3.x / Si91x Peripheral Driver)
 * Reference: https://docs.silabs.com/wiseconnect/latest/wifi-siwg917-gpio-debugging/gpio-configuration
 * ------------------------------------------------------------------------------------------------------
 * PORT  | DOMAIN    | SDK CONSTANT         | PIN RANGE          | CHARACTERISTICS
 * ------------------------------------------------------------------------------------------------------
 * 0     | HP        | SL_GPIO_PORT_0       | GPIO_0  - GPIO_15  | High Power (M4 Domain)
 * 1     | HP        | SL_GPIO_PORT_1       | GPIO_16 - GPIO_31  | High Power (M4 Domain)
 * 2     | HP        | SL_GPIO_PORT_2       | GPIO_32 - GPIO_47  | High Power (M4 Domain)
 * 3     | HP        | SL_GPIO_PORT_3       | GPIO_48 - GPIO_57  | High Power (M4 Domain)
 * 4     | ULP       | SL_GPIO_ULP_PORT     | ULP_0   - ULP_11   | Ultra Low Power Domain
 * 5     | UULP_VBAT | SL_GPIO_UULP_PORT    | UULP_0  - UULP_3   | VBAT Domain (Deep Sleep)
 * ------------------------------------------------------------------------------------------------------
 * B24 Mapping: Port 5 (UULP_VBAT), Pin 2  (RTE_BUTTON0)
 * A37 Mapping: Port 4 (ULP),       Pin 2  (ULP_GPIO2 / RTE_LED0)
 * P11 Mapping: Port 0 (HP),        Pin 11 (GPIO11 / RTE_BUTTON1)
 * P24 Mapping: Port 1 (HP),        Pin 24 (GPIO24)
 * P45 Mapping: Port 2 (HP),        Pin 45 (GPIO45)
 * P50 Mapping: Port 3 (HP),        Pin 50 (GPIO50)
 * -------------------------------------------------------------------------------------------------------
 * SDK API USAGE:
 * Ports 0-4 : sl_si91x_gpio_set_pin_direction(PORT, PIN, DIRECTION);
 * Port 5    : sl_si91x_gpio_set_uulp_vbat_pin_direction(PIN, DIRECTION);
 * -------------------------------------------------------------------------------------------------------*/

sl_si91x_gpio_pin_config_t btn_gpio_cfg[] = {
  { {0, 26}, GPIO_INPUT },  // [0] multi function button
  { {5, 2},  GPIO_INPUT },  // [1] BUTTON0
  { {0, 11}, GPIO_INPUT }   // [2] BUTTON1 (B4)
};

sl_si91x_gpio_pin_config_t load_gpio_cfg[] = {
  { {0, 9},  GPIO_OUTPUT },  // [0] INDICATER LED
  { {4, 2},  GPIO_OUTPUT },  // [1] LED0
  { {0, 10}, GPIO_OUTPUT }   // [2] LED1
};

/* ================== LED ACTION ================== */

void manual_switch_handling(uint8_t index)
{
    uint8_t gpio_level = 0;

    // Toggle logic for Si917
    sl_gpio_driver_toggle_pin(&load_gpio_cfg[index].port_pin);
    sl_gpio_driver_get_pin(&load_gpio_cfg[index].port_pin, &gpio_level);
    // Populate device control for status update
    memset(&device_control, 0, sizeof(device_control));
    device_control.endpoints_counter = 1;
    device_control.endpoint_info[0].endpoint = index;
    device_control.endpoint_info[0].set_value = gpio_level;
    strcpy(device_control.msg_from, MANUAL_CONTROL);

    casa_device_status_update();
    LOG_INFO(TAG, "Manual control Endpoint %d state %d", index, gpio_level);
}


void reg_dereg_button(void)
{
    LOG_WARN(TAG, "MANUAL INTERRUPT HANDLER Registration/ De-registration mode initiated");
    if (casa_ctx.reg_status != REGISTRATION_DONE) {
        memset(&casa_ctx, 0, sizeof( casa_context_t ));
        casa_ctx.current_operation = REGISTRATION;
    } else if (casa_ctx.reg_status == REGISTRATION_DONE) {
        casa_ctx.de_registration = NULL;
        memset(casa_ctx.event_by, '\0', EVENT_BY_LEN);
        memset(casa_ctx.event_by_id, '\0', USRID_LEN);
        strcpy(casa_ctx.event_by, MANUAL_CONTROL);
        strncpy(casa_ctx.event_by_id, casa_ctx.userid, strlen(casa_ctx.userid));
        casa_ctx.current_operation = DEREGISTRATION;
    }
}

//static void led_blink_sequence(uint8_t index)
//{
//  for (uint8_t i = 0; i < 10; i++) {
//    sl_gpio_driver_toggle_pin(&load_gpio_cfg[index].port_pin);
//    osDelay(200);
//  }
//  sl_gpio_driver_clear_pin(&load_gpio_cfg[index].port_pin);
//}

/* ================== BUTTON HANDLER ================== */

void push_button(uint8_t index)
{
  uint8_t gpio_level = 1;

  sl_gpio_driver_get_pin(&btn_gpio_cfg[index].port_pin, &gpio_level);

  /* -------- PRESS DETECT -------- */
  if (gpio_level == 0 && btn_edge_flag[index]) {
    btn_press_time_ms[index] = osKernelGetTickCount();
    btn_long_press_active[index] = 1;
    btn_edge_flag[index] = 0;
    LOG_INFO(TAG, "BTN %d PRESSED", index);
  }

  /* -------- RELEASE DETECT -------- */
  else if (gpio_level == 1 && !btn_edge_flag[index]) {
    btn_release_time_ms[index] = osKernelGetTickCount();
    btn_edge_flag[index] = 1;
    btn_released_event[index] = 1;
    btn_long_press_active[index] = 0;
    LOG_INFO(TAG, "BTN %d RELEASED", index);
  }

  time_current = osKernelGetTickCount();

  /* -------- MULTI CLICK -------- */
  if ((btn_release_time_ms[index] - btn_press_time_ms[index] < SHORT_PRESS_THRESHOLD_MS) && btn_released_event[index]) {

    if (!btn_window_active[index]) {
      memset(btn_window_active, false, sizeof(btn_window_active));
      memset(btn_click_count, 0, sizeof(btn_click_count));

      btn_window_active[index] = true;
      multi_click_start_time_ms = osKernelGetTickCount();
    }

    btn_click_count[index]++;
    btn_released_event[index] = 0;
    manual_switch_handling(index);

    LOG_INFO(TAG, "BTN %d CLICK COUNT = %d", index, btn_click_count[index]);
  }

  /* -------- SHORT PRESS -------- */
  else if ((btn_release_time_ms[index] - btn_press_time_ms[index] >= SHORT_PRESS_THRESHOLD_MS) &&
           (btn_release_time_ms[index] - btn_press_time_ms[index] <= MULTI_CLICK_WINDOW_MS) &&
           btn_released_event[index]) {

    btn_released_event[index] = 0;
    LOG_WARN(TAG, "BTN %d SHORT PRESS", index);
//    led_blink_sequence(index);
  }

  /* -------- LONG PRESS -------- */
  else if ((time_current - btn_press_time_ms[index] > MULTI_CLICK_WINDOW_MS) && btn_long_press_active[index]) {

    LOG_WARN("GPIO", "BTN %d LONG PRESS", index);
    reg_dereg_button();

    while (1) {
      sl_gpio_driver_get_pin(&btn_gpio_cfg[index].port_pin, &gpio_level);
      osDelay(50);
      if (gpio_level == 1) {
        btn_long_press_active[index] = 0;
        btn_released_event[index] = 0;
        btn_edge_flag[index] = 1;
        break;
      }
    }
  }

  /* -------- MULTI CLICK VALIDATION -------- */
  if (btn_click_count[index] && btn_window_active[index]) {

    if ((time_current - multi_click_start_time_ms <= MULTI_CLICK_WINDOW_MS) &&
        (btn_click_count[index] == MULTI_CLICK_REQUIRED)) {

      LOG_INFO("GPIO", "BTN %d MULTI CLICK SUCCESS", index);
      reg_dereg_button();

      btn_click_count[index] = 0;
      btn_window_active[index] = false;
    }
    else if (time_current - multi_click_start_time_ms > MULTI_CLICK_WINDOW_MS) {

      LOG_WARN("GPIO", "BTN %d MULTI CLICK TIMEOUT", index);

      btn_click_count[index] = 0;
      btn_window_active[index] = false;
    }
  }
}

/* ================== APPLICATION TASK ================== */

void switches_handling_task(void *argument)
{
    UNUSED_PARAMETER(argument);

    int idx = 0;
    for( idx = 0; idx <= NO_OF_ENDPOINTS; idx++)
    {
        btn_edge_flag[idx] = true;               /* When restart Not enter button released or pressed conditions*/
        btn_window_active[idx] = false;
        btn_released_event[idx] =false;
    }
    idx =0;
    long long current_timer = 0;

    casa_ctx.switch_interrupts = ENABLE;

    LOG_INFO("GPIO", "GPIO task created");
    while (1) {
        if (casa_ctx.switch_interrupts == ENABLE) {
            for (uint8_t i = 1; i <= NO_OF_ENDPOINTS; i++) {
                push_button(i);
                current_timer = osKernelGetTickCount();
                if (i < NO_OF_ENDPOINTS && gpio_timer[i].active && !timer_ctrl_status[i]) {
                    if (current_timer - gpio_timer[i].start_time >= gpio_timer[i].duration_ticks) {
                        LOG_WARN(TAG,"current_timer - gpio_timer[%d].start_time :%lld sec >= %lld \n",idx,((current_timer - gpio_timer[idx].start_time)),(gpio_timer[idx].duration_ticks));
                        timer_ctrl_status[i] = true;
                         manual_switch_handling(i + 1);
                    }
                }
            }
        }
      osDelay(20);
    }
}

/* ================== INIT ================== */

void gpio_init(void)
{
  LOG_INFO(TAG, "GPIO INIT");
  sl_gpio_driver_init();

  for (uint8_t i = 0; i <= NO_OF_ENDPOINTS; i++) {
    sl_gpio_set_configuration(btn_gpio_cfg[i]);
    sl_gpio_set_configuration(load_gpio_cfg[i]);
    sl_gpio_driver_clear_pin(&load_gpio_cfg[i].port_pin);
  }
  osThreadNew((osThreadFunc_t)switches_handling_task, NULL, NULL);
  sl_gpio_driver_clear_pin(&load_gpio_cfg[0].port_pin);
}
