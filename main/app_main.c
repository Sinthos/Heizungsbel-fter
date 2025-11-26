/**
 * @file app_main.c
 * @brief Main application for ESP32-C6 Zigbee Fan Switch
 * 
 * This is the entry point for the Zigbee-controlled fan switch.
 * It initializes all subsystems and starts the Zigbee stack.
 * 
 * Application Flow:
 *   1. Initialize NVS (Non-Volatile Storage)
 *   2. Initialize relay GPIO (default: OFF)
 *   3. Initialize Zigbee stack
 *   4. Register relay control callback
 *   5. Start Zigbee stack (enters main loop)
 * 
 * The device operates as a Zigbee End Device with an On/Off Light
 * endpoint that controls a relay connected to a 230V fan.
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
 * Application Entry Point
 * ============================================================================= */

/**
 * @brief Main application entry point
 * 
 * Initializes all subsystems and starts the Zigbee stack.
 * This function does not return under normal operation.
 */
void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "ESP32-C6 Zigbee Fan Switch Starting...");
    ESP_LOGI(TAG, "========================================");
    
    /* -------------------------------------------------------------------------
     * Step 1: Initialize NVS (Non-Volatile Storage)
     * 
     * NVS is required for storing Zigbee network credentials and settings.
     * If the NVS partition is corrupted or uses an old format, we erase
     * and reinitialize it.
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
     * 
     * The relay GPIO is configured as output with initial state OFF.
     * This is a failsafe to prevent the fan from starting unexpectedly.
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
     * 
     * Configures the ESP32-C6 as a Zigbee End Device with an On/Off Light
     * endpoint. The device will automatically try to join a Zigbee network.
     * ------------------------------------------------------------------------- */
    ESP_LOGI(TAG, "Initializing Zigbee...");
    
    ret = zigbee_handler_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Zigbee: %s", esp_err_to_name(ret));
        return;
    }
    
    /* -------------------------------------------------------------------------
     * Step 4: Register Relay Control Callback
     * 
     * Links the Zigbee On/Off commands to the relay control function.
     * When a Zigbee command is received, the relay will be switched.
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
     * This starts the Zigbee stack and enters the main loop.
     * The function does not return under normal operation.
     * The device will automatically:
     *   - Search for a Zigbee coordinator
     *   - Join the network
     *   - Handle On/Off commands
     * ------------------------------------------------------------------------- */
    zigbee_handler_start();
    
    /* We should never reach here under normal operation */
    ESP_LOGE(TAG, "Zigbee main loop exited unexpectedly!");
}
