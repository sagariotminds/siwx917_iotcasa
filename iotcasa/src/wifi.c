/*
 * wifi.c
 *
 *  Created on: 28-Jan-2026
 *      Author: User
 */

/* --- Wi-Fi & General Networking --- */
#include <casa_log.h>

#include "ble_config.h"
#include "wifi.h"
#include "casa_module.h"
#include "mqtt_handler.h"
//#include "udp.h"
/******************************************************
 *                    AP + STA  Macros
 ******************************************************/
#define MY_WIFI_CLIENT_PROFILE_SSID "YOUR_AP_SSID"
#define MY_WIFI_CLIENT_CREDENTIAL   "YOUR_AP_PASSPHRASE"

#define MY_WIFI_AP_PROFILE_SSID "iotminds_SiWX917"
#define MY_WIFI_AP_CREDENTIAL   "12345678"

#define WIFI_CLIENT_SECURITY_TYPE   SL_WIFI_WPA_WPA2_MIXED
#define WIFI_CLIENT_ENCRYPTION_TYPE SL_WIFI_CCMP_ENCRYPTION

#define MY_WIFI_MODULE_IP_ADDRESS 0x0104A8C0   // 192.168.4.1
#define MY_WIFI_GATEWAY_ADDRESS   0x0104A8C0   // 192.168.4.1
#define MY_WIFI_SN_MASK_ADDRESS   0x00FFFFFF  // 255.255.255.0


#define FLAG_WIFI_RECONNECT_REQ      (1 << 0)

#define REMOTE_IP_ADDRESS "8.8.8.8"
#define PING_COUNT        4
#define PING_SIZE         64
#define PING_TIMEOUT_MS   2000

static volatile bool is_wifi_connected = false;
casa_wifi_status_t casa_wifi_status = {0};
extern casa_context_t casa_ctx;
bool internet_status = false;
bool wifi_connected = false;  // Track previous connection state
osThreadId_t wifiConnectHandle = NULL;



const osThreadAttr_t wifi_thread_attributes = {
  .name       = "ap_app",
  .stack_size = 8000,
  .priority   = osPriorityHigh,
};

const sl_wifi_device_configuration_t sl_wifi_triple_mode_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = {
    // 1. Set to Concurrent (AP+STA) and enable WLAN+BLE Coexistence
    .oper_mode              = SL_SI91X_CONCURRENT_MODE,
    .coex_mode              = SL_SI91X_WLAN_BLE_MODE,

    // 2. Merged Feature Bit Map (Aggregation + PSK Security)
    .feature_bit_map        = (SL_SI91X_FEAT_WPS_DISABLE
                               | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE
                               | SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1
                               | SL_SI91X_FEAT_SECURITY_PSK      // Required for WPA2
                               | SL_WIFI_FEAT_AGGREGATION),

    // 3. TCP/IP Feature Bit Map - Fixed for DNS and SSL
    .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT
                               | SL_SI91X_TCP_IP_FEAT_DHCPV4_SERVER
                               | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT // FIX: Required for CloudAMQP URL
                               | SL_SI91X_TCP_IP_FEAT_SSL        // FIX: Required for Secure MQTT
                               | SL_SI91X_TCP_IP_FEAT_ICMP
                               | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),

    .custom_feature_bit_map = (SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID),

    .ext_custom_feature_bit_map = (SL_SI91X_EXT_FEAT_LOW_POWER_MODE
                                   | SL_SI91X_EXT_FEAT_XTAL_CLK
                                   | MEMORY_CONFIG
                                   | SL_SI91X_EXT_FEAT_SSL_VERSIONS_SUPPORT
                                   | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
                                   | SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE),

    .bt_feature_bit_map         = (SL_SI91X_BT_RF_TYPE | SL_SI91X_ENABLE_BLE_PROTOCOL),

    // 4. Extended TCP/IP - Fixed for Embedded MQTT
    .ext_tcp_ip_feature_bit_map = (SL_SI91X_CONFIG_FEAT_EXTENTION_VALID
                                   | SL_SI91X_EXT_TCP_IP_FEAT_SSL_THREE_SOCKETS
                                   | SL_SI91X_EXT_TCP_IP_FEAT_SSL_MEMORY_CLOUD
                                   | SL_SI91X_EXT_EMB_MQTT_ENABLE // FIX: Required for MQTT logic
#ifdef RSI_PROCESS_MAX_RX_DATA
                                   | SL_SI91X_EXT_TCP_MAX_RECV_LENGTH
#endif
                                   ),

    // 5. BLE Feature Set (Preserving your logic)
    .ble_feature_bit_map =
      ((SL_SI91X_BLE_MAX_NBR_PERIPHERALS(RSI_BLE_MAX_NBR_PERIPHERALS)
        | SL_SI91X_BLE_MAX_NBR_CENTRALS(RSI_BLE_MAX_NBR_CENTRALS)
        | SL_SI91X_BLE_MAX_NBR_ATT_SERV(RSI_BLE_MAX_NBR_ATT_SERV)
        | SL_SI91X_BLE_MAX_NBR_ATT_REC(RSI_BLE_MAX_NBR_ATT_REC))
       | SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENTION_VALID | SL_SI91X_BLE_PWR_INX(RSI_BLE_PWR_INX)
       | SL_SI91X_BLE_PWR_SAVE_OPTIONS(RSI_BLE_PWR_SAVE_OPTIONS) | SL_SI91X_916_BLE_COMPATIBLE_FEAT_ENABLE
#if RSI_BLE_GATT_ASYNC_ENABLE
       | SL_SI91X_BLE_GATT_ASYNC_ENABLE
#endif
       ),

       .ble_ext_feature_bit_map =
             ((SL_SI91X_BLE_NUM_CONN_EVENTS(RSI_BLE_NUM_CONN_EVENTS)
               | SL_SI91X_BLE_NUM_REC_BYTES(RSI_BLE_NUM_REC_BYTES))
       #if RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST
              | SL_SI91X_BLE_INDICATE_CONFIRMATION_FROM_HOST
       #endif
       #if RSI_BLE_MTU_EXCHANGE_FROM_HOST
              | SL_SI91X_BLE_MTU_EXCHANGE_FROM_HOST
       #endif
       #if RSI_BLE_SET_SCAN_RESP_DATA_FROM_HOST
              | (SL_SI91X_BLE_SET_SCAN_RESP_DATA_FROM_HOST)
       #endif
       #if RSI_BLE_DISABLE_CODED_PHY_FROM_HOST
              | (SL_SI91X_BLE_DISABLE_CODED_PHY_FROM_HOST)
       #endif
       #if BLE_SIMPLE_GATT
              | SL_SI91X_BLE_GATT_INIT
       #endif
              ),

    .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | SL_SI91X_ENABLE_ENHANCED_MAX_PSP)
  }
};

/**************************STA MODE********************************/

void wifi_close(void)
{
    sl_status_t status;

    status = sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);
    LOG_INFO("WIFI", "Execution: %s - Network interface down", (status == SL_STATUS_OK) ? "SUCCESS" : "ALREADY DOWN");

    status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
    LOG_INFO("WIFI", "Execution: %s - Network manager de-initialized", (status == SL_STATUS_OK) ? "SUCCESS" : "FAILED");
    status = sl_wifi_deinit();
    LOG_INFO("WIFI", "Execution: %s - Wi-Fi Radio hardware stopped", (status == SL_STATUS_OK) ? "SUCCESS" : "FAILED");
    sl_net_down(SL_NET_WIFI_AP_INTERFACE);
    sl_wifi_stop_ap(SL_WIFI_AP_INTERFACE);
}

void stop_wifi_client(void)
{
    sl_status_t status;
    // 1. Bring the Client interface down (Deactivates the link)
    status = sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);
    LOG_INFO("WIFI", "Execution: %s - STA Link Down (Status: 0x%lx)", (status == SL_STATUS_OK) ? "SUCCESS" : "ALREADY DOWN", status);

    // 2. De-initialize the Client network stack (Frees resources)
    status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
    LOG_INFO("WIFI", "Execution: %s - STA Network Deinit", (status == SL_STATUS_OK) ? "SUCCESS" : "FAILED");

    // 3. Power off the Wi-Fi Radio hardware
    sl_wifi_deinit();
}


static bool check_internet_connectivity(void)
{
  int sock;
  struct sockaddr_in server_addr;
  struct timeval timeout;

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0) {
    LOG_ERROR("WIFI", "Socket create failed");
    return false;
  }

  timeout.tv_sec  = 3;   // 3 seconds timeout
  timeout.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port   = htons(443);          // HTTPS (very reliable)
  uint32_t ip_addr;
  if (sl_net_inet_addr("8.8.8.8", &ip_addr) != SL_STATUS_OK) {
    LOG_ERROR("WIFI", "Invalid IP address");
    close(sock);
    return false;
  }

  server_addr.sin_addr.s_addr = ip_addr;

  LOG_INFO("WIFI", "Checking internet...");

  if (connect(sock,
              (struct sockaddr *)&server_addr,
              sizeof(server_addr)) == 0) {

    LOG_INFO("WIFI", "Internet AVAILABLE");
    close(sock);
    return true;
  }
  LOG_WARN("WIFI", "Internet NOT available");
  close(sock);
  printf("check_internet_connectivity end \r\n");
  return false;
}

sl_net_wifi_client_profile_t casa_sta_profile;

bool wifi_sta(const char *ssid, const char *password)
{

  printf("ssid %s, password %s\r\n",ssid,password);
  if (ssid == NULL || password == NULL) {
      return false;
  }
  sl_status_t status;
  sl_mac_address_t mac_addr = {0};
  sl_wifi_firmware_version_t fw_ver = {0};

  casa_sta_profile.config.channel.channel = SL_WIFI_AUTO_CHANNEL;
  casa_sta_profile.config.channel.band = SL_WIFI_AUTO_BAND;
  casa_sta_profile.config.channel.bandwidth = SL_WIFI_AUTO_BANDWIDTH;
  casa_sta_profile.config.bss_type = SL_WIFI_BSS_TYPE_INFRASTRUCTURE;
  casa_sta_profile.config.security = SL_WIFI_WPA_WPA2_MIXED;
  casa_sta_profile.config.encryption = SL_WIFI_CCMP_ENCRYPTION;
  casa_sta_profile.config.client_options = 0;
  casa_sta_profile.config.credential_id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;
  casa_sta_profile.ip.mode = SL_IP_MANAGEMENT_DHCP;
  casa_sta_profile.ip.type = SL_IPV4;

  size_t ssid_len = strlen(ssid);
  if (ssid_len >= sizeof(casa_sta_profile.config.ssid.value)) {
      LOG_ERROR("WIFI", "SSID too long");
      return false;
  }
  memset(casa_sta_profile.config.ssid.value, 0, sizeof(casa_sta_profile.config.ssid.value));
  memcpy(casa_sta_profile.config.ssid.value, ssid, ssid_len);
  casa_sta_profile.config.ssid.length = ssid_len;

  size_t host_len = strlen(casa_ctx.hostname);
//  if (host_len >= sizeof(casa_sta_profile.ip.host_name)) {
//      LOG_ERROR("WIFI", "Hostname too long");
//      return false;
//  }
  memcpy(casa_sta_profile.ip.host_name, casa_ctx.hostname, host_len);

  LOG_INFO("WIFI", "Initializing STA mode");

  if (casa_ctx.reg_status == REGISTRATION_DONE) {
      status = sl_net_wifi_client_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_triple_mode_configuration, NULL, NULL);
  } else {
      status = sl_net_wifi_client_init(SL_NET_WIFI_CLIENT_INTERFACE, &sl_wifi_triple_mode_configuration, NULL, NULL);
  }

  if (status != SL_STATUS_OK) {
      LOG_ERROR("WIFI", "Init failed: 0x%lx", status);
      return false;
  }

  // MAC Print
  if (sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_addr) == SL_STATUS_OK) {
      LOG_INFO("WIFI", "Device MAC: %02X:%02X:%02X:%02X:%02X:%02X", mac_addr.octet[0], mac_addr.octet[1], mac_addr.octet[2],
                                                                    mac_addr.octet[3], mac_addr.octet[4], mac_addr.octet[5]);
  }

  // Firmware Print - Using chip_id and patch_num for latest SDK
  if (sl_wifi_get_firmware_version(&fw_ver) == SL_STATUS_OK) {
      LOG_INFO("WIFI", "FW Version: %d.%d.%d.%d", fw_ver.chip_id, fw_ver.major, fw_ver.minor, fw_ver.patch_num);
  }

  status = sl_net_set_credential(SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID, SL_NET_WIFI_PSK, password, strlen(password));
  if (status != SL_STATUS_OK) {
      LOG_ERROR("WIFI", "Credential set failed: 0x%lx", status);
      return false;
  }

  status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1, &casa_sta_profile);
  if (status != SL_STATUS_OK) {
      LOG_ERROR("WIFI", "Profile set failed: 0x%lx", status);
      return false;
  }
  LOG_INFO("WIFI", "Attempting to connect to %s...", ssid);


  wifiConnectHandle = NULL;

  wifiConnectHandle = osThreadNew(wifi_sta_monitor_task, NULL, &wifi_thread_attributes);
  if (wifiConnectHandle != NULL) {
      LOG_INFO("WIFI", "Wi-Fi Connection checking task created.");
  } else {
      LOG_ERROR("WIFI", "Wi-Fi Connection checking task not created.");
  }

  int retry_count = 0;
    status = SL_STATUS_FAIL;
    bool net_up_triggered = false;

    LOG_INFO("WIFI", "Starting Wi-Fi connection sequence...");

    while (retry_count < WIFI_MAXIMUM_RETRY) {

        // 2. Trigger sl_net_up ONCE at the start (or if it failed to initiate)
        if (!net_up_triggered) {
            status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_PROFILE_ID_1);

            if (status == SL_STATUS_OK || status == SL_STATUS_IN_PROGRESS) {
                LOG_INFO("WIFI", "sl_net_up initiated (Status: 0x%lx).", status);
                net_up_triggered = true;
            } else {
                LOG_ERROR("WIFI", "sl_net_up failed to start: 0x%lx", status);
            }
        }

        // 3. Check for success (updated by monitor task/callbacks)
        if (casa_wifi_status.is_connected) {
              LOG_INFO("WIFI", "Connected Successfully!");
              is_wifi_connected = true;
              return true;
        }

        // 4. If not connected yet, wait and retry
        retry_count++;
        LOG_WARN("WIFI", "Waiting for connection... attempt %d/%d", retry_count, WIFI_MAXIMUM_RETRY);

        // Increased to 2000ms: Wi-Fi handshakes (DHCP) need time.
        // Fast polling can cause internal driver congestion.
        osDelay(1000);
    }
  return false;
}

bool station_wifi_detailes_get(sl_net_wifi_client_profile_t *profile) {
  if (profile == NULL) {
      return false;
  }
  
  // Check if IP is valid (not zero)
  if (profile->ip.ip.v4.ip_address.value == 0) {
      return false;
  }
  
  // Extract and populate IP (only done once on connection)
  uint8_t *ip = profile->ip.ip.v4.ip_address.bytes;
  snprintf(casa_wifi_status.ip_str, sizeof(casa_wifi_status.ip_str), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  
  // Extract and populate SSID (only done once on connection)
  if (profile->config.ssid.length > 0 && profile->config.ssid.length <= 32) {
      memcpy(casa_wifi_status.ssid, profile->config.ssid.value, profile->config.ssid.length);
      casa_wifi_status.ssid[profile->config.ssid.length] = '\0';
  }
  
  return true;
}

/**
 * Clear WiFi status when disconnected
 */
void station_wifi_status_clear(void) {
  memset(casa_wifi_status.ssid, 0, sizeof(casa_wifi_status.ssid));
  memset(casa_wifi_status.ip_str, 0, sizeof(casa_wifi_status.ip_str));
  casa_wifi_status.rssi = 0;
  casa_wifi_status.is_connected = false;
  check_internet_connectivity();
}

void wifi_sta_monitor_task(void *argument)
{
  UNUSED_PARAMETER(argument);

  printf("thread is created wifi\r\n");
//  int mqtt_started = 1;

  while (1) {
      int32_t rssi = 0;
      sl_net_wifi_client_profile_t profile = {0};

      // 1. Check current status
      sl_status_t rssi_status = sl_wifi_get_signal_strength(SL_WIFI_CLIENT_INTERFACE, &rssi);
      sl_net_get_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, (sl_net_profile_t *)&profile);

      casa_wifi_status.is_connected = (rssi_status == SL_STATUS_OK) && (rssi < 0) && (profile.ip.ip.v4.ip_address.value != 0);

      if (casa_wifi_status.is_connected) {
          // Always update RSSI every cycle
          casa_wifi_status.rssi = rssi;
          
          // Only update SSID/IP once on connection (not on every cycle)
          if (!wifi_connected) {
              // Transitioned from disconnected to connected
              if (station_wifi_detailes_get(&profile)) {
                  LOG_INFO("WIFI", "[Connected] SSID: %s | RSSI: %ld dBm | IP: %s", casa_wifi_status.ssid, casa_wifi_status.rssi, casa_wifi_status.ip_str);
                  wifi_connected = true;
              }
          } else {
              // Already connected, just log RSSI update periodically
              LOG_INFO("WIFI", "[Heartbeat] CONNECTED | SSID: %s | RSSI: %ld dBm | IP: %s", casa_wifi_status.ssid, casa_wifi_status.rssi, casa_wifi_status.ip_str);
          }

          if (check_internet_connectivity()) {
//          if (1) {
              LOG_INFO("WIFI", "Internet AVAILABLE");
              LOG_INFO("WIFI", "Internet OK ✅");
              internet_status = 1;

//              if(mqtt_started) {
//                  mqtt_connection_task_create();
//                  mqtt_started = 0;
//              }
          } else {
              LOG_ERROR("WIFI", "No Internet ❌");
              internet_status = 0;
          }
          osDelay(5000);
       }
      else {
          // Connection lost
          if (wifi_connected) {
              // Transitioned from connected to disconnected
              LOG_WARN("WIFI", "Link Lost or IP Invalid. Clearing status...");
              station_wifi_status_clear();
              wifi_connected = false;
          }
          
          internet_status = 0;

          sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);
          osDelay(2000);

          sl_status_t status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE,
                                      SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID,
                                      (const sl_net_profile_t *)&casa_sta_profile);

          if (status != SL_STATUS_OK) {
              LOG_ERROR("WIFI", "Profile Re-set failed: 0x%lx", status);
          }

          // Step C: Attempt Rejoin
          LOG_INFO("WIFI", "Attempting Rejoin to: %s", MY_WIFI_CLIENT_PROFILE_SSID);
          sl_status_t join_status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);

          if (join_status == SL_STATUS_OK) {
            LOG_INFO("WIFI", "Join Success. Waiting for DHCP...");
            osDelay(5000);
          } else {
            // If you get 0x21 here, it means my_sta_config_variable is empty or credential was lost
            LOG_ERROR("WIFI", "Join failed (0x%lx). Retrying in 5s...", join_status);
            osDelay(5000);
          }
      }
  }
}
