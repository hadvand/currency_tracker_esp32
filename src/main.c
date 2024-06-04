/**
 * @file main.c
 * @brief Main application file for handling MQTT communication, displaying currency information on an OLED screen,
 *        and interacting with an APDS9960 gesture sensor.
 *
 * @author Murad Mikogaziev
 * @date 14/11/2023
 */
#include "main.h"

 // Pin configurations for SPI communication
 #define CONFIG_MOSI_GPIO 23
#define CONFIG_SCLK_GPIO 18
#define CONFIG_CS_GPIO 5
#define CONFIG_DC_GPIO 27
#define CONFIG_RESET_GPIO 17

// Pin configurations for I2C communication
#define CONFIG_SDA_GPIO 25
#define CONFIG_SCL_GPIO 26

// APDS9960 I2C address
#define APDS9960_ADDR 0x39 

// Tags for logging purposes
#define TAG_WIFI "WIFI"
#define TAG_MQTT "MQTT"
#define TAG_SSD1306 "SSD1306"
#define TAG_APDS9960 "APDS9960"

// Message prefixes
#define PREFIX_CURRENCY "[CURRENCY]"
#define PREFIX_DATA "[DATA]"

// MQTT broker configuration
#define CONFIG_MQTT_TOPIC "test"

// Wi-Fi credentials
#define SSID "GL65-9SC"
#define PASSWORD "gnes2600"

// Maximum buffer size for data
#define MAX_BUFF 256

// Buffers for storing sensor data
char RUB[MAX_BUFF] = { 0 };
char EUR[MAX_BUFF] = { 0 };
char CZK[MAX_BUFF] = { 0 };
char BTC[MAX_BUFF] = { 0 };
char ETH[MAX_BUFF] = { 0 };
int CURRENCY = 0;

// I2C bus and APDS9960 sensor handles
i2c_bus_handle_t i2c_bus;
apds9960_handle_t apds9960;
SSD1306_t dev;

// Cleans up resources used by the program
void cleanup() {
    apds9960_delete(&apds9960);
    i2c_bus_delete(&i2c_bus);
}

// Waits for a gesture input from the APDS9960 sensor
int8_t wait_for_gesture() {
    int8_t gesture;

    // Wait for a gesture to be read
    while (!(gesture = apds9960_read_gesture(apds9960)));

    return gesture;
}

// Handles Wi-Fi events, such as connection status and disconnection
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    }
}

// Parses and extracts data from an MQTT message.
void mqtt_parse_data(char* token) {
    for (int i = 0; token != NULL; token = strtok(NULL, ","), i++) {
        if (i == 1) strcpy(RUB, token);
        else if (i == 2) strcpy(EUR, token);
        else if (i == 3) strcpy(CZK, token);
        else if (i == 4) strcpy(BTC, token);
        else if (i == 5) strcpy(ETH, token);
    }
}

// Handles MQTT events, such as connection status, received data, and disconnection.
void mqtt_event(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data)
{
    // Extract MQTT event data
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    // Process different MQTT event types
    switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
        // Handle successful MQTT connection
        esp_mqtt_client_subscribe(client, "test", 0);
        break;
    case MQTT_EVENT_DATA:
        // Process received MQTT data
        char buffer[MAX_BUFF] = { 0 };
        strncpy(buffer, event->data, event->data_len);

        // Extract the prefix from the received message
        char* messagePrefix = strtok(buffer, " ");

        // Check if the message contains the expected prefix
        if (strstr(messagePrefix, PREFIX_DATA) != NULL) {
            mqtt_parse_data(messagePrefix);
        }
        break;
    default:
        break;
    }
}


// MQTT task responsible for handling MQTT communication.
static void mqtt_task()
{
    // Configure the MQTT client with the broker's URI
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://broker.emqx.io:1883"
    };

    // Initialize the MQTT client
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    // Register the MQTT event handler for handling MQTT events
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event, client));

    // Start the MQTT client
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));

    while (1) {
        char buff[MAX_BUFF];
        sprintf(buff, "%s %s", PREFIX_CURRENCY, MENU_CONFIG[CURRENCY]);
        printf("%s\n", buff);

        // Publish the message to the MQTT broker
        esp_mqtt_client_publish(client, "test", buff, strlen(buff), 1, 0);

        // Delay before publishing the next message
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

// Initializes the NVS flash.
void init_nvs_flash() {
    // Initialize the NVS flash
    esp_err_t ret = nvs_flash_init();

    // Check for specific NVS initialization errors
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Erase NVS flash and attempt initialization again
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    // Log any errors encountered during NVS flash initialization
    ESP_ERROR_CHECK(ret);
}

// Initializes the WiFi connection for the station (STA) mode.
void init_wifi() {
    // Initialize the network interface
    ESP_ERROR_CHECK(esp_netif_init());

    // Create the default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create the default WiFi station interface
    esp_netif_create_default_wifi_sta();

    // Initialize the WiFi driver with default configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    // Configure WiFi connection parameters
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password = PASSWORD,
        },
    };

    // Set WiFi storage mode to RAM
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // Set WiFi mode to station (STA)
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // Set WiFi configuration
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    // Start the WiFi driver
    ESP_ERROR_CHECK(esp_wifi_start());

    // Wait to connect to wifi
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}

// Displays currency information on the OLED screen.
void view_rub() {
    while (1) {
        // Update OLED screen with ruble price
        ssd1306_clear_screen(&dev, false);
        ssd1306_contrast(&dev, 0xff);
        ssd1306_display_text(&dev, 0, "     RUB     ", 16, true);
        ssd1306_display_text(&dev, 4, RUB, strlen(RUB), false);

        // Process gesture data
        switch (wait_for_gesture()) {
            case APDS9960_UP:
                ESP_LOGI(TAG_APDS9960, "Gesture: DOWN");
                break;
            case APDS9960_DOWN:
                ESP_LOGI(TAG_APDS9960, "Gesture: UP");
                break;
            case APDS9960_LEFT:
                ESP_LOGI(TAG_APDS9960, "Gesture: RIGHT");
                break;
            case APDS9960_RIGHT:
                ESP_LOGI(TAG_APDS9960, "Gesture: LEFT");
                return;
        }
    }
}

// Displays eur information on the OLED screen.
void view_eur() {
    while (1) {
        // Update OLED screen with euro price
        ssd1306_clear_screen(&dev, false);
        ssd1306_display_text(&dev, 0, "     EUR     ", 16, true);
        ssd1306_display_text(&dev, 4, EUR, strlen(EUR), false);

        // Process gesture data
        switch (wait_for_gesture()) {
        case APDS9960_UP:
            ESP_LOGI(TAG_APDS9960, "Gesture: DOWN");
            break;
        case APDS9960_DOWN:
            ESP_LOGI(TAG_APDS9960, "Gesture: UP");
            break;
        case APDS9960_LEFT:
            ESP_LOGI(TAG_APDS9960, "Gesture: RIGHT");
            break;
        case APDS9960_RIGHT:
            ESP_LOGI(TAG_APDS9960, "Gesture: LEFT");
            return;
        }
    }
}

// Displays czk information on the OLED screen.
void view_czk() {
    while (1) {
        // Update OLED screen with czech crown price
        ssd1306_clear_screen(&dev, false);
        ssd1306_display_text(&dev, 0, "     CZK     ", 16, true);
        ssd1306_display_text(&dev, 4, CZK, strlen(CZK), false);

        // Process gesture data
        switch (wait_for_gesture()) {
        case APDS9960_UP:
            ESP_LOGI(TAG_APDS9960, "Gesture: DOWN");
            break;
        case APDS9960_DOWN:
            ESP_LOGI(TAG_APDS9960, "Gesture: UP");
            break;
        case APDS9960_LEFT:
            ESP_LOGI(TAG_APDS9960, "Gesture: RIGHT");
            break;
        case APDS9960_RIGHT:
            ESP_LOGI(TAG_APDS9960, "Gesture: LEFT");
            return;
        }
    }
}

// Displays bitcoin price on the OLED screen.
void view_btc() {
    while (1) {
        // Update OLED screen with bicoin price
        ssd1306_clear_screen(&dev, false);
        ssd1306_display_text(&dev, 0, "     BTC     ", 16, true);
        ssd1306_display_text(&dev, 4, BTC, strlen(BTC), false);

        // Process gesture data
        switch (wait_for_gesture()) {
        case APDS9960_UP:
            ESP_LOGI(TAG_APDS9960, "Gesture: DOWN");
            break;
        case APDS9960_DOWN:
            ESP_LOGI(TAG_APDS9960, "Gesture: UP");
            break;
        case APDS9960_LEFT:
            ESP_LOGI(TAG_APDS9960, "Gesture: RIGHT");
            break;
        case APDS9960_RIGHT:
            ESP_LOGI(TAG_APDS9960, "Gesture: LEFT");
            return;
        }
    }
}

// Displays ethereum information on the OLED screen.
void view_eth() {
    while (1) {
        // Update OLED screen with ethereum price
        ssd1306_clear_screen(&dev, false);
        ssd1306_display_text(&dev, 0, "     ETH     ", 16, true);
        ssd1306_display_text(&dev, 4, ETH, strlen(ETH), false);

        // Process gesture data
        switch (wait_for_gesture()) {
        case APDS9960_UP:
            ESP_LOGI(TAG_APDS9960, "Gesture: DOWN");
            break;
        case APDS9960_DOWN:
            ESP_LOGI(TAG_APDS9960, "Gesture: UP");
            break;
        case APDS9960_LEFT:
            ESP_LOGI(TAG_APDS9960, "Gesture: RIGHT");
            break;
        case APDS9960_RIGHT:
            ESP_LOGI(TAG_APDS9960, "Gesture: LEFT");
            return;
        }
    }
}

// Displays a menu on the OLED screen and handles gesture-based navigation
void view_menu() {
    const view_t MENU_VIEWS[] = {
        [MENU_RUB] = view_rub,
        [MENU_EUR] = view_eur,
        [MENU_CZK] = view_czk,
        [MENU_BTC] = view_btc,
        [MENU_ETH] = view_eth
    };

    int view_i = 0;   // Index of the currently selected menu option
    view_t view = MENU_VIEWS[view_i];    // Function pointer to the selected view

    while (1) {
        char buff[MAX_BUFF];

        // Update OLED screen with menu information
        ssd1306_clear_screen(&dev, false);
        ssd1306_display_text(&dev, 0, "     Menu     ", 16, false);
        
        // Display menu options on the OLED screen
        for (int i = 0; i < MENU_SIZE; i++) {
            ssd1306_display_text(&dev, i + 1, (char*)MENU_CONFIG[i], strlen(MENU_CONFIG[i]), view_i == i);
        }

        // Delay to prevent rapid menu changes
        vTaskDelay(500 / portTICK_PERIOD_MS);

        // Process gesture data
        switch (wait_for_gesture()) {
        case APDS9960_UP:
            ESP_LOGI(TAG_APDS9960, "Gesture: DOWN");
            view_i = (view_i + 1) % MENU_SIZE;
            break;
        case APDS9960_DOWN:
            ESP_LOGI(TAG_APDS9960, "Gesture: UP");
            view_i = (view_i - 1 + MENU_SIZE) % MENU_SIZE;
            break;
        case APDS9960_LEFT:
            ESP_LOGI(TAG_APDS9960, "Gesture: RIGHT");
            CURRENCY = view_i;
            return;
        case APDS9960_RIGHT:
            ESP_LOGI(TAG_APDS9960, "Gesture: LEFT");
            return;
        }
    }
}

// Displays a welcome message on the OLED screen
void view_welcome() {
    ssd1306_clear_screen(&dev, true);

    ssd1306_display_text(&dev, 4, "Swipe to launch", 16, true);

    wait_for_gesture();

    view_menu();
}

// Initializes the application
void app_run() {
    spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO);
    ssd1306_init(&dev, 128, 64);

    // Initialize I2C bus for APDS9960
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_SDA_GPIO,
        .scl_io_num = CONFIG_SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };

    // Create I2C bus handle based on config
    i2c_bus = i2c_bus_create(I2C_NUM_1, &conf);
    
    // Create APDS9960 handle
    apds9960 = apds9960_create(i2c_bus, APDS9960_I2C_ADDRESS);

    // Initialize gesture engine
    ESP_ERROR_CHECK(apds9960_gesture_init(apds9960));
    // Enable gesture engine
    ESP_ERROR_CHECK(apds9960_enable_gesture_engine(apds9960, true));

    // Create view with welcome text
    view_welcome();
}

// Main application entry point
void app_main(void)
{
    init_nvs_flash();
    init_wifi();
    xTaskCreate(mqtt_task, "mqtt_task", 8192, NULL, 5, NULL);
    app_run();
    cleanup();
    esp_restart();
}
