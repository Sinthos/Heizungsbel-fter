# Schritt-für-Schritt Anleitung: Heizungsbelüfter mit Arduino IDE flashen

Diese Anleitung beschreibt, wie Sie das Projekt mit der Arduino IDE auf Ihren ESP32-C6 flashen.

## 1. Voraussetzungen

Stellen Sie sicher, dass Sie die folgenden Dinge installiert haben:

1.  **Arduino IDE** (Version 2.0 oder neuer empfohlen).
2.  **ESP32 Board-Unterstützung** (Version **3.0.0** oder neuer ist zwingend erforderlich für ESP32-C6 und Zigbee).

### Installation der ESP32 Board-Unterstützung (falls noch nicht geschehen):
1.  Öffnen Sie in der Arduino IDE: `Datei` -> `Einstellungen`.
2.  Fügen Sie bei "Zusätzliche Boardverwalter-URLs" folgende URL hinzu:
    `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3.  Gehen Sie zu `Werkzeuge` -> `Board` -> `Boardverwalter...`.
4.  Suchen Sie nach "esp32" (von Espressif Systems).
5.  Klicken Sie auf "Installieren" (Achten Sie darauf, dass Version **3.0.0** oder höher installiert wird).

## 2. Projekt öffnen

1.  Starten Sie die Arduino IDE.
2.  Gehen Sie auf `Datei` -> `Öffnen...`.
3.  Navigieren Sie in den Ordner `Heizungsbeluefter` dieses Projekts.
4.  Wählen Sie die Datei `Heizungsbeluefter.ino` aus und öffnen Sie sie.

## 3. Board-Einstellungen vornehmen

Gehen Sie im Menü auf `Werkzeuge` und stellen Sie folgendes ein:

*   **Board:** Wählen Sie `ESP32C6 Dev Module` (unter `esp32`).
*   **USB CDC On Boot:** `Enabled` (Damit Sie Ausgaben im Seriellen Monitor sehen).
*   **Zigbee Mode:** `Enabled` (WICHTIG! Sonst kompiliert der Code nicht).
*   **Partition Scheme:**
    *   Wählen Sie idealerweise **Custom** (falls verfügbar), damit die im Projektordner liegende `partitions.csv` verwendet wird.
    *   *Falls "Custom" nicht verfügbar ist:* Wählen Sie ein Schema mit **Zigbee** im Namen (z.B. "Zigbee 4MB with Spiffs") oder ein Schema, das mindestens 1MB für die App reserviert.
*   **Port:** Wählen Sie den COM-Port (Windows) oder Serial-Port (macOS/Linux), an dem Ihr ESP32-C6 angeschlossen ist.

## 4. Kompilieren und Hochladen

1.  Verbinden Sie den ESP32-C6 per USB-Kabel mit dem Computer.
    *   *Tipp:* Bei manchen Boards müssen Sie zum Flashen die "BOOT"-Taste gedrückt halten und kurz "RST" drücken, um in den Download-Modus zu gelangen.
2.  Klicken Sie oben links auf den **Pfeil-Button (Hochladen)**.
3.  Die IDE wird das Projekt nun kompilieren und auf den ESP32 übertragen. Dies kann beim ersten Mal einige Minuten dauern.

## 5. Testen & Pairing

1.  Nach dem erfolgreichen Hochladen öffnet sich meist nicht automatisch der Serielle Monitor.
2.  Klicken Sie oben rechts auf das **Lupen-Symbol (Serieller Monitor)**.
3.  Stellen Sie die Baudrate unten rechts auf **115200 Baud**.
4.  Drücken Sie ggf. die "RST"-Taste am ESP32.
5.  Sie sollten nun Ausgaben sehen wie:
    ```
    ESP32-C6 Zigbee Fan Switch Starting...
    Initializing Zigbee...
    ...
    Starting Zigbee network steering...
    ```
6.  Versetzen Sie nun Ihren Zigbee-Koordinator (z.B. Zigbee2MQTT oder Home Assistant ZHA) in den **Anlern-Modus (Pairing Mode)**.
7.  Der ESP32 sollte dem Netzwerk automatisch beitreten.

## Fehlerbehebung

*   **Fehler "esp_zigbee_core.h: No such file or directory":**
    *   Überprüfen Sie, ob unter `Werkzeuge` -> `Zigbee Mode` wirklich auf **Enabled** steht.
    *   Stellen Sie sicher, dass Sie die ESP32 Board-Version 3.0.0 oder neuer installiert haben.
*   **Endlose "Reboot"-Schleife oder Abstürze:**
    *   Dies liegt oft an einer falschen Partitionstabelle. Stellen Sie sicher, dass Sie **Custom** oder ein **Zigbee-kompatibles Partitionsschema** gewählt haben. Falls nötig, wählen Sie im Menü "Erase All Flash Before Sketch Upload" -> "Enabled", um alte, inkompatible Daten zu löschen.
