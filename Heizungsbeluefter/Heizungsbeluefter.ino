/**
 * @file Heizungsbeluefter.ino
 * @brief Arduino Sketch for ESP32-C6 Zigbee Fan Switch
 * 
 * This sketch implements a Zigbee-controlled fan switch using the ESP32-C6.
 * It uses the ESP-IDF Zigbee stack through the Arduino ESP32 core.
 * 
 * Hardware:
 *   - ESP32-C6 Dev Module
 *   - Relay Module connected to GPIO8
 * 
 * Setup Instructions:
 *   1. Select Board: "ESP32C6 Dev Module"
 *   2. Zigbee Mode: "Enabled" (Tools -> Zigbee Mode)
 *   3. Partition Scheme: "Custom" (Uses partitions.csv in sketch folder)
 *      OR select a scheme with at least 1MB App + NVS + Zigbee partitions.
 *      Ideally, use the provided partitions.csv.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_check.h"

#include "relay.h"
#include "zigbee_handler.h"

/* =============================================================================
 * Private Constants
 * ============================================================================= */

static const char *TAG = "MAIN";

/* =============================================================================
 * Private Function Declarations
 * ============================================================================= */

static void on_zigbee_on_off_command(bool on);

/* =============================================================================
 * Private Function Implementations
 * ============================================================================= */

/**
 * @brief Callback for Zigbee On/Off commands
 * 
 * This function is called when an On/Off command is received from the
 * Zigbee network. It controls the relay to switch the fan.
 * 
 * @param on true = turn fan ON, false = turn fan OFF
 */
static void on_zigbee_on_off_command(bool on)
{
    ESP_LOGI(TAG, "Zigbee command received: %s", on ? "ON" : "OFF");
    
    /* Set the relay state */
    relay_set(on);
}

/* =============================================================================
 * Arduino Setup & Loop
 * ============================================================================= */

void setup()
{
    // Serial init for logging
    Serial.begin(115200);
    // Wait a bit for serial to stabilize
    delay(1000);

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "ESP32-C6 Zigbee Fan Switch Starting...");
    ESP_LOGI(TAG, "========================================");
    
    /* -------------------------------------------------------------------------
     * Step 1: Initialize NVS (Non-Volatile Storage)
     * ------------------------------------------------------------------------- */
    ESP_LOGI(TAG, "Initializing NVS...");
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition needs to be erased");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "NVS initialized successfully");
    
    /* -------------------------------------------------------------------------
     * Step 2: Initialize Relay GPIO
     * ------------------------------------------------------------------------- */
    ESP_LOGI(TAG, "Initializing relay...");
    
    ret = relay_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize relay: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "Relay initialized - GPIO%d, initial state: OFF", RELAY_GPIO_PIN);
    
    /* -------------------------------------------------------------------------
     * Step 3: Initialize Zigbee Stack
     * ------------------------------------------------------------------------- */
    ESP_LOGI(TAG, "Initializing Zigbee...");
    
    ret = zigbee_handler_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Zigbee: %s", esp_err_to_name(ret));
        return;
    }
    
    /* -------------------------------------------------------------------------
     * Step 4: Register Relay Control Callback
     * ------------------------------------------------------------------------- */
    zigbee_handler_register_on_off_callback(on_zigbee_on_off_command);
    
    ESP_LOGI(TAG, "----------------------------------------");
    ESP_LOGI(TAG, "Initialization complete!");
    ESP_LOGI(TAG, "Hardware Configuration:");
    ESP_LOGI(TAG, "  - Relay GPIO: %d", RELAY_GPIO_PIN);
    ESP_LOGI(TAG, "  - Active Level: %s", RELAY_ACTIVE_LEVEL ? "HIGH" : "LOW");
    ESP_LOGI(TAG, "Zigbee Configuration:");
    ESP_LOGI(TAG, "  - Endpoint: %d", ZIGBEE_ENDPOINT);
    ESP_LOGI(TAG, "  - Device Type: End Device");
    ESP_LOGI(TAG, "----------------------------------------");
    ESP_LOGI(TAG, "Starting Zigbee network steering...");
    ESP_LOGI(TAG, "Put your Zigbee coordinator in pairing mode!");
    ESP_LOGI(TAG, "----------------------------------------");
    
    /* -------------------------------------------------------------------------
     * Step 5: Start Zigbee Stack
     * 
     * This calls esp_zb_start() and esp_zb_stack_main_loop().
     * Since esp_zb_stack_main_loop() is blocking, setup() will NEVER return.
     * loop() will never be called. This is expected for this implementation.
     * ------------------------------------------------------------------------- */
    zigbee_handler_start();
}

void loop()
{
    // This code is unreachable because zigbee_handler_start() blocks.
}
