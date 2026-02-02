MH_LAB_CAM | High-Definition ESP32-CAM
A robust, web-based camera system for the ESP32-CAM. This project features high-definition image capture, a smart dual-mode WiFi system (Station + Access Point), and a persistent user interface for laboratory and remote monitoring.

Features
Smart WiFi Fallback: Automatically attempts to connect to the Lab WiFi. If it fails, it starts its own WiFi Hotspot (MH_LAB_CAM_PRO).

High Definition: Captures photos at UXGA (1600x1200) resolution with low JPEG compression for maximum detail.

Sticky Interface: The Flash setting remembers your preference and remains checked or unchecked for subsequent photos.

Selectable Deletion: Browse the gallery and delete individual photos or wipe the entire SD card.

Mobile Optimized: A clean, dark-mode web interface designed specifically for standard mobile browsers.

Permanent Storage: Photos are saved to a microSD card and persist after power loss or unplugging.

Hardware Requirements
ESP32-CAM Module (AI-Thinker model recommended)

MicroSD Card (Formatted to FAT32)

5V Power Supply (Minimum 1A recommended for HD capture and Flash stability)

FTDI Programmer (For code uploading)

Setup and Installation
Library Requirements:

EloquentESP32Cam library.

WiFi and SD_MMC (Standard ESP32 core libraries).

Configuration: Open the code and edit the following variables to match your environment:

LAB_SSID: Your laboratory router name.

LAB_PASS: Your laboratory router password.

AP_PASS: The password you want for direct phone-to-camera connection.

Upload:

Select "AI Thinker ESP32-CAM" in the Arduino IDE.

Ensure "PSRAM: Enabled" is selected in the Tools menu.

Connect GPIO 0 to GND to enter flash mode, upload, then remove the jumper and reset.

Operating Instructions
Mode A: Laboratory WiFi
Power on the camera within range of the configured router.

Open your browser and navigate to the IP address shown in the Serial Monitor.

Mode B: Direct to Phone (Access Point)
If the camera cannot find the Lab WiFi after 10 seconds, it starts its own hotspot.

Connect your phone to the WiFi network: MH_LAB_CAM_PRO.

Open your browser and navigate to: http://192.168.4.1.

Interface Guide
ENABLE FLASH: Toggle the onboard LED. This setting is "sticky" and persists between captures.

TAKE HD PHOTO: Captures a 1600x1200 image. Note: High-resolution processing requires 3 to 5 seconds to write to the SD card.

DELETE (Under Image): Removes only that specific file.

DELETE ALL: Permanently clears all JPEG files from the microSD card.

Troubleshooting
Brownout Error: If the device resets during a photo, the power supply is insufficient. Ensure you are providing a steady 5V.

Blurry Images: The lens is fixed-focus. To adjust, carefully remove the factory glue and rotate the lens barrel manually to find the sharpest focus point for your subject distance.

SD Card Errors: Ensure the card is 32GB or smaller and formatted to FAT32.
