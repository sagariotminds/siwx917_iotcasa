/*******************************************************************************
 * @file  casa_log.c
 * @brief Professional SDK-level logging implementation
 *
 * Supports:
 * - ANSI color codes for terminal/serial
 * - Millisecond timestamps
 * - Module tags
 * - Runtime level control
 ******************************************************************************/

#include <casa_log.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "cmsis_os2.h"  /* For osKernelGetTickCount() */

/* ============================================================================
 * ANSI Color Codes (works in terminals & serial with ANSI support)
 * ============================================================================*/

#define COLOR_RESET    "\033[0m"
#define COLOR_CRITICAL "\033[1;35m"  /* Bright Magenta */
#define COLOR_ERROR    "\033[1;31m"  /* Bright Red */
#define COLOR_WARN     "\033[1;33m"  /* Bright Yellow */
#define COLOR_INFO     "\033[1;36m"  /* Bright Cyan */
#define COLOR_DEBUG    "\033[0;32m"  /* Green */
#define COLOR_TAG      "\033[1;37m"  /* Bright White */
#define COLOR_TIME     "\033[0;90m"  /* Dark Gray */

/* ============================================================================
 * State
 * ============================================================================*/

static log_level_t current_level = LOG_DEFAULT_LEVEL;
static uint32_t log_start_tick = 0;

/* ============================================================================
 * Helper: Get milliseconds since boot
 * ============================================================================*/

static uint32_t log_get_millis(void)
{
  /* Use FreeRTOS kernel tick count (already in milliseconds on most configs) */
  uint32_t ticks = osKernelGetTickCount();
  
  /* FreeRTOS tick period is typically 1ms, but for safety multiply by tick period
   * In SiWX917 default config, configTICK_RATE_HZ = 1000, so tick period = 1ms */
  return ticks;  /* Already in milliseconds */
}

/* ============================================================================
 * Public API
 * ============================================================================*/

void log_init(void)
{
  log_start_tick = log_get_millis();
}

void log_set_level(log_level_t level)
{
  current_level = level;
}

log_level_t log_get_level(void)
{
  return current_level;
}

void log_write(log_level_t level, const char *tag, const char *fmt, ...)
{
  if (level > current_level || level == LOG_LEVEL_NONE) {
    return;
  }

  /* Determine level string and color */
  const char *lvl_str = "?";
  const char *lvl_color = COLOR_RESET;

  switch (level) {
    case LOG_LEVEL_CRITICAL:
      lvl_str = "CRIT";
      lvl_color = COLOR_CRITICAL;
      break;
    case LOG_LEVEL_ERROR:
      lvl_str = "ERR ";
      lvl_color = COLOR_ERROR;
      break;
    case LOG_LEVEL_WARN:
      lvl_str = "WARN";
      lvl_color = COLOR_WARN;
      break;
    case LOG_LEVEL_INFO:
      lvl_str = "INFO";
      lvl_color = COLOR_INFO;
      break;
    case LOG_LEVEL_DEBUG:
      lvl_str = "DBG ";
      lvl_color = COLOR_DEBUG;
      break;
    default:
      lvl_str = "???";
      lvl_color = COLOR_RESET;
      break;
  }

  /* Format user message - increased buffer for large payloads like JSON */
//  char msg_buf[1024];
  char msg_buf[256];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(msg_buf, sizeof(msg_buf), fmt, ap);
  va_end(ap);

  /* Get timestamp (ms since boot) */
  uint32_t total_ms = log_get_millis();
  
  /* Convert to HH:MM:SS.MS format */
  uint32_t hours = (total_ms / 3600000) % 24;      /* Milliseconds -> hours */
  uint32_t minutes = (total_ms / 60000) % 60;      /* Milliseconds -> minutes */
  uint32_t seconds = (total_ms / 1000) % 60;       /* Milliseconds -> seconds */
  uint32_t ms_part = total_ms % 1000;              /* Remaining milliseconds */

  /* Print with color, timestamp, tag, and level
   * Format: [HH:MM:SS.MMM] TAG LEVEL: Message
   */

#if LOG_USE_TIMESTAMP && LOG_USE_COLOR
  printf("%s[%02lu:%02lu:%02lu.%03lu]%s %s%-8s%s %s%s%s: %s\r\n",
         COLOR_TIME, hours, minutes, seconds, ms_part, COLOR_RESET,
         COLOR_TAG, tag ? tag : "APP", COLOR_RESET,
         lvl_color, lvl_str, COLOR_RESET,
         msg_buf);
#elif LOG_USE_TIMESTAMP
  printf("[%02lu:%02lu:%02lu.%03lu] %-8s %s: %s\r\n",
         hours, minutes, seconds, ms_part,
         tag ? tag : "APP",
         lvl_str,
         msg_buf);
#elif LOG_USE_COLOR
  printf("%s%-8s%s %s%s%s: %s\r\n",
         COLOR_TAG, tag ? tag : "APP", COLOR_RESET,
         lvl_color, lvl_str, COLOR_RESET,
         msg_buf);
#else
  printf("%-8s %s: %s\r\n",
         tag ? tag : "APP",
         lvl_str,
         msg_buf);
#endif
}

/**
 * Log large strings/payloads in chunks (e.g., JSON responses)
 * Useful for strings larger than buffer size
 * @param level   Log level
 * @param tag     Module/subsystem tag
 * @param payload Large string to log
 * @param chunk_size Size of each chunk (default: 256 bytes)
 */
void log_write_large(log_level_t level, const char *tag, const char *payload, size_t chunk_size)
{
  if (level > current_level || level == LOG_LEVEL_NONE || payload == NULL) {
    return;
  }

  if (chunk_size == 0) {
    chunk_size = 256;  /* default chunk size */
  }

  size_t payload_len = strlen(payload);
  size_t offset = 0;
  int chunk_num = 0;

  while (offset < payload_len) {
    size_t chunk_len = (payload_len - offset < chunk_size) ? 
                       (payload_len - offset) : chunk_size;
    
    /* Determine level string and color */
    const char *lvl_str = "?";
    const char *lvl_color = COLOR_RESET;

    switch (level) {
      case LOG_LEVEL_CRITICAL:
        lvl_str = "CRIT";
        lvl_color = COLOR_CRITICAL;
        break;
      case LOG_LEVEL_ERROR:
        lvl_str = "ERR ";
        lvl_color = COLOR_ERROR;
        break;
      case LOG_LEVEL_WARN:
        lvl_str = "WARN";
        lvl_color = COLOR_WARN;
        break;
      case LOG_LEVEL_INFO:
        lvl_str = "INFO";
        lvl_color = COLOR_INFO;
        break;
      case LOG_LEVEL_DEBUG:
        lvl_str = "DBG ";
        lvl_color = COLOR_DEBUG;
        break;
      default:
        lvl_str = "???";
        lvl_color = COLOR_RESET;
        break;
    }

    /* Get timestamp */
    uint32_t total_ms = log_get_millis();
    uint32_t hours = (total_ms / 3600000) % 24;
    uint32_t minutes = (total_ms / 60000) % 60;
    uint32_t seconds = (total_ms / 1000) % 60;
    uint32_t ms_part = total_ms % 1000;

    /* Print chunk with indicator */
#if LOG_USE_TIMESTAMP && LOG_USE_COLOR
    printf("%s[%02lu:%02lu:%02lu.%03lu]%s %s%-8s%s %s%s%s [%d]: ",
           COLOR_TIME, hours, minutes, seconds, ms_part, COLOR_RESET,
           COLOR_TAG, tag ? tag : "APP", COLOR_RESET,
           lvl_color, lvl_str, COLOR_RESET,
           chunk_num++);
#elif LOG_USE_TIMESTAMP
    printf("[%02lu:%02lu:%02lu.%03lu] %-8s %s [%d]: ",
           hours, minutes, seconds, ms_part,
           tag ? tag : "APP",
           lvl_str,
           chunk_num++);
#elif LOG_USE_COLOR
    printf("%s%-8s%s %s%s%s [%d]: ",
           COLOR_TAG, tag ? tag : "APP", COLOR_RESET,
           lvl_color, lvl_str, COLOR_RESET,
           chunk_num++);
#else
    printf("%-8s %s [%d]: ",
           tag ? tag : "APP",
           lvl_str,
           chunk_num++);
#endif

    /* Print chunk (without newline, add it when complete) */
    fwrite(&payload[offset], 1, chunk_len, stdout);
    
    offset += chunk_len;
    
    /* Add newline if this is the last chunk or every chunk for readability */
    if (offset >= payload_len) {
      printf("\r\n");
    } else {
      printf("..\r\n");  /* Indicate continuation */
    }
  }
}
