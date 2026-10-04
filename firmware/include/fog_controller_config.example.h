#pragma once

// Copy this file to fog_controller_config.h and change the values.
// Do not commit real Wi-Fi or OTA credentials to a public repository.

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define DEVICE_HOSTNAME "fogbox"
#define SETUP_AP_PASSWORD "change-this-ap-password"
#define OTA_PASSWORD "change-this-ota-password"

// Sensor role indexes after you identify and label each DS18B20.
// Version 1 uses discovery order; replace with explicit addresses later.
#define COOLER_SENSOR_INDEX 0
#define OUTLET_SENSOR_INDEX 1
