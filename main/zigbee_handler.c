/**
 * @file zigbee_handler.c
 * @brief Zigbee On/Off endpoint handler implementation for ESP32-C6 Fan Switch
 * 
 * This module implements:
 *   - Zigbee stack initialization for End Device role
 *   - On/Off Light endpoint creation with standard HA clusters
 *   - Attribute change callbacks for relay control
 *   - ZDO signal handling for network events
 */

#include "zigbee_handler.h"
#include "esp_zigbee_core.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "esp_log.h"
#include "esp_check.h"
#include <string.h>

/* =============================================================================
 * Private Constants and Variables
 * ============================================================================= */

static const char *TAG = "ZIGBEE";

/** Callback function for On/Off commands from Zigbee network */
static zigbee_on_off_callback_t s_on_off_callback = NULL;

/* =============================================================================
 * Private Function Declarations
 * ============================================================================= */

static void zb_zdo_signal_handler(esp_zb_app_signal_t *signal_struct);
static esp_err_t zb_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message);
static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message);

/* =============================================================================
 * Private Function Implementations
 * ============================================================================= */

/**
 * @brief Handle Zigbee Device Object (ZDO) signals
 * 
 * This function processes network-related events such as:
 *   - Stack initialization
 *   - Network steering (joining)
 *   - Device announcements
 * 
 * @param signal_struct Signal information from Zigbee stack
 */
static void zb_zdo_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    uint32_t *p_sg_p = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = *p_sg_p;
    
    switch (sig_type) {
        case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
            ESP_LOGI(TAG, "Zigbee stack initialized");
            /* Start network steering (join network) */
            esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
            break;
            
        case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
        case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
            if (err_status == ESP_OK) {
                ESP_LOGI(TAG, "Device started up in %s mode",
                         sig_type == ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START ? 
                         "factory-reset" : "non factory-reset");
                         
                if (esp_zb_bdb_is_factory_new()) {
                    ESP_LOGI(TAG, "Start network steering (searching for coordinator)");
                    esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
                } else {
                    ESP_LOGI(TAG, "Device already commissioned, rejoining network");
                }
            } else {
                ESP_LOGW(TAG, "Device startup failed, status: %s, retrying...", 
                         esp_err_to_name(err_status));
                esp_zb_scheduler_alarm((esp_zb_callback_t)esp_zb_bdb_start_top_level_commissioning,
                                       ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
            }
            break;
            
        case ESP_ZB_BDB_SIGNAL_STEERING:
            if (err_status == ESP_OK) {
                esp_zb_ieee_addr_t extended_pan_id;
                esp_zb_get_extended_pan_id(extended_pan_id);
                ESP_LOGI(TAG, "Joined network successfully!");
                ESP_LOGI(TAG, "  Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
                         extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                         extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0]);
                ESP_LOGI(TAG, "  PAN ID: 0x%04x", esp_zb_get_pan_id());
                ESP_LOGI(TAG, "  Channel: %d", esp_zb_get_current_channel());
                ESP_LOGI(TAG, "  Short Address: 0x%04x", esp_zb_get_short_address());
            } else {
                ESP_LOGW(TAG, "Network steering failed, status: %s", esp_err_to_name(err_status));
                /* Retry steering after delay */
                esp_zb_scheduler_alarm((esp_zb_callback_t)esp_zb_bdb_start_top_level_commissioning,
                                       ESP_ZB_BDB_MODE_NETWORK_STEERING, 2000);
            }
            break;
            
        default:
            ESP_LOGD(TAG, "ZDO signal: %s (0x%x), status: %s",
                     esp_zb_zdo_signal_to_string(sig_type), sig_type,
                     esp_err_to_name(err_status));
            break;
    }
}

/**
 * @brief Handle attribute value changes from Zigbee network
 * 
 * Called when an On/Off command is received and the attribute is updated.
 * 
 * @param message Attribute change message with cluster/attribute info
 * @return ESP_OK on success
 */
static esp_err_t zb_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message)
{
    esp_err_t ret = ESP_OK;
    
    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG,
                        TAG, "Received message: error status(%d)", message->info.status);
    
    ESP_LOGI(TAG, "Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)",
             message->info.dst_endpoint, message->info.cluster,
             message->attribute.id, message->attribute.data.size);
    
    /* Handle On/Off cluster */
    if (message->info.dst_endpoint == ZIGBEE_ENDPOINT) {
        if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF) {
            if (message->attribute.id == ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID &&
                message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL) {
                
                bool on_off_value = message->attribute.data.value ? 
                                    *(bool *)message->attribute.data.value : false;
                
                ESP_LOGI(TAG, "On/Off command received: %s", on_off_value ? "ON" : "OFF");
                
                /* Invoke callback to control relay */
                if (s_on_off_callback) {
                    s_on_off_callback(on_off_value);
                }
            }
        }
    }
    
    return ret;
}

/**
 * @brief Central action handler for Zigbee core callbacks
 * 
 * Routes different callback types to appropriate handlers.
 * 
 * @param callback_id Type of callback
 * @param message Callback-specific message data
 * @return ESP_OK on success
 */
static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message)
{
    esp_err_t ret = ESP_OK;
    
    switch (callback_id) {
        case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
            ret = zb_attribute_handler((esp_zb_zcl_set_attr_value_message_t *)message);
            break;
            
        default:
            ESP_LOGW(TAG, "Receive Zigbee action(0x%x) callback", callback_id);
            break;
    }
    
    return ret;
}

/**
 * @brief Create On/Off Light endpoint with required clusters
 * 
 * Creates a standard Home Automation On/Off Light device with:
 *   - Basic cluster (manufacturer info)
 *   - Identify cluster
 *   - Groups cluster
 *   - Scenes cluster
 *   - On/Off cluster (main functionality)
 * 
 * @return Endpoint list ready for device registration
 */
static esp_zb_ep_list_t *create_on_off_light_ep(void)
{
    /* Create cluster list for this endpoint */
    esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();
    
    /* Basic cluster (mandatory) - Contains manufacturer info */
    esp_zb_basic_cluster_cfg_t basic_cfg = {
        .zcl_version = ESP_ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE,
        .power_source = 0x01,  /* Mains single phase */
    };
    esp_zb_attribute_list_t *basic_cluster = esp_zb_basic_cluster_create(&basic_cfg);
    esp_zb_basic_cluster_add_attr(basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, 
                                   (void *)MANUFACTURER_NAME);
    esp_zb_basic_cluster_add_attr(basic_cluster, ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, 
                                   (void *)MODEL_IDENTIFIER);
    esp_zb_cluster_list_add_basic_cluster(cluster_list, basic_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    
    /* Identify cluster */
    esp_zb_identify_cluster_cfg_t identify_cfg = {
        .identify_time = 0,
    };
    esp_zb_cluster_list_add_identify_cluster(cluster_list, 
                                              esp_zb_identify_cluster_create(&identify_cfg),
                                              ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    
    /* Groups cluster */
    esp_zb_groups_cluster_cfg_t groups_cfg = {
        .groups_name_support_id = 0,
    };
    esp_zb_cluster_list_add_groups_cluster(cluster_list,
                                            esp_zb_groups_cluster_create(&groups_cfg),
                                            ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    
    /* Scenes cluster */
    esp_zb_scenes_cluster_cfg_t scenes_cfg = {
        .scenes_count = 0,
        .current_scene = 0,
        .current_group = 0,
        .scene_valid = false,
        .name_support = 0,
    };
    esp_zb_cluster_list_add_scenes_cluster(cluster_list,
                                            esp_zb_scenes_cluster_create(&scenes_cfg),
                                            ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    
    /* On/Off cluster (main functionality) - Initial state: OFF */
    esp_zb_on_off_cluster_cfg_t on_off_cfg = {
        .on_off = false,  /* Initial state: OFF (failsafe) */
    };
    esp_zb_cluster_list_add_on_off_cluster(cluster_list,
                                            esp_zb_on_off_cluster_create(&on_off_cfg),
                                            ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    
    /* Create endpoint configuration */
    esp_zb_endpoint_config_t endpoint_config = {
        .endpoint = ZIGBEE_ENDPOINT,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .app_device_id = ESP_ZB_HA_ON_OFF_LIGHT_DEVICE_ID,  /* On/Off Light for best Z2M compatibility */
        .app_device_version = 0,
    };
    
    /* Create endpoint list and add this endpoint */
    esp_zb_ep_list_t *ep_list = esp_zb_ep_list_create();
    esp_zb_ep_list_add_ep(ep_list, cluster_list, endpoint_config);
    
    return ep_list;
}

/* =============================================================================
 * Public Function Implementations
 * ============================================================================= */

esp_err_t zigbee_handler_init(void)
{
    ESP_LOGI(TAG, "Initializing Zigbee stack");
    
    /* Configure Zigbee platform */
    esp_zb_platform_config_t platform_config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    ESP_ERROR_CHECK(esp_zb_platform_config(&platform_config));
    
    /* Configure network role as End Device */
    esp_zb_cfg_t zb_nwk_cfg = {
        .esp_zb_role = ESP_ZB_DEVICE_TYPE_ED,  /* End Device */
        .install_code_policy = false,
        .nwk_cfg = {
            .zed_cfg = {
                .ed_timeout = ESP_ZB_ED_AGING_TIMEOUT_64MIN,
                .keep_alive = 3000,  /* milliseconds */
            }
        }
    };
    
    /* Initialize Zigbee stack */
    esp_zb_init(&zb_nwk_cfg);
    
    /* Create and register the On/Off Light endpoint */
    esp_zb_ep_list_t *ep_list = create_on_off_light_ep();
    esp_zb_device_register(ep_list);
    
    /* Register action handler for attribute changes */
    esp_zb_core_action_handler_register(zb_action_handler);
    
    /* Register ZDO signal handler for network events */
    esp_zb_app_signal_handler_register(zb_zdo_signal_handler);
    
    /* Set primary channel mask (all channels) */
    esp_zb_set_primary_network_channel_set(ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK);
    
    ESP_LOGI(TAG, "Zigbee stack initialized successfully");
    ESP_LOGI(TAG, "  Device Type: End Device");
    ESP_LOGI(TAG, "  Endpoint: %d", ZIGBEE_ENDPOINT);
    ESP_LOGI(TAG, "  Device ID: On/Off Light (0x%04x)", ESP_ZB_HA_ON_OFF_LIGHT_DEVICE_ID);
    
    return ESP_OK;
}

void zigbee_handler_start(void)
{
    ESP_LOGI(TAG, "Starting Zigbee stack");
    
    /* Start Zigbee stack (does not return on success) */
    ESP_ERROR_CHECK(esp_zb_start(false));
    
    /* Enter main loop - this is blocking and does not return */
    esp_zb_stack_main_loop();
}

esp_err_t zigbee_handler_set_on_off_attribute(bool on)
{
    ESP_LOGI(TAG, "Setting On/Off attribute to: %s", on ? "ON" : "OFF");
    
    esp_zb_zcl_status_t status = esp_zb_zcl_set_attribute_val(
        ZIGBEE_ENDPOINT,
        ESP_ZB_ZCL_CLUSTER_ID_ON_OFF,
        ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID,
        &on,
        false  /* Don't check access */
    );
    
    if (status == ESP_ZB_ZCL_STATUS_SUCCESS) {
        return ESP_OK;
    }
    
    ESP_LOGE(TAG, "Failed to set On/Off attribute, status: 0x%x", status);
    return ESP_FAIL;
}

void zigbee_handler_register_on_off_callback(zigbee_on_off_callback_t callback)
{
    s_on_off_callback = callback;
    ESP_LOGI(TAG, "On/Off callback registered");
}
