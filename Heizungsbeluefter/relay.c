/**
 * @file relay.c
 * @brief Relay control module implementation for ESP32-C6 Zigbee Fan Switch
 * 
 * This module implements GPIO-based relay control for switching a fan ON/OFF.
 */

#include "relay.h"
#include "driver/gpio.h"
#include "esp_log.h"

/* =============================================================================
 * Private Constants and Variables
 * ============================================================================= */

static const char *TAG = "RELAY";

/** Current relay state (true = ON, false = OFF) */
static bool s_relay_state = false;

/* =============================================================================
 * Public Function Implementations
 * ============================================================================= */

esp_err_t relay_init(void)
{
    ESP_LOGI(TAG, "Initializing relay on GPIO%d", RELAY_GPIO_PIN);
    
    /* Configure GPIO as output */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RELAY_GPIO_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO%d: %s", RELAY_GPIO_PIN, esp_err_to_name(ret));
        return ret;
    }
    
    /* Set initial state to OFF (failsafe - fan should not start unexpectedly) */
    s_relay_state = false;
    
    /* Apply the initial state to GPIO */
    uint32_t level = RELAY_ACTIVE_LEVEL ? 0 : 1;  /* OFF state */
    gpio_set_level(RELAY_GPIO_PIN, level);
    
    ESP_LOGI(TAG, "Relay initialized - initial state: OFF (failsafe)");
    
    return ESP_OK;
}

void relay_set(bool on)
{
    /* Calculate the GPIO level based on active level configuration */
    uint32_t level;
    
    if (RELAY_ACTIVE_LEVEL) {
        /* Active-high: HIGH = ON, LOW = OFF */
        level = on ? 1 : 0;
    } else {
        /* Active-low: LOW = ON, HIGH = OFF */
        level = on ? 0 : 1;
    }
    
    /* Apply to GPIO */
    gpio_set_level(RELAY_GPIO_PIN, level);
    
    /* Update state tracking */
    s_relay_state = on;
    
    ESP_LOGI(TAG, "Relay set to %s (GPIO%d = %lu)", 
             on ? "ON" : "OFF", RELAY_GPIO_PIN, level);
}

void relay_toggle(void)
{
    /* Invert current state */
    relay_set(!s_relay_state);
    
    ESP_LOGI(TAG, "Relay toggled to %s", s_relay_state ? "ON" : "OFF");
}

bool relay_get_state(void)
{
    return s_relay_state;
}
