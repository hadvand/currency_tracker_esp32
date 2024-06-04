#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#include "driver/i2c.h"

#include "ssd1306.h"
#include "font8x8_basic.h"

#include "apds9960.h"
#include "mqtt_client.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_now.h"

typedef void (*view_t)();

typedef enum {
    MENU_RUB = 0,
    MENU_EUR = 1,
    MENU_CZK = 2,
    MENU_BTC = 3,
    MENU_ETH = 4
} view_menu_t;

const unsigned int MENU_SIZE = 5;

const char* MENU_CONFIG[] = {
    [MENU_RUB] = "RUB",
    [MENU_EUR] = "EUR",
    [MENU_CZK] = "CZK",
    [MENU_BTC] = "BTC",
    [MENU_ETH] = "ETH"
};

#endif
