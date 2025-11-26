/**
 * @file zigbee_handler.h
 * @brief Zigbee On/Off endpoint handler for ESP32-C6 Fan Switch
 * 
 * This module implements the Zigbee stack initialization and On/Off cluster
 * handling. The device operates as a Zigbee End Device (ZED) with a single
 * On/Off Light endpoint that controls a relay.
 * 
 * Zigbee Configuration:
 *   - Device Type: End Device (ZED)
 *   - Profile: Home Automation (HA)
 *   - Device ID: On/Off Light (for best Zigbee2MQTT compatibility)
 *   - Endpoint: 10 (configurable)
 *   - Clusters: Basic, Identify, Groups, Scenes, On/Off
 */

#ifndef ZIGBEE_HANDLER_H
#define ZIGBEE_HANDLER_H

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 * Configuration Constants
 * ============================================================================= */

/**
 * @brief Zigbee endpoint number for the On/Off switch
 * 
 * Endpoint 10 is commonly used in Espressif examples and is well-supported
 * by Zigbee2MQTT and Home Assistant.
 */
#define ZIGBEE_ENDPOINT         10

/**
 * @brief Manufacturer name reported to Zigbee network
 */
#define MANUFACTURER_NAME       "\x09""ESPRESSIF"

/**
 * @brief Model identifier reported to Zigbee network
 * 
 * This helps Zigbee2MQTT identify the device type.
 */
#define MODEL_IDENTIFIER        "\x10""ESP32C6_FAN_SWITCH"

/* =============================================================================
 * Public Functions
 * ============================================================================= */

/**
 * @brief Initialize the Zigbee stack
 * 
 * This function:
 *   - Configures the Zigbee platform (radio and host)
 *   - Sets up the device as an End Device
 *   - Creates the On/Off Light endpoint with required clusters
 *   - Registers action callbacks for handling On/Off commands
 * 
 * Must be called after NVS initialization and before esp_zb_start().
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t zigbee_handler_init(void);

/**
 * @brief Start the Zigbee stack and main loop
 * 
 * This function:
 *   - Starts the Zigbee stack
 *   - Enters the Zigbee main loop (blocking)
 * 
 * Note: This function does not return under normal operation.
 */
void zigbee_handler_start(void);

/**
 * @brief Update the On/Off attribute in the Zigbee cluster
 * 
 * Call this function when the relay state is changed locally (e.g., by a
 * physical button) to keep the Zigbee attribute in sync.
 * 
 * @param on true = ON, false = OFF
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t zigbee_handler_set_on_off_attribute(bool on);

/**
 * @brief Callback type for relay control from Zigbee
 * 
 * This callback is invoked when a Zigbee On/Off command is received.
 * 
 * @param on true = turn ON, false = turn OFF
 */
typedef void (*zigbee_on_off_callback_t)(bool on);

/**
 * @brief Register callback for Zigbee On/Off commands
 * 
 * The registered callback will be invoked whenever an On/Off command
 * is received from the Zigbee network.
 * 
 * @param callback Function to call when On/Off command is received
 */
void zigbee_handler_register_on_off_callback(zigbee_on_off_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif /* ZIGBEE_HANDLER_H */
