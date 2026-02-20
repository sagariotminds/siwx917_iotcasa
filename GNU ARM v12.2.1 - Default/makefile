################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include wiseconnect3_sdk_3.5.2/third_party/json_parser/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/sli_wifi_command_engine/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/sli_si91x_wifi_event_handler/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/service/sl_http_server/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/service/network_manager/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/service/mqtt/si91x/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/service/bsd_socket/si91x_socket/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/protocol/wifi/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/protocol/wifi/si91x/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/socket/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/sl_net/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/memory/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/icmp/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/host_mcu/si91x/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/firmware_upgrade/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/errno/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ble/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/asynchronous_socket/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ahb_interface/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/unified_peripheral_drivers/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/unified_api/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/systemlevel/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/service/sleeptimer/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/service/clock_manager/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/cmsis_driver/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/core/config/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/core/common/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/core/chip/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/common/src/subdir.mk
-include wiseconnect3_sdk_3.5.2/components/board/silabs/src/subdir.mk
-include simplicity_sdk_2025.6.2/util/third_party/freertos/kernel/portable/MemMang/subdir.mk
-include simplicity_sdk_2025.6.2/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F/subdir.mk
-include simplicity_sdk_2025.6.2/util/third_party/freertos/kernel/subdir.mk
-include simplicity_sdk_2025.6.2/util/third_party/freertos/cmsis/Source/subdir.mk
-include simplicity_sdk_2025.6.2/platform/service/sleeptimer/src/subdir.mk
-include simplicity_sdk_2025.6.2/platform/service/sl_main/src/rtos/subdir.mk
-include simplicity_sdk_2025.6.2/platform/service/sl_main/src/subdir.mk
-include simplicity_sdk_2025.6.2/platform/emdrv/nvm3/src/subdir.mk
-include simplicity_sdk_2025.6.2/platform/common/src/subdir.mk
-include simplicity_sdk_2025.6.2/platform/CMSIS/RTOS2/Source/subdir.mk
-include simplicity_sdk_2025.6.2/app/common/util/app_timer/freertos/subdir.mk
-include iotcasa/third_party_components/cjson/subdir.mk
-include iotcasa/src/subdir.mk
-include autogen/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := IOTCASA_SiWX917
BUILD_ARTIFACT_EXTENSION := axf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all:
	+@$(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: IOTCASA_SiWX917.axf

# Tool invocations
IOTCASA_SiWX917.axf: $(OBJS) $(USER_OBJS) makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m4 -mthumb -T "C:\Users\User\SimplicityStudio\v5_workspace\IOTCASA_SiWX917\autogen\linkerfile_SoC.ld" -u _printf_float -Wl,--wrap=main -fno-lto -Wl,--no-warn-rwx-segments -Xlinker --gc-sections -Xlinker -Map="IOTCASA_SiWX917.map" -mfpu=fpv4-sp-d16 -mfloat-abi=softfp --specs=nano.specs -o IOTCASA_SiWX917.axf -Wl,--start-group "./app.o" "./main.o" "./autogen/sl_event_handler.o" "./iotcasa/src/bluetooth.o" "./iotcasa/src/casa_log.o" "./iotcasa/src/casa_module.o" "./iotcasa/src/casa_msg_parser.o" "./iotcasa/src/de_registration.o" "./iotcasa/src/device_control.o" "./iotcasa/src/eeprom_manage.o" "./iotcasa/src/gpio_control.o" "./iotcasa/src/mqtt_handler.o" "./iotcasa/src/registration.o" "./iotcasa/src/udp.o" "./iotcasa/src/wifi.o" "./iotcasa/third_party_components/cjson/cJSON.o" "./iotcasa/third_party_components/cjson/cJSON_Utils.o" "./simplicity_sdk_2025.6.2/app/common/util/app_timer/freertos/app_timer_freertos.o" "./simplicity_sdk_2025.6.2/platform/CMSIS/RTOS2/Source/os_systick.o" "./simplicity_sdk_2025.6.2/platform/common/src/sl_assert.o" "./simplicity_sdk_2025.6.2/platform/common/src/sl_cmsis_os2_common.o" "./simplicity_sdk_2025.6.2/platform/common/src/sl_core_cortexm.o" "./simplicity_sdk_2025.6.2/platform/common/src/sl_slist.o" "./simplicity_sdk_2025.6.2/platform/common/src/sl_string.o" "./simplicity_sdk_2025.6.2/platform/common/src/sl_syscalls.o" "./simplicity_sdk_2025.6.2/platform/common/src/sli_cmsis_os2_ext_task_register.o" "./simplicity_sdk_2025.6.2/platform/emdrv/nvm3/src/nvm3.o" "./simplicity_sdk_2025.6.2/platform/emdrv/nvm3/src/nvm3_cache.o" "./simplicity_sdk_2025.6.2/platform/emdrv/nvm3/src/nvm3_default_common_linker.o" "./simplicity_sdk_2025.6.2/platform/emdrv/nvm3/src/nvm3_lock.o" "./simplicity_sdk_2025.6.2/platform/emdrv/nvm3/src/nvm3_object.o" "./simplicity_sdk_2025.6.2/platform/emdrv/nvm3/src/nvm3_page.o" "./simplicity_sdk_2025.6.2/platform/emdrv/nvm3/src/nvm3_utils.o" "./simplicity_sdk_2025.6.2/platform/service/sl_main/src/rtos/main_retarget.o" "./simplicity_sdk_2025.6.2/platform/service/sl_main/src/sl_main_init.o" "./simplicity_sdk_2025.6.2/platform/service/sl_main/src/sl_main_init_memory.o" "./simplicity_sdk_2025.6.2/platform/service/sl_main/src/sl_main_kernel.o" "./simplicity_sdk_2025.6.2/platform/service/sleeptimer/src/sl_sleeptimer.o" "./simplicity_sdk_2025.6.2/util/third_party/freertos/cmsis/Source/cmsis_os2.o" "./simplicity_sdk_2025.6.2/util/third_party/freertos/kernel/croutine.o" "./simplicity_sdk_2025.6.2/util/third_party/freertos/kernel/event_groups.o" "./simplicity_sdk_2025.6.2/util/third_party/freertos/kernel/list.o" "./simplicity_sdk_2025.6.2/util/third_party/freertos/kernel/queue.o" "./simplicity_sdk_2025.6.2/util/third_party/freertos/kernel/stream_buffer.o" "./simplicity_sdk_2025.6.2/util/third_party/freertos/kernel/tasks.o" "./simplicity_sdk_2025.6.2/util/third_party/freertos/kernel/timers.o" "./simplicity_sdk_2025.6.2/util/third_party/freertos/kernel/portable/GCC/ARM_CM4F/port.o" "./simplicity_sdk_2025.6.2/util/third_party/freertos/kernel/portable/MemMang/heap_4.o" "./wiseconnect3_sdk_3.5.2/components/board/silabs/src/rsi_board.o" "./wiseconnect3_sdk_3.5.2/components/common/src/sl_utility.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/ipmu_apis.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/core/chip/src/iPMU_prog/iPMU_dotc/rsi_system_config_917.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/core/chip/src/rsi_deepsleep_soc.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/core/chip/src/rsi_ps_ram_func.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/core/chip/src/startup_si91x.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/core/chip/src/system_si91x.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/core/common/src/malloc_thread_safety.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/core/common/src/rsi_debug.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/core/common/src/sl_si91x_stack_object_declare.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/core/common/src/syscalls.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/core/config/src/rsi_nvic_priorities_config.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/cmsis_driver/UDMA.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/cmsis_driver/USART.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/aux_reference_volt_config.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/clock_update.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_adc.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_crc.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_dac.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_egpio.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_opamp.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_sysrtc.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_udma_wrapper.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/peripheral_drivers/src/rsi_usart.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/service/clock_manager/src/sl_si91x_clock_manager.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_common_flash_intf.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/service/nvm3/src/sl_si91x_nvm3_hal_flash.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/service/sleeptimer/src/sl_sleeptimer_hal_si91x_sysrtc.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_bod.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ipmu.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_pll.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_power_save.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_rtc.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_temp_sensor.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_time_period.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_ulpss_clk.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/systemlevel/src/rsi_wwdt.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/unified_api/src/sl_si91x_adc.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/unified_api/src/sl_si91x_bjt_temperature_sensor.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/unified_api/src/sl_si91x_dma.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/unified_api/src/sl_si91x_driver_gpio.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/unified_api/src/sl_si91x_watchdog_timer.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/mcu/drivers/unified_peripheral_drivers/src/sl_si91x_peripheral_gpio.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_ram.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ahb_interface/src/rsi_hal_mcu_m4_rom.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ahb_interface/src/sl_platform_wireless.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_bus.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ahb_interface/src/sl_si91x_timer.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ahb_interface/src/sli_siwx917_soc.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/asynchronous_socket/src/sl_si91x_socket.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ble/src/rsi_ble_gap_apis.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ble/src/rsi_ble_gatt_apis.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ble/src/rsi_bt_ble.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ble/src/rsi_bt_common_apis.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ble/src/rsi_common_apis.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ble/src/rsi_utils.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/ble/src/sl_si91x_ble.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/errno/src/sl_si91x_errno.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/firmware_upgrade/firmware_upgradation.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/host_mcu/si91x/siwx917_soc_ncp_host.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/icmp/sl_net_ping.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/memory/malloc_buffers.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/sl_net/src/sl_net_rsi_utility.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/sl_net/src/sl_net_si91x.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/sl_net/src/sl_net_si91x_callback_framework.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/sl_net/src/sl_net_si91x_integration_handler.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/sl_net/src/sl_si91x_net_internal_stack.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/sl_net/src/sli_net_si91x_utility.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/socket/src/sl_si91x_socket_utility.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/src/sl_rsi_utility.o" "./wiseconnect3_sdk_3.5.2/components/device/silabs/si91x/wireless/src/sl_si91x_driver.o" "./wiseconnect3_sdk_3.5.2/components/protocol/wifi/si91x/sl_wifi.o" "./wiseconnect3_sdk_3.5.2/components/protocol/wifi/src/sl_wifi_basic_credentials.o" "./wiseconnect3_sdk_3.5.2/components/protocol/wifi/src/sl_wifi_callback_framework.o" "./wiseconnect3_sdk_3.5.2/components/service/bsd_socket/si91x_socket/sl_si91x_bsd_socket.o" "./wiseconnect3_sdk_3.5.2/components/service/mqtt/si91x/sl_mqtt_client.o" "./wiseconnect3_sdk_3.5.2/components/service/network_manager/src/sl_net.o" "./wiseconnect3_sdk_3.5.2/components/service/network_manager/src/sl_net_basic_certificate_store.o" "./wiseconnect3_sdk_3.5.2/components/service/network_manager/src/sl_net_basic_profiles.o" "./wiseconnect3_sdk_3.5.2/components/service/network_manager/src/sl_net_credentials.o" "./wiseconnect3_sdk_3.5.2/components/service/network_manager/src/sli_net_common_utility.o" "./wiseconnect3_sdk_3.5.2/components/service/sl_http_server/src/sl_http_server.o" "./wiseconnect3_sdk_3.5.2/components/sli_si91x_wifi_event_handler/src/sli_si91x_wifi_event_handler.o" "./wiseconnect3_sdk_3.5.2/components/sli_wifi_command_engine/src/sli_wifi_command_engine.o" "./wiseconnect3_sdk_3.5.2/third_party/json_parser/src/jsmn.o" -lgcc -lnosys -lc -lm -Wl,--end-group -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building out file: IOTCASA_SiWX917.out'
	arm-none-eabi-objcopy "IOTCASA_SiWX917.axf" "IOTCASA_SiWX917.out"
	@echo ' '

	@echo 'Building hex file: IOTCASA_SiWX917.hex'
	arm-none-eabi-objcopy -O ihex "IOTCASA_SiWX917.axf" "IOTCASA_SiWX917.hex"
	@echo ' '

	@echo 'Building bin file: IOTCASA_SiWX917.bin'
	arm-none-eabi-objcopy -O binary "IOTCASA_SiWX917.axf" "IOTCASA_SiWX917.bin"
	@echo ' '

	@echo 'Building s37 file: IOTCASA_SiWX917.s37'
	arm-none-eabi-objcopy -O srec "IOTCASA_SiWX917.axf" "IOTCASA_SiWX917.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "IOTCASA_SiWX917.axf" -A
	@echo ' '

# Other Targets
clean:
	-@$(foreach FILE, $(EXECUTABLES), $(shell $(RM) $(FILE)) )
	-@$(foreach FILE, $(OBJS), $(shell $(RM) $(FILE)) )
	-@$(foreach FILE, $(C_DEPS), $(shell $(RM) $(FILE)) )
	-$(RM) IOTCASA_SiWX917.axf
	-@echo ' '

post-build:
	-"C:\SiliconLabs\SimplicityStudio\v5\developer\adapter_packs\commander\commander.exe" postbuild "C:\Users\User\SimplicityStudio\v5_workspace\IOTCASA_SiWX917/IOTCASA_SiWX917.slpb" --parameter build_dir:"C:\Users\User\SimplicityStudio\v5_workspace\IOTCASA_SiWX917\GNU ARM v12.2.1 - Default"
	-@echo ' '

.PHONY: all clean dependents main-build post-build

-include ../makefile.targets
