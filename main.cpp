#include <Arduino.h>
#include <WiFi.h>
#include <eloquent_esp32cam.h>
#include <eloquent_esp32cam/extra/esp32/fs/sdmmc.h>

using namespace eloq;

// --- CONFIGURATION ---
const char* LAB_SSID = "MH_LAB_CAM"; 
const char* LAB_PASS = "camera413"; 

const char* AP_SSID = "MH_LAB_CAM_PRO"; // Hotspot name
const char* AP_PASS = "12345678";       // Hotspot password

const int FLASH_PIN = 4; 
int photoCounter = 0;
WiFiServer server(80);

// ... (Functions stay the same as our previous stable version)
void initializeCamera();
void takeAndSavePhoto(bool useFlash);
void syncPhotoCounter();

void setup() {
    Serial.begin(115200);
    delay(2000);
    pinMode(FLASH_PIN, OUTPUT);
    digitalWrite(FLASH_PIN, LOW); 

    initializeCamera();
    sdmmc.begin(); 
    syncPhotoCounter(); 

    // 1. TRY LAB WIFI FIRST
    Serial.print("Connecting to Lab WiFi...");
    WiFi.begin(LAB_SSID, LAB_PASS);
    
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) { // 10 seconds timeout
        delay(500);
        Serial.print(".");
        retry++;
    }

    // 2. IF FAILED, START HOTSPOT
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nCONNECTED TO LAB WIFI!");
        Serial.print("Go to: http://");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nLab WiFi not found. Starting Hotspot...");
        WiFi.disconnect();
        WiFi.softAP(AP_SSID, AP_PASS);
        
        Serial.println("HOTSPOT ACTIVE!");
        Serial.print("Connect phone to WiFi: "); Serial.println(AP_SSID);
        Serial.print("Then go to: http://");
        Serial.println(WiFi.softAPIP()); // Usually 192.168.4.1
    }
    
    server.begin();
}

// ... (Keep the rest of your loop, capture, and delete functions from the previous code)