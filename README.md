# ESP32-C6 Zigbee Fan Switch

A minimal ESP-IDF project for controlling a 230V fan via Zigbee using an ESP32-C6 microcontroller. The device operates as a Zigbee End Device with a simple On/Off relay control, fully compatible with Zigbee2MQTT and Home Assistant.

## Features

- **Simple On/Off Control**: Single relay channel for fan control (no speed/PWM)
- **Zigbee 3.0 Compatible**: Standard Home Automation profile for maximum compatibility
- **Zigbee2MQTT Support**: Auto-detected as On/Off Light device
- **Home Assistant Integration**: Automatic MQTT Discovery support
- **Failsafe Design**: Relay defaults to OFF on boot
- **Detailed Logging**: Serial output for debugging and monitoring

## Hardware Requirements

### Components

| Component | Description |
|-----------|-------------|
| ESP32-C6 DevKit | Espressif development board with 802.15.4/Zigbee support |
| Relay Module | 1-channel relay module (3.3V or 5V logic) |
| 230V Fan | Any standard mains-powered fan |
| Power Supply | 5V USB or appropriate power for the relay module |

### Pin Configuration

| ESP32-C6 Pin | Function | Connection |
|--------------|----------|------------|
| GPIO8 | Relay Control | Relay module IN |
| 3.3V | Power | Relay module VCC (if 3.3V compatible) |
| 5V | Power | Relay module VCC (if 5V required) |
| GND | Ground | Relay module GND |

### Wiring Diagram

```
┌─────────────────┐          ┌──────────────────┐
│   ESP32-C6      │          │   Relay Module   │
│                 │          │                  │
│    GPIO8  ──────┼──────────┼─► IN             │
│    3.3V   ──────┼──────────┼─► VCC            │
│    GND    ──────┼──────────┼─► GND            │
│                 │          │                  │
│                 │          │    NO ─────┐     │
│                 │          │    COM ────┼──►  230V Fan
│                 │          │    NC      │     │
└─────────────────┘          └────────────┴─────┘
                                          │
                                         ═╧═ Mains (L)
```

**⚠️ WARNING**: Working with 230V mains voltage is dangerous! Ensure proper isolation and follow electrical safety guidelines. If unsure, consult a qualified electrician.

### Relay Logic

- **Active-High Configuration (Default)**:
  - GPIO HIGH (1) → Relay ON → Fan Running
  - GPIO LOW (0) → Relay OFF → Fan Stopped

- If your relay module is **active-low** (most common for 5V relay modules), change in `main/relay.h`:
  ```c
  #define RELAY_ACTIVE_LEVEL      0  // Change from 1 to 0
  ```

## Software Requirements

### ESP-IDF Installation

This project requires **ESP-IDF v5.1 or later** with Zigbee support.

1. **Install ESP-IDF** following the official guide:
   - [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/get-started/)

2. **Recommended Version**: ESP-IDF v5.1.2 or v5.2.x

3. **Verify Installation**:
   ```bash
   idf.py --version
   ```

### Project Dependencies

The project automatically downloads required components via the ESP-IDF Component Manager:
- `espressif/esp-zigbee-lib` (Zigbee SDK)

## Building and Flashing

### 1. Clone/Download the Project

```bash
cd /path/to/project
```

### 2. Set Up ESP-IDF Environment

```bash
# Linux/macOS
. $HOME/esp/esp-idf/export.sh

# Windows (PowerShell)
.$HOME\esp\esp-idf\export.ps1
```

### 3. Set Target to ESP32-C6

```bash
idf.py set-target esp32c6
```

### 4. Build the Project

```bash
idf.py build
```

### 5. Flash to Device

Connect your ESP32-C6 DevKit via USB, then:

```bash
# Find your serial port:
# Linux: /dev/ttyUSB0 or /dev/ttyACM0
# macOS: /dev/cu.usbserial-*
# Windows: COM3 (or similar)

# Erase flash (recommended for first flash)
idf.py -p /dev/ttyUSB0 erase-flash

# Flash firmware
idf.py -p /dev/ttyUSB0 flash
```

### 6. Monitor Serial Output

```bash
idf.py -p /dev/ttyUSB0 monitor
```

Press `Ctrl+]` to exit the monitor.

### Combined Flash and Monitor

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

## Zigbee2MQTT Integration

### 1. Enable Pairing Mode

In your Zigbee2MQTT instance, enable pairing mode:

**Via Web UI**: Click "Permit join (All)" button

**Via MQTT**:
```bash
mosquitto_pub -h localhost -t 'zigbee2mqtt/bridge/request/permit_join' -m '{"value": true}'
```

**Via configuration.yaml**:
```yaml
permit_join: true
```

### 2. Power On the ESP32-C6

When powered on for the first time (or after factory reset), the device will:
1. Initialize the Zigbee stack
2. Start network steering (searching for coordinator)
3. Join the network automatically

### 3. Device Discovery

The device will appear in Zigbee2MQTT with:
- **Device Type**: On/Off Light
- **Manufacturer**: ESPRESSIF
- **Model**: ESP32C6_FAN_SWITCH
- **Endpoint**: 10

### 4. Rename the Device (Optional)

In Zigbee2MQTT, rename the device to something meaningful:
```yaml
# In Zigbee2MQTT devices.yaml or via the web UI
friendly_name: 'heater_fan'
```

### Expected Zigbee2MQTT Device Info

```yaml
ieee_address: '0x1234567890abcdef'
friendly_name: 'esp32c6_fan_switch'
type: 'EndDevice'
definition:
  model: 'ESP32C6_FAN_SWITCH'
  vendor: 'ESPRESSIF'
  description: 'On/Off Light'
  supports_ota: false
  exposes:
    - type: switch
      features:
        - property: 'state'
          type: 'binary'
          value_on: 'ON'
          value_off: 'OFF'
```

## Home Assistant Integration

### Automatic Discovery

If you have Zigbee2MQTT's Home Assistant integration enabled, the device will be automatically discovered.

The device appears as either:
- **Light entity**: `light.esp32c6_fan_switch`
- **Switch entity**: `switch.esp32c6_fan_switch`

### Manual Configuration (Optional)

If auto-discovery doesn't work, add manually in `configuration.yaml`:

```yaml
# As a switch
switch:
  - platform: mqtt
    name: "Heater Fan"
    state_topic: "zigbee2mqtt/esp32c6_fan_switch"
    command_topic: "zigbee2mqtt/esp32c6_fan_switch/set"
    value_template: "{{ value_json.state }}"
    payload_on: '{"state": "ON"}'
    payload_off: '{"state": "OFF"}'
    state_on: "ON"
    state_off: "OFF"

# As a fan entity
fan:
  - platform: mqtt
    name: "Heater Fan"
    state_topic: "zigbee2mqtt/esp32c6_fan_switch"
    command_topic: "zigbee2mqtt/esp32c6_fan_switch/set"
    state_value_template: "{{ value_json.state }}"
    payload_on: '{"state": "ON"}'
    payload_off: '{"state": "OFF"}'
```

### Example Automation

```yaml
automation:
  - alias: "Turn on heater fan when temperature is high"
    trigger:
      - platform: numeric_state
        entity_id: sensor.living_room_temperature
        above: 25
    action:
      - service: switch.turn_on
        target:
          entity_id: switch.esp32c6_fan_switch

  - alias: "Turn off heater fan when temperature is normal"
    trigger:
      - platform: numeric_state
        entity_id: sensor.living_room_temperature
        below: 22
    action:
      - service: switch.turn_off
        target:
          entity_id: switch.esp32c6_fan_switch
```

## Testing and Debugging

### Serial Monitor Output

When the device starts, you should see:

```
I (xxx) MAIN: ========================================
I (xxx) MAIN: ESP32-C6 Zigbee Fan Switch Starting...
I (xxx) MAIN: ========================================
I (xxx) MAIN: Initializing NVS...
I (xxx) MAIN: NVS initialized successfully
I (xxx) MAIN: Initializing relay...
I (xxx) RELAY: Initializing relay on GPIO8
I (xxx) RELAY: Relay initialized - initial state: OFF (failsafe)
I (xxx) MAIN: Relay initialized - GPIO8, initial state: OFF
I (xxx) MAIN: Initializing Zigbee...
I (xxx) ZIGBEE: Initializing Zigbee stack
I (xxx) ZIGBEE: Zigbee stack initialized successfully
I (xxx) ZIGBEE:   Device Type: End Device
I (xxx) ZIGBEE:   Endpoint: 10
I (xxx) ZIGBEE:   Device ID: On/Off Light (0x0100)
I (xxx) MAIN: ----------------------------------------
I (xxx) MAIN: Initialization complete!
I (xxx) MAIN: Hardware Configuration:
I (xxx) MAIN:   - Relay GPIO: 8
I (xxx) MAIN:   - Active Level: HIGH
I (xxx) MAIN: Zigbee Configuration:
I (xxx) MAIN:   - Endpoint: 10
I (xxx) MAIN:   - Device Type: End Device
I (xxx) MAIN: ----------------------------------------
I (xxx) MAIN: Starting Zigbee network steering...
I (xxx) MAIN: Put your Zigbee coordinator in pairing mode!
I (xxx) MAIN: ----------------------------------------
I (xxx) ZIGBEE: Starting Zigbee stack
I (xxx) ZIGBEE: Device started up in factory-reset mode
I (xxx) ZIGBEE: Start network steering (searching for coordinator)
```

After successful join:

```
I (xxx) ZIGBEE: Joined network successfully!
I (xxx) ZIGBEE:   Extended PAN ID: xx:xx:xx:xx:xx:xx:xx:xx
I (xxx) ZIGBEE:   PAN ID: 0xxxx
I (xxx) ZIGBEE:   Channel: xx
I (xxx) ZIGBEE:   Short Address: 0xxxx
```

When receiving On/Off commands:

```
I (xxx) ZIGBEE: Received message: endpoint(10), cluster(0x6), attribute(0x0), data size(1)
I (xxx) ZIGBEE: On/Off command received: ON
I (xxx) MAIN: Zigbee command received: ON
I (xxx) RELAY: Relay set to ON (GPIO8 = 1)
```

### Manual Testing via Zigbee2MQTT

**Via MQTT command line**:
```bash
# Turn ON
mosquitto_pub -h localhost -t 'zigbee2mqtt/esp32c6_fan_switch/set' -m '{"state": "ON"}'

# Turn OFF
mosquitto_pub -h localhost -t 'zigbee2mqtt/esp32c6_fan_switch/set' -m '{"state": "OFF"}'

# Toggle
mosquitto_pub -h localhost -t 'zigbee2mqtt/esp32c6_fan_switch/set' -m '{"state": "TOGGLE"}'
```

**Via Zigbee2MQTT Web UI**:
1. Open Zigbee2MQTT dashboard
2. Find your device
3. Click on the toggle switch

### Factory Reset

To factory reset the device and remove network credentials:

1. Erase flash:
   ```bash
   idf.py -p /dev/ttyUSB0 erase-flash
   ```

2. Re-flash firmware:
   ```bash
   idf.py -p /dev/ttyUSB0 flash
   ```

The device will start fresh and search for a new coordinator.

## Project Structure

```
esp32c6_fan_switch/
├── CMakeLists.txt           # Project CMake configuration
├── sdkconfig.defaults       # Default SDK configuration for ESP32-C6 + Zigbee
├── partitions.csv           # Custom partition table
├── README.md                # This documentation
└── main/
    ├── CMakeLists.txt       # Main component CMake configuration
    ├── idf_component.yml    # Component dependencies (esp-zigbee-lib)
    ├── app_main.c           # Application entry point
    ├── relay.h              # Relay control header
    ├── relay.c              # Relay control implementation
    ├── zigbee_handler.h     # Zigbee handler header
    └── zigbee_handler.c     # Zigbee handler implementation
```

## Configuration Options

### Changing the Relay GPIO Pin

Edit `main/relay.h`:
```c
#define RELAY_GPIO_PIN          8  // Change to desired GPIO
```

### Changing the Zigbee Endpoint

Edit `main/zigbee_handler.h`:
```c
#define ZIGBEE_ENDPOINT         10  // Change if needed
```

### Changing Device Identification

Edit `main/zigbee_handler.h`:
```c
#define MANUFACTURER_NAME       "\x09""ESPRESSIF"
#define MODEL_IDENTIFIER        "\x10""ESP32C6_FAN_SWITCH"
```

## Troubleshooting

### Device Won't Join Network

1. **Check coordinator**: Ensure Zigbee2MQTT/coordinator is running
2. **Enable pairing**: Make sure permit_join is enabled
3. **Erase flash**: Try `idf.py erase-flash` and reflash
4. **Check distance**: Move closer to coordinator during pairing
5. **Check channel**: Ensure coordinator is on a valid channel

### Relay Not Switching

1. **Check wiring**: Verify GPIO8 is connected to relay IN
2. **Check power**: Ensure relay module has proper power supply
3. **Check active level**: Your relay might be active-low (change `RELAY_ACTIVE_LEVEL` to 0)
4. **Monitor serial**: Check for error messages in serial output

### Device Disappears from Network

1. **Check power**: Ensure stable power supply
2. **Increase keep-alive**: Modify `keep_alive` in `zigbee_handler.c`
3. **Check distance**: ESP32-C6 might be too far from coordinator/router

## Quick Start Summary

1. **Install ESP-IDF v5.1+**
2. **Clone project** to your workspace
3. **Set target**: `idf.py set-target esp32c6`
4. **Build**: `idf.py build`
5. **Flash**: `idf.py -p PORT erase-flash && idf.py -p PORT flash`
6. **Enable pairing** in Zigbee2MQTT
7. **Monitor**: `idf.py -p PORT monitor`
8. **Control** via Zigbee2MQTT or Home Assistant

## License

This project is provided as-is for educational and personal use.

## Acknowledgments

- Espressif Systems for ESP-IDF and ESP-Zigbee-SDK
- Zigbee2MQTT project
- Home Assistant community
