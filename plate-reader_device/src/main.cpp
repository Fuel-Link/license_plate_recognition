#include <Arduino.h>
#include <CommsHandler.h>
#include <cameraHandler.h>
#include <cardHandler.h>

/*
    ##########################################################################
    ############                  Definitions                     ############
    ##########################################################################
*/
#ifndef CAPTURE_RATE
#define CAPTURE_RATE 10000   // in ms
#endif

#ifdef MQTT_MAX_PACKET_SIZE 
#define MAX_PHOTO_SIZE MQTT_MAX_PACKET_SIZE
#else
#define MAX_PHOTO_SIZE 20000    // in bytes
#endif

#ifndef IMAGE_SDCARD_PATH
#define IMAGE_SDCARD_PATH "/images"
#endif

#ifndef IMAGE_URL_PATH
#define IMAGE_URL_PATH "/images"
#endif

/*
    ##########################################################################
    ############                 Global Variables                 ############
    ##########################################################################
*/
CommsHandler comms;

int photoCount = 0;

/*
    ##########################################################################
    ############                     Functions                    ############
    ##########################################################################
*/
void setup();
void loop();

/*
    ##########################################################################
    ############                   Gated Code                     ############
    ##########################################################################
*/
void CommsHandler::mqtt_message_callback(char* topic, byte* payload, unsigned int length){
    Serial.print("Message arrived in topic [");
    Serial.print(topic);
    Serial.println("]:");
    Serial.println(" - Size: " + String(length));
    Serial.print(" - Message: ");
    for(int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Allocate the JSON document
    JsonDocument doc;

    // Parse JSON object
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
        Serial.print(F("Error: deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    // Extract values
    //Serial.println(F("Response:"));
    //Serial.println(doc["sensor"].as<const char*>());
}

void CommsHandler::process_api_request(Request &request, Response &response){
    Serial.println("How you doing?");
    request.println();
    /*uint32_t fileCount = readNumber(0);
    uint32_t tableIndex = lookupTableIndex(req.path(), fileCount);
    uint32_t infoIndex = 4 + (fileCount * 4) + (tableIndex * 12);
    uint32_t calculatedHash = calculateHash(req.path());
    uint32_t storedHash = readNumber(infoIndex);

    if (calculatedHash != storedHash) {
        return;
    }

    uint32_t offset = readNumber(infoIndex + 4);
    uint32_t length = readNumber(infoIndex + 8);

    dataFile.seek(offset);

    res.set("Connection", "close");
    res.beginHeaders();
    while (length > 0) {
        int toRead = length > READ_BUFFER_SIZE ? READ_BUFFER_SIZE : length;
        dataFile.read(readBuffer, toRead);
        res.write(readBuffer, toRead);
        length = length - toRead;
    }
    res.end();*/
}


/*
    ##########################################################################
    ############                  Shared Code                     ############
    ##########################################################################
*/

void despatcher_api_fN(void* pvParameters){
    CommsHandler *comms = (CommsHandler*) pvParameters;
    while(true){
        comms->listen_to_api_clients();
        delay(10);
    }
}

void program_life(){
    Serial.printf("Taking photo %d", ++photoCount);

    // Capture photo
    camera_fb_t* fb = CameraHandler::take_photo();

    if (fb->len > MAX_PHOTO_SIZE) {
        Serial.printf("The size of the photo is too large (%d bytes)\n", fb->len);
        Serial.println();
        esp_camera_fb_return(fb);
        return;
    }

    Serial.println(" - Photo Size: " + String(fb->len) + " bytes");

    // Get Image ID (timestamp)
    timeval currentTime;    // 8 bytes
    gettimeofday(&currentTime, NULL);
    long imageId = comms.get_time_in_ms(currentTime);

    // Save photo to SD card
    fs::FS &fs = SD_MMC;

    String strPath = String(IMAGE_SDCARD_PATH) + "/" + String(imageId) + ".jpg";
    
    char path[strPath.length() + 1];
    strPath.toCharArray(path, strPath.length() + 1);

    if(!CardHandler::save_data(fs, path, fb->buf, fb->len)){
        Serial.println("Error: Problem occurred saving photo to SD card");
        esp_camera_fb_return(fb);
        return;
    }

    esp_camera_fb_return(fb);

    Serial.println(" - Photo saved to SD card with path: " + strPath);

    String url = "http://" + WiFi.localIP().toString() + ":" + String(API_SERVER_PORT) + IMAGE_URL_PATH + "?id=" + String(imageId);
    Serial.println(" - Image being served in URL: " + url);

    // Publish photo to MQTT
    comms.publish_image(imageId, url);

    Serial.println(" - Photo and info published to MQTT");
    Serial.println();
}

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("Plate Reader Terminal ready");
    Serial.println();
    Serial.flush();

    // Setup Camera
    while(!CameraHandler::init_camera())
        delay(1000);

    // Setup SD Card
    while(!CardHandler::init_memory_card())
        delay(1000);

    // Reset SD Card
    fs::FS &fs = SD_MMC;
    CardHandler::reset_card(fs);

    // Create image directory
    CardHandler::create_directory(fs, IMAGE_SDCARD_PATH);

    // Setup WiFi
    comms.connect_wifi();

    // Setup MQTT
    comms.connect_mqtt();

    // Start API server
    comms.start_api_server();

    // Start task of processing api requests
    xTaskCreate(despatcher_api_fN, "API request processing function", 2048, (void*) &comms, 5, NULL);

}

void loop() {
    // Check if connected to WiFi
    if(!comms.connected_to_wifi())
        comms.connect_wifi();

    // Check if connected to MQTT
    if(!comms.connected_to_mqtt())
        comms.connect_mqtt();

    program_life();

    delay(CAPTURE_RATE);
}
