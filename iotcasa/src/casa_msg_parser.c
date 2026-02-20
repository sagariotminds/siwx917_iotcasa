/*
 * casa_msg_parser.c
 *
 *  Created on: 03-Feb-2026
 *      Author: User
 */

#include <casa_log.h>
#include "casa_msg_parser.h"
#include "casa_module.h"
#include "registration.h"

extern casa_context_t casa_ctx;


int registration_udp_ble_parser(cJSON *json_parse)
{
    const char *authtoken_json = NULL;


    if (casa_ctx.registration == NULL) {
        LOG_WARN("PARSER", "Registration structure context is not initialised");

        casa_ctx.registration = (registration_context_t*) malloc (sizeof(registration_context_t));
        if (casa_ctx.registration == NULL) {
            LOG_ERROR("PARSER", "Malloc failed");
            return FAIL;
        } else {
            LOG_DEBUG("PARSER", "Malloc succeeded");
            memset(casa_ctx.registration, 0 , sizeof(registration_context_t));
        }

//        return FAIL;
    }

    casa_ctx.registration->requestId = cJSON_GetObjectItem(json_parse, "requestId")->valuedouble;     /* parsing requestId registraion struct */

    cJSON *ssid_json = cJSON_GetObjectItem(json_parse, "ssid");                                       /* parsing wifi ssid to casa struct */
    if (ssid_json == NULL) {
        LOG_ERROR("PARSER", "SSID JSON not found");
//        construct_mqtt_device_log_msg(E_WIFI_SSID,L_REGISTRATION);
        return FAIL;
    }
    strncpy(casa_ctx.ssid, ssid_json->valuestring, strlen(ssid_json->valuestring));
    LOG_INFO("PARSER", "SSID: %s", casa_ctx.ssid);

    cJSON *pwd_json = cJSON_GetObjectItem(json_parse, "password");                                    /* parsing wifi password  to casa struct */
    if (pwd_json == NULL) {
        LOG_ERROR("PARSER", "Password JSON not found");
//        construct_mqtt_device_log_msg(E_WIFI_PWD,L_REGISTRATION);
        return FAIL;
    }
    strncpy(casa_ctx.password, pwd_json->valuestring, strlen(pwd_json->valuestring));
    LOG_INFO("PARSER", "Password: %s", casa_ctx.password);

    cJSON *discovery_token_json = cJSON_GetObjectItem(json_parse, "token");                           /* parsing discovery token to registration struct */
    if (discovery_token_json == NULL) {
        LOG_ERROR("PARSER", "Discovery token JSON not found");
//        construct_mqtt_device_log_msg(E_TOKEN,L_REGISTRATION);
        return FAIL;
    }
    strncpy(casa_ctx.registration->discovery_token, discovery_token_json->valuestring, strlen(discovery_token_json->valuestring));
    LOG_INFO("PARSER", "Discovery token: %s", casa_ctx.registration->discovery_token);

    cJSON *userid_json = cJSON_GetObjectItem(json_parse, "ownedBy");                                   /* parsing user ID to casa struct */
    if (userid_json == NULL) {
        LOG_ERROR("PARSER", "User ID JSON not found");
//        construct_mqtt_device_log_msg(E_USER_ID,L_REGISTRATION);
        return FAIL;
    }
    strncpy(casa_ctx.userid, userid_json->valuestring, strlen(userid_json->valuestring));
    LOG_INFO("PARSER", "User ID: %s", casa_ctx.userid);

    cJSON *event_by_id_json = cJSON_GetObjectItem(json_parse, "eById");
    if (event_by_id_json == NULL) {
        LOG_ERROR("PARSER", "Event By ID JSON not found");
//        construct_mqtt_device_log_msg(E_EBY_ID,L_REGISTRATION);
        return FAIL;
    }
    memset(casa_ctx.event_by_id, '\0', USRID_LEN);
    strncpy(casa_ctx.event_by_id, event_by_id_json->valuestring, strlen(event_by_id_json->valuestring));
    LOG_INFO("PARSER", "Event By ID: %s", casa_ctx.event_by_id);

    cJSON *event_by_json = cJSON_GetObjectItem(json_parse, "eBy");
    if (event_by_json == NULL) {
        LOG_ERROR("PARSER", "Event By JSON not found");
//        construct_mqtt_device_log_msg(E_EBY,L_REGISTRATION);
        return FAIL;
    }
    memset(casa_ctx.event_by, '\0', EVENT_BY_LEN);
    strncpy(casa_ctx.event_by, event_by_json->valuestring, strlen(event_by_json->valuestring));
    LOG_INFO("PARSER", "Event By: %s", casa_ctx.event_by);

    cJSON *locationid_json = cJSON_GetObjectItem(json_parse, "locId");       /* parsing Location ID to casa struct */
    if(locationid_json == NULL) {
        LOG_ERROR("PARSER","location id not found.\r\n");
//        construct_mqtt_device_log_msg(E_LOC_ID,L_REGISTRATION);
        return FAIL;
    }
    strncpy(casa_ctx.location, locationid_json->valuestring, strlen(locationid_json->valuestring));
    LOG_INFO("PARSER", "location id: %s\r\n", casa_ctx.location);

    //authtoken_json = local_doc["authToken"];                                   /* parsing authentication token to registration struct */
    authtoken_json = "Manual";
    if (authtoken_json == NULL) {
        LOG_ERROR("PARSER","authtoken not found.\r\n");
//        construct_mqtt_device_log_msg(E_AUTH_TOKEN,L_REGISTRATION);
        return FAIL;
    }
    strncpy(casa_ctx.registration->authtoken, authtoken_json, strlen(authtoken_json)+1);
    LOG_INFO("PARSER", "authtoken: %s\r\n", casa_ctx.registration->authtoken);

    return PARSE_SUCCESS;
}

int cloud_mqtt_reg_json_parser(cJSON *json_parse)
{
//    const char *fota_url = NULL;

    // 1. Extract 'code' and map to cloud_mqtt_reg_status
    cJSON *code_json = cJSON_GetObjectItem(json_parse, "code");
    if (code_json == NULL) {
        LOG_ERROR("PARSE", "Execution: FAILED - 'code' field missing");
        // Update structure status even on failure if necessary
        return FAIL;
    }

    // Assigning to uint16_t as per your new structure
    casa_ctx.registration->cloud_mqtt_reg_status = (uint16_t)code_json->valueint;
    LOG_INFO("PARSE", "Execution: SUCCESS - Reg Status Code: %d", casa_ctx.registration->cloud_mqtt_reg_status);

    // 2. Check for FOTA URL
    cJSON *url = cJSON_GetObjectItemCaseSensitive(json_parse, "url");

    if (!cJSON_IsString(url) || (url->valuestring == NULL)) {
        LOG_WARN("PARSE", "Execution: NOTICE - No FOTA URL in this response");
    } else {
        // Safe Memory Allocation for FOTA context
//        if (casa_ctx.fota_update == NULL) {
//            casa_ctx.fota_update = (fota_context_t*) malloc(sizeof(fota_context_t));
//        }
//
//        if (casa_ctx.fota_update == NULL) {
//            LOG_ERROR("SYS", "Execution: Malloc failed for FOTA context");
//            return FAIL;
//        }
//
//        // --- FIXED ORDER ---
//        // Initialize memory BEFORE assigning parsed values
//        memset(casa_ctx.fota_update, 0, sizeof(fota_context_t));
//
//        // 3. Extract 'source'
//        cJSON *src_json = cJSON_GetObjectItem(json_parse, "source");
//        casa_ctx.url_src = (src_json != NULL) ? src_json->valueint : 0;
//
//        // 4. Extract 'requestId'
//        // Note: Structure uses long long, cJSON uses valuedouble for large numbers
//        cJSON *req_id = cJSON_GetObjectItem(json_parse, "requestId");
//        if (req_id != NULL) {
//            casa_ctx.fota_update->requestId = (long long)req_id->valuedouble;
//        }
//
//        // 5. Secure String Copy to fota_context_t
//        fota_url = url->valuestring;
//        strncpy(casa_ctx.fota_update->fota_url, fota_url, sizeof(casa_ctx.fota_update->fota_url) - 1);
//
//        // Update the flag in your new registration structure
//        casa_ctx.registration->reg_fota_status = 1;
//
//        LOG_INFO("PARSE", "Execution: FOTA URL saved for RequestID: %lld", casa_ctx.fota_update->requestId);
    }

    return PARSE_SUCCESS;
}

int deregistration_msg_parser(cJSON *json_parse)
{
    // 1. Safe Memory Allocation for De-registration Context
    if (casa_ctx.de_registration == NULL) {
        casa_ctx.de_registration = (de_registration_context_t*) malloc(sizeof(de_registration_context_t));

        if (casa_ctx.de_registration == NULL) {
            LOG_ERROR("PARSE", "Execution: Malloc failed for de-registration context");
//            construct_device_log_msg(E_MALLOC, L_DEREGISTRATION + L_MSG_PARSE);
            return FAIL;
        }

        // Initialize and set defaults
        memset(casa_ctx.de_registration, 0, sizeof(de_registration_context_t));
        snprintf(casa_ctx.de_registration->dereg_from, sizeof(casa_ctx.de_registration->dereg_from), "mobile");

        casa_ctx.de_registration->current_op = DE_REG_DETAILS_SHARING;
        casa_ctx.switch_interrupts = DISABLE;
        LOG_INFO("PARSE", "Execution: Context allocated for De-registration");
    }

    // 2. Parse Request ID (long long cast for 64-bit safety)
    cJSON *req_id = cJSON_GetObjectItem(json_parse, "requestId");
    if (req_id != NULL) {
        casa_ctx.de_registration->requestId = (long long)req_id->valuedouble;
    }

    // 3. Parse 'eById' (Event By ID)
    cJSON *event_by_id_json = cJSON_GetObjectItem(json_parse, "eById");
    if (event_by_id_json == NULL || !cJSON_IsString(event_by_id_json)) {
        LOG_ERROR("PARSE", "Execution: FAILED - 'eById' missing or invalid");
//        construct_device_log_msg(E_EBY_ID, L_DEREGISTRATION);
        return FAIL;
    }
    memset(casa_ctx.event_by_id, 0, USRID_LEN);
    strncpy(casa_ctx.event_by_id, event_by_id_json->valuestring, USRID_LEN - 1);
    LOG_INFO("PARSE", "Event By ID: %s", casa_ctx.event_by_id);

    // 4. Parse 'eBy' (Event By)
    cJSON *event_by_json = cJSON_GetObjectItem(json_parse, "eBy");
    if (event_by_json == NULL || !cJSON_IsString(event_by_json)) {
        LOG_ERROR("PARSE", "Execution: FAILED - 'eBy' missing");
//        construct_device_log_msg(E_EBY, L_DEREGISTRATION);
        return FAIL;
    }
    memset(casa_ctx.event_by, 0, EVENT_BY_LEN);
    strncpy(casa_ctx.event_by, event_by_json->valuestring, EVENT_BY_LEN - 1);
    LOG_INFO("PARSE", "Event By: %s", casa_ctx.event_by);

    // 5. Parse 'authToken'
    cJSON *authtoken_json = cJSON_GetObjectItem(json_parse, "authToken");
    if (authtoken_json == NULL || !cJSON_IsString(authtoken_json)) {
        LOG_ERROR("PARSE", "Execution: FAILED - 'authToken' missing");
//        construct_device_log_msg(E_AUTH_TOKEN, L_DEREGISTRATION);
        return FAIL;
    }
    // Safely copy to the context structure
    memset(casa_ctx.de_registration->authtoken, 0, sizeof(casa_ctx.de_registration->authtoken));
    strncpy(casa_ctx.de_registration->authtoken, authtoken_json->valuestring, sizeof(casa_ctx.de_registration->authtoken) - 1);

    // 6. Finalize Operation State
    casa_ctx.current_operation = DEREGISTRATION;
    LOG_INFO("PARSE", "Execution: SUCCESS - De-registration parsed and state updated to DEREGISTRATION");

    return PARSE_SUCCESS;
}

int cloud_mqtt_dereg_json_parser(cJSON *json_parse)
{
    // 1. Parse 'msg' (Cloud Description)
    cJSON *msg_json = cJSON_GetObjectItem(json_parse, "msg");
    if (msg_json == NULL || !cJSON_IsString(msg_json)) {
        LOG_ERROR("PARSER", "Execution: FAILED - 'msg' field missing or invalid");
//        construct_mqtt_device_log_msg(E_MSG, L_DEREGISTRATION);
        return FAIL;
    }

    // Safety: Ensure we don't overflow the context buffer
    // We use sizeof() - 1 to leave room for the null terminator
    memset(casa_ctx.de_registration->cloud_description_msg, 0, sizeof(casa_ctx.de_registration->cloud_description_msg));
    strncpy(casa_ctx.de_registration->cloud_description_msg, msg_json->valuestring, sizeof(casa_ctx.de_registration->cloud_description_msg) - 1);

    LOG_INFO("PARSER", "Cloud Msg: %s", casa_ctx.de_registration->cloud_description_msg);

    // 2. Parse 'code' (Deregistration Status)
    cJSON *code_json = cJSON_GetObjectItem(json_parse, "code");
    if (code_json == NULL) {
        LOG_ERROR("PARSER", "Execution: FAILED - 'code' json not found");
//        construct_mqtt_device_log_msg(E_CODE, L_DEREGISTRATION);
        return FAIL;
    }

    // Assigning to uint16_t or int based on your structure
    casa_ctx.de_registration->cloud_mqtt_dereg_status = code_json->valueint;

    LOG_INFO("PARSER", "Execution: SUCCESS - Dereg Status Code: %d", casa_ctx.de_registration->cloud_mqtt_dereg_status);

    return PARSE_SUCCESS;
}



int casa_message_parser(char *casa_req, int Req_length)
{
      (void)Req_length;
      char operation_code = 0;
      char ret_val = 0;
      cJSON *root = cJSON_Parse(casa_req);
      if (root == NULL) {
          LOG_ERROR("PARSER", "Message parsing error");
          cJSON_Delete(root);
          return FAIL;
      }

      cJSON *optCode = cJSON_GetObjectItemCaseSensitive(root, "optCode");
      if (cJSON_IsNumber(optCode)) {
          operation_code = cJSON_GetObjectItem(root, "optCode")->valueint;
      } else {
          LOG_ERROR("PARSER", "Operation_code not found");
          cJSON_Delete(root);
          return FAIL;
      }

      switch (operation_code)
      {
          case UDP_BLE_REGISTRATION :
          {
              ret_val = registration_udp_ble_parser(root);
              cJSON_Delete(root);
              if(ret_val) {
                  return PARSE_SUCCESS;
              } else {
                  return FAIL;
              }
              break;
          }

          case CLOUD_REG_MQTT_RESP:
          {
              if (casa_ctx.registration  != NULL) {
                  ret_val = cloud_mqtt_reg_json_parser(root);
                  cJSON_Delete(root);
              }
              if (ret_val) {
                  return PARSE_SUCCESS;
              } else {
                  return FAIL;
              }
              break;
          }

          case DE_REGISTRATION_MSG:
          {
              ret_val = deregistration_msg_parser(root);
              cJSON_Delete(root);
              if (ret_val) {
                  return PARSE_SUCCESS;
              } else {
                  free(casa_ctx.de_registration);
                  casa_ctx.de_registration = NULL;
                  return FAIL;
              }
              break;
          }

          case CLOUD_DEREG_MQTT_RESP:
          {
              if (casa_ctx.de_registration  != NULL) {
                  ret_val = cloud_mqtt_dereg_json_parser(root);
                  cJSON_Delete(root);
              }
              if (ret_val) {
                  return PARSE_SUCCESS;
              } else {
                  return FAIL;
              }
              break;
          }

          default:
          {
              cJSON_Delete(root);
              break;
          }
       }
       return 1;
}

