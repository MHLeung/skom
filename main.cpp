#include <Arduino.h>
#include <WiFi.h>
#include <eloquent_esp32cam.h>
#include <eloquent_esp32cam/extra/esp32/fs/sdmmc.h>

using namespace eloq;

// --- CONFIGURATION ---
const char* LAB_SSID = "MH_LAB_CAM"; 
const char* LAB_PASS = "camera413"; 
const char* AP_SSID  = "MH_LAB_CAM_PRO"; 
const char* AP_PASS  = "12345678";       

const int FLASH_PIN = 4; 
int photoCounter = 0;
bool isHotspot = false;
bool flashSticky = false; 
WiFiServer server(80);

// --- FUNCTION DEFINITIONS ---

void initializeCamera() {
    camera.pinout.aithinker();
    camera.brownout.disable();
    camera.resolution.uxga(); // 1600x1200
    camera.quality.low();     // High detail
    camera.begin();

    sensor_t * s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s, 1);     
        s->set_gain_ctrl(s, 1);
    }
}

void syncPhotoCounter() {
    photoCounter = 0;
    while (SD_MMC.exists("/photo_" + String(photoCounter < 100 ? (photoCounter < 10 ? "00" : "0") : "") + String(photoCounter) + ".jpg")) {
        photoCounter++;
    }
}

void takeAndSavePhoto(bool useFlash) {
    if (useFlash) { 
        digitalWrite(FLASH_PIN, HIGH); 
        delay(400); 
    }
    if (camera.capture().isOk()) {
        char filename[32];
        sprintf(filename, "/photo_%03d.jpg", photoCounter++);
        sdmmc.save(camera.frame).to(filename);
    }
    if (useFlash) digitalWrite(FLASH_PIN, LOW);
}

void deleteAllPhotos() {
    File root = SD_MMC.open("/");
    File file = root.openNextFile();
    while (file) {
        String name = "/" + String(file.name());
        if (name.endsWith(".jpg")) SD_MMC.remove(name);
        file = root.openNextFile();
    }
    photoCounter = 0;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    pinMode(FLASH_PIN, OUTPUT);
    digitalWrite(FLASH_PIN, LOW); 

    initializeCamera();
    sdmmc.begin(); 
    syncPhotoCounter(); 

    WiFi.begin(LAB_SSID, LAB_PASS);
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 15) {
        delay(500);
        Serial.print(".");
        retry++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nCONNECTED TO LAB");
    } else {
        WiFi.disconnect();
        WiFi.softAP(AP_SSID, AP_PASS);
        isHotspot = true;
        Serial.println("\nHOTSPOT ACTIVE");
    }
    server.begin();
}

void loop() {
    WiFiClient client = server.available();
    if (client) {
        String request = client.readStringUntil('\r');
        
        if (request.indexOf("/capture") != -1) {
            flashSticky = (request.indexOf("flash=on") != -1);
            takeAndSavePhoto(flashSticky);
            client.println("HTTP/1.1 303 See Other\nLocation: /\n\n");
        } 
        else if (request.indexOf("/delete_one?n=") != -1) {
            int start = request.indexOf("n=") + 2;
            int end = request.indexOf(" ", start);
            String filename = request.substring(start, end);
            if (!filename.startsWith("/")) filename = "/" + filename;
            if (SD_MMC.exists(filename)) SD_MMC.remove(filename);
            client.println("HTTP/1.1 303 See Other\nLocation: /\n\n");
        }
        else if (request.indexOf("/delete_all") != -1) {
            deleteAllPhotos();
            client.println("HTTP/1.1 303 See Other\nLocation: /\n\n");
        }
        else if (request.indexOf("/img?n=") != -1) {
            int start = request.indexOf("n=") + 2;
            int end = request.indexOf(" ", start);
            String filename = request.substring(start, end);
            if (!filename.startsWith("/")) filename = "/" + filename;
            if (SD_MMC.exists(filename)) {
                File file = SD_MMC.open(filename, FILE_READ);
                client.println("HTTP/1.1 200 OK\nContent-Type: image/jpeg\nConnection: close\n");
                uint8_t buf[1024];
                while (file.available()) {
                    client.write(buf, file.read(buf, sizeof(buf)));
                }
                file.close();
            }
        }
        else {
            client.println("HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\n\n");
            client.println("<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>");
            client.println("<style>body{background:#000; color:#fff; text-align:center; font-family:sans-serif;}");
            client.println(".green-btn{display:block; width:90%; padding:20px; margin:10px auto; background:#28a745; color:#fff; font-weight:bold; font-size:18px; border-radius:10px; border:none;}");
            client.println(".del-btn{background:#800; padding:5px; display:block; text-decoration:none; color:#fff; font-size:11px; margin-top:-10px; border-radius:0 0 5px 5px;}");
            client.println(".img-container{width:45%; display:inline-block; vertical-align:top; margin:5px;}");
            client.println("img{width:100%; border:1px solid #444; background:#222; border-radius:5px 5px 0 0;}</style></head><body>");
            
            client.printf("<h3>HD CAMERA (%s)</h3>", isHotspot ? "HOTSPOT" : "LAB WIFI");
            client.println("<form action='/capture' method='get'>");
            client.println("<label style='font-size:18px;'>");
            client.printf("<input type='checkbox' name='flash' %s> ENABLE FLASH</label>", flashSticky ? "checked" : "");
            client.println("<button type='submit' class='green-btn'>TAKE HD PHOTO</button>");
            client.println("</form>");
            client.println("<a href='/delete_all' style='color:#666; font-size:11px;'>[ DELETE ALL ]</a><hr>");

            int count = 0;
            for (int i = photoCounter - 1; i >= 0 && count < 8; i--) {
                char fname[32];
                sprintf(fname, "photo_%03d.jpg", i);
                if (SD_MMC.exists("/" + String(fname))) {
                    client.println("<div class='img-container'>");
                    client.printf("<img src='/img?n=%s'>", fname);
                    client.printf("<a href='/delete_one?n=%s' class='del-btn'>DELETE</a>", fname);
                    client.println("</div>");
                    count++;
                }
            }
            client.println("</body></html>");
        }
        client.stop();
    }
}
