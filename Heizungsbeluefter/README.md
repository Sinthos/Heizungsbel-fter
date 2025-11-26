# Heizungsbelüfter (Arduino Version)

This project has been converted from ESP-IDF to an Arduino Sketch. It controls a fan using a relay via Zigbee (On/Off Cluster).

## Requirements

1.  **Arduino IDE** with **ESP32 Board Package (v3.0.0 or later)** installed.
2.  **Hardware:** ESP32-C6 (or H2) Dev Board.

## Setup Instructions

1.  Open `Heizungsbeluefter/Heizungsbeluefter.ino` in Arduino IDE.
2.  **Select Board:**
    *   `Tools` -> `Board` -> `ESP32C6 Dev Module`
3.  **Enable Zigbee:**
    *   `Tools` -> `Zigbee Mode` -> `Enabled`
4.  **Partition Scheme:**
    *   The project includes a custom `partitions.csv` in the sketch folder. Arduino IDE should detect it if you select `Tools` -> `Partition Scheme` -> `Custom` (if available) or `Default 4MB with Spiffs` might work if it has enough space, but **using the custom partition file is recommended** to ensure `zb_storage` is available.
    *   *Note:* If "Custom" is not an option, ensure you select a partition scheme that reserves space for Zigbee (often included in "Zigbee" specific schemes if available in the menu).
5.  **Compile & Upload:**
    *   Click the Upload button.

## Notes

*   The main logic is in `Heizungsbeluefter.ino`.
*   Zigbee and Relay helper files (`relay.c/h`, `zigbee_handler.c/h`) are included in the sketch folder and compiled automatically.
*   The device acts as a Zigbee End Device.
*   Upon first boot, it will attempt to join a Zigbee network. Put your coordinator (Zigbee2MQTT/ZHA) in pairing mode.
