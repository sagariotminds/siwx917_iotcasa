/*******************************************************************************
 * @file  sl_si91x_ota_config.h
 * @brief OTA (Over-The-Air) Configuration for SiWX917
 *
 * Flash Partition Layout (8MB Flash):
 * 0x00000000 - 0x0007FFFF (512 KB)   - Bootloader (M4 core)
 * 0x00080000 - 0x0027FFFF (2 MB)     - Network Processor ROM
 * 0x00280000 - 0x0047FFFF (2 MB)     - Application Slot A (Active)
 * 0x00480000 - 0x0067FFFF (2 MB)     - Application Slot B (OTA Update)
 * 0x00680000 - 0x006FFFFF (512 KB)   - OTA Metadata
 * 0x00700000 - 0x007FFFFF (1 MB)     - NVM3 Storage & User Data
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 ******************************************************************************/

#ifndef SL_SI91X_OTA_CONFIG_H
#define SL_SI91X_OTA_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h>OTA Configuration

// <o SL_SI91X_OTA_SLOT_A_ADDR> Application Slot A (Primary) Address
// <i> Starting address of primary application partition
// <i> Default: 0x8280000 (Offset 0x00280000 from external flash base)
#define SL_SI91X_OTA_SLOT_A_ADDR 0x8280000

// <o SL_SI91X_OTA_SLOT_A_SIZE> Application Slot A Size
// <i> Size of application slot A (2 MB)
// <i> Default: 0x200000
#define SL_SI91X_OTA_SLOT_A_SIZE 0x200000

// <o SL_SI91X_OTA_SLOT_B_ADDR> Application Slot B (Alternate) Address
// <i> Starting address of alternate application partition for OTA updates
// <i> Default: 0x8480000 (Offset 0x00480000 from external flash base)
#define SL_SI91X_OTA_SLOT_B_ADDR 0x8480000

// <o SL_SI91X_OTA_SLOT_B_SIZE> Application Slot B Size
// <i> Size of application slot B (2 MB)
// <i> Default: 0x200000
#define SL_SI91X_OTA_SLOT_B_SIZE 0x200000

// <o SL_SI91X_OTA_METADATA_ADDR> OTA Metadata Address
// <i> Address where OTA metadata is stored
// <i> Default: 0x8680000 (Offset 0x00680000 from external flash base)
#define SL_SI91X_OTA_METADATA_ADDR 0x8680000

// <o SL_SI91X_OTA_METADATA_SIZE> OTA Metadata Size
// <i> Size allocated for OTA metadata (512 KB)
// <i> Default: 0x80000
#define SL_SI91X_OTA_METADATA_SIZE 0x80000

// <o SL_SI91X_NVM3_BASE_ADDR> NVM3 Storage Base Address
// <i> Starting address of NVM3 storage region
// <i> Default: 0x8700000 (Offset 0x00700000 from external flash base)
#define SL_SI91X_NVM3_BASE_ADDR 0x8700000

// <o SL_SI91X_NVM3_SIZE> NVM3 Storage Size
// <i> Size allocated for NVM3 storage (1 MB)
// <i> Default: 0x100000
#define SL_SI91X_NVM3_SIZE 0x100000

// <q SL_SI91X_OTA_DUAL_BANK_ENABLED> Enable Dual-Bank OTA Updates
// <i> When enabled, supports two application slots for atomic OTA updates
// <i> Default: 1
#define SL_SI91X_OTA_DUAL_BANK_ENABLED 1

// <q SL_SI91X_OTA_BOOTLOADER_ENABLED> Enable Bootloader Support
// <i> When enabled, bootloader validates and switches between slots
// <i> Default: 1
#define SL_SI91X_OTA_BOOTLOADER_ENABLED 1

// </h>

// <<< end of configuration section >>>

#endif // SL_SI91X_OTA_CONFIG_H
