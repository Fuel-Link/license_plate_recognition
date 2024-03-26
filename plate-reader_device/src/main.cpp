#include <Arduino.h>
#include <CommsHandler.h>
#include <cameraHandler.h>
#include <cardHandler.h>

/*
    ##########################################################################
    ############                  Definitions                     ############
    ##########################################################################
*/
#define CAPTURE_RATE 5000   // in ms
#ifdef MQTT_MAX_PACKET_SIZE 
    #define MAX_PHOTO_SIZE MQTT_MAX_PACKET_SIZE
#else
    #define MAX_PHOTO_SIZE 20000    // in bytes
#endif
#define IMAGE_CARD_PATH "/images"

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
void CommsHandler::message_callback(char* topic, byte* payload, unsigned int length){
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

/*
    ##########################################################################
    ############                  Shared Code                     ############
    ##########################################################################
*/

void program_life(){
    Serial.println("");
    Serial.printf("Taking photo %d", ++photoCount);
    Serial.println();

    // Capture photo
    camera_fb_t* fb = CameraHandler::take_photo();

    if (fb->len > MAX_PHOTO_SIZE) {
        Serial.printf("The size of the photo is too large (%d bytes)\n", fb->len);
        Serial.println();
        esp_camera_fb_return(fb);
        return;
    }

    // Get Image ID (timestamp)
    timeval currentTime;    // 8 bytes
    gettimeofday(&currentTime, NULL);
    long imageId = comms.get_time_in_ms(currentTime);

    // Save photo to SD card
    fs::FS &fs = SD_MMC;

    String tempPath = IMAGE_CARD_PATH + '/' + String(imageId) + ".jpg";
    char path[tempPath.length() + 1];
    tempPath.toCharArray(path, tempPath.length() + 1);

    sprintf(path, "/photo_%d.jpg", photoCount);
    CardHandler::save_data(fs, path, fb->buf, fb->len);

    // Publish photo to MQTT
    comms.publish_image(imageId, tempPath);

    esp_camera_fb_return(fb);
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
    CardHandler::create_directory(fs, IMAGE_CARD_PATH);

    // Setup WiFi
    comms.connect_wifi();

    // Setup MQTT
    comms.connect_mqtt();

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
