/*******************************************************************************
 * @file  log.h
 * @brief Professional SDK-level logging with colors, timestamps, and tags
 *
 * Features:
 * - Log levels: CRITICAL, ERROR, WARN, INFO, DEBUG
 * - ANSI color codes (terminal & serial)
 * - Millisecond timestamps
 * - Module tags for context
 * - Runtime enable/disable per level
 * - Compile-time stripping support
 ******************************************************************************/

#ifndef IOTCASA_LOG_H
#define IOTCASA_LOG_H

#include <stdio.h>
#include <stdarg.h>


#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Configuration
 * ============================================================================*/

/* Enable/disable logging at compile-time */
#ifndef LOG_ENABLED
#define LOG_ENABLED 1
#endif

/* Default log level (CRITICAL=0, ERROR=1, WARN=2, INFO=3, DEBUG=4) */
#ifndef LOG_DEFAULT_LEVEL
#define LOG_DEFAULT_LEVEL 3 /* INFO */
#endif

/* Enable ANSI color codes */
#ifndef LOG_USE_COLOR
#define LOG_USE_COLOR 1
#endif

/* Enable timestamps */
#ifndef LOG_USE_TIMESTAMP
#define LOG_USE_TIMESTAMP 1
#endif

/* ============================================================================
 * Log Levels
 * ============================================================================*/

typedef enum {
  LOG_LEVEL_NONE = -1,
  LOG_LEVEL_CRITICAL = 0,
  LOG_LEVEL_ERROR = 1,
  LOG_LEVEL_WARN = 2,
  LOG_LEVEL_INFO = 3,
  LOG_LEVEL_DEBUG = 4,
} log_level_t;

/* ============================================================================
 * API
 * ============================================================================*/

/**
 * Initialize logging (optional, for future RTT/file routing)
 */
void log_init(void);

/**
 * Set runtime log level
 */
void log_set_level(log_level_t level);

/**
 * Get current log level
 */
log_level_t log_get_level(void);

/**
 * Core logging function
 * @param level   Log level
 * @param tag     Module/subsystem tag (e.g., "MQTT", "WIFI", "BLE")
 * @param fmt     Format string (printf-style)
 */
void log_write(log_level_t level, const char *tag, const char *fmt, ...);

/**
 * Log large payloads (JSON, long strings) in chunks
 * @param level      Log level
 * @param tag        Module/subsystem tag
 * @param payload    Large string to log
 * @param chunk_size Size of each chunk (0 = use default 256)
 */
void log_write_large(log_level_t level, const char *tag, const char *payload, size_t chunk_size);

/* ============================================================================
 * Convenience Macros
 * ============================================================================*/

#if LOG_ENABLED

#define LOG_CRITICAL(tag, fmt, ...) \
  do { \
    if (log_get_level() >= LOG_LEVEL_CRITICAL) \
      log_write(LOG_LEVEL_CRITICAL, tag, fmt, ##__VA_ARGS__); \
  } while(0)

#define LOG_ERROR(tag, fmt, ...) \
  do { \
    if (log_get_level() >= LOG_LEVEL_ERROR) \
      log_write(LOG_LEVEL_ERROR, tag, fmt, ##__VA_ARGS__); \
  } while(0)

#define LOG_WARN(tag, fmt, ...) \
  do { \
    if (log_get_level() >= LOG_LEVEL_WARN) \
      log_write(LOG_LEVEL_WARN, tag, fmt, ##__VA_ARGS__); \
  } while(0)

#define LOG_INFO(tag, fmt, ...) \
  do { \
    if (log_get_level() >= LOG_LEVEL_INFO) \
      log_write(LOG_LEVEL_INFO, tag, fmt, ##__VA_ARGS__); \
  } while(0)

#define LOG_DEBUG(tag, fmt, ...) \
  do { \
    if (log_get_level() >= LOG_LEVEL_DEBUG) \
      log_write(LOG_LEVEL_DEBUG, tag, fmt, ##__VA_ARGS__); \
  } while(0)

#define LOG_INFO_LARGE(tag, payload) \
  do { \
    if (log_get_level() >= LOG_LEVEL_INFO) \
      log_write_large(LOG_LEVEL_INFO, tag, payload, 0); \
  } while(0)

#define LOG_ERROR_LARGE(tag, payload) \
  do { \
    if (log_get_level() >= LOG_LEVEL_ERROR) \
      log_write_large(LOG_LEVEL_ERROR, tag, payload, 0); \
  } while(0)

#else /* LOG_ENABLED == 0 */

#define LOG_CRITICAL(tag, fmt, ...) do {} while(0)
#define LOG_ERROR(tag, fmt, ...)    do {} while(0)
#define LOG_WARN(tag, fmt, ...)     do {} while(0)
#define LOG_INFO(tag, fmt, ...)     do {} while(0)
#define LOG_DEBUG(tag, fmt, ...)    do {} while(0)
#define LOG_INFO_LARGE(tag, payload) do {} while(0)
#define LOG_ERROR_LARGE(tag, payload) do {} while(0)

#endif /* LOG_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* IOTCASA_LOG_H */
