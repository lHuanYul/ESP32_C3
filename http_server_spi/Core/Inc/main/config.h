#pragma once

#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>

#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define WIFI_SSID "HY-TPL-BF94"
#define WIFI_PSWD "23603356"

#define ESP32_DEVICE
#define ESP32_C3_SUPER_MINI
#ifndef float32_t
#define float32_t float
#endif

#define HY_MOD_ESP32_WIFI
#define HY_MOD_ESP32_HTTP
#define HY_MOD_ESP32_SPI
#define HY_MOD_ESP32_JSON
#define JSON_PKT_LEN            1024
#define JSON_PKT_POOL_CAP       10
#define JSON_TRSM_BUF_CAP       4
#define JSON_RECV_BUF_CAP       4
