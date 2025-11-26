/**
 * @file relay.h
 * @brief Relay control module for ESP32-C6 Zigbee Fan Switch
 * 
 * This module provides simple functions to control a relay connected to a GPIO pin.
 * The relay is used to switch a 230V fan ON/OFF.
 * 
 * Hardware Configuration:
 *   - GPIO8 is used as the default relay control pin
 *   - HIGH level (1) = Relay ON = Fan running
 *   - LOW level (0) = Relay OFF = Fan stopped
 * 
 * Note: Most relay modules are active-low (LOW = relay energized), but we assume
 *       active-high logic here. If your relay module is active-low, change
 *       RELAY_ACTIVE_LEVEL to 0.
 */

#ifndef RELAY_H
#define RELAY_H

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 * Configuration Constants
 * ============================================================================= */

/**
 * @brief GPIO pin number for relay control
 * 
 * GPIO8 is chosen because:
 *   - It's a general-purpose GPIO on ESP32-C6
 *   - No special boot-time functions
 *   - Directly accessible on most DevKit boards
 */
#define RELAY_GPIO_PIN          8

/**
 * @brief Active level for relay
 * 
 * Set to 1 for active-high relay modules (HIGH = relay energized)
 * Set to 0 for active-low relay modules (LOW = relay energized)
 */
#define RELAY_ACTIVE_LEVEL      1

/* =============================================================================
 * Public Functions
 * ============================================================================= */

/**
 * @brief Initialize the relay GPIO
 * 
 * Configures the relay GPIO as output and sets initial state to OFF (failsafe).
 * Must be called once at startup before using other relay functions.
 * 
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t relay_init(void);

/**
 * @brief Set the relay state
 * 
 * @param on true = Relay ON (fan running), false = Relay OFF (fan stopped)
 */
void relay_set(bool on);

/**
 * @brief Toggle the relay state
 * 
 * Switches the relay from ON to OFF or from OFF to ON.
 */
void relay_toggle(void);

/**
 * @brief Get the current relay state
 * 
 * @return true if relay is ON (fan running), false if OFF (fan stopped)
 */
bool relay_get_state(void);

#ifdef __cplusplus
}
#endif

#endif /* RELAY_H */
