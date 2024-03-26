#include <Arduino.h>
#include <CommsHandler.h>
#include <cameraHandler.h>
#include <cardHandler.h>
#include <psram.h>
#include <utils.h>

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

#ifndef READ_FILE_BUFFER_SIZE
#define READ_FILE_BUFFER_SIZE 40000
#endif

/*
    ##########################################################################
    ############                 Global Variables                 ############
    ##########################################################################
*/
CommsHandler comms;
int photoCount = 0;
long lastImageId = 0;
PSRAMHandler psram;

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
    // Checks the path
    if(strcmp(request.path(), "/images") != 0){
        response.sendStatus(400);
        return;
    }

    // let the user request the last image, without need for id
    long imageId = lastImageId;
    
    // Check the query parameters
    char id[50];
    request.query("id", id, 50);
    if(strlen(id) > 0)
        imageId = atol(id);
    
    String strPath = get_image_path(imageId);
    char path[strPath.length() + 1];
    strPath.toCharArray(path, strPath.length() + 1);

    // Check if the image exists      
    fs::FS &fs = SD_MMC;
    File file = CardHandler::read_data(fs, path);
    if(file.size() == 0){
        response.sendStatus(404); // image not present
        return;
    }

    // Set the headers and send the response
    response.set("Content-Type", "image/jpeg");
    response.set("Connection", "close");

    // Allocate memory for the sending buffer
    psram.allocate(READ_FILE_BUFFER_SIZE);

    // Continuously read and send the image in the buffer
    int bytesRead = 0;
    int bytesCounter = 0;
    do{
        bytesRead = file.readBytes((char*) psram.get_mem_ptr(), READ_FILE_BUFFER_SIZE);
        response.write(psram.get_mem_ptr(), bytesRead);
        bytesCounter += bytesRead;
    }while (bytesRead > 0);
    

    Serial.println("Image sent with size: " + String(bytesCounter) + " bytes");

    // Free the memory
    psram.destroy();
    file.close();

    // Finish response
    response.end();
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

    String strPath = get_image_path(imageId);
    
    char path[strPath.length() + 1];
    strPath.toCharArray(path, strPath.length() + 1);

    if(!CardHandler::save_data(fs, path, fb->buf, fb->len)){
        Serial.println("Error: Problem occurred saving photo to SD card");
        esp_camera_fb_return(fb);
        return;
    }

    esp_camera_fb_return(fb);

    lastImageId = imageId;
    Serial.println(" - Photo saved to SD card with path: " + strPath);

    String url = "http://" + WiFi.localIP().toString() + ":" + String(API_SERVER_PORT) + IMAGE_URL_PATH + "?id=" + String(imageId);
    Serial.println(" - Image being served in URL: " + url);

    // Publish photo to MQTT
    comms.publish_image(currentTime, url);

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
    //xTaskCreate(despatcher_api_fN, "API request processing function", 4096, (void*) &comms, 5, NULL);

}

void loop() {
    // Check if connected to WiFi
    if(!comms.connected_to_wifi())
        comms.connect_wifi();

    // Check if connected to MQTT
    if(!comms.connected_to_mqtt())
        comms.connect_mqtt();

    program_life();

    // Listen to API clients and hold the program for CAPTURE_RATE ms
    uint32_t start = millis();
    while(millis() - start < CAPTURE_RATE){
        comms.listen_to_api_clients();
        delay(1);
    }

    //delay(CAPTURE_RATE);
}
