#include <Arduino.h>
#include <MQTTHandler.h>
#include <cameraHandler.h>
#include <cardHandler.h>

/*
    ##########################################################################
    ############                  Definitions                     ############
    ##########################################################################
*/
#define CAPTURE_RATE 1000   // in ms
#ifdef MQTT_MAX_PACKET_SIZE 
    #define MAX_PHOTO_SIZE MQTT_MAX_PACKET_SIZE
#else
    #define MAX_PHOTO_SIZE 20000    // in bytes
#endif
#define MAX_PHOTO_NAME_SIZE 50
// Comment when needed
#define SAVE_PHOTO_SD_CARD

/*
    ##########################################################################
    ############                 Global Variables                 ############
    ##########################################################################
*/
MQTTHandler mqtt;

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
void MQTTHandler::message_callback(char* topic, byte* payload, unsigned int length){
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

    Serial.print("Photo taken with size of: ");
    Serial.println(fb->len);

    #ifdef SAVE_PHOTO_SD_CARD
        // Save photo to SD card
        fs::FS &fs = SD_MMC;
        char path[MAX_PHOTO_NAME_SIZE];
        sprintf(path, "/photo_%d.jpg", photoCount);
        CardHandler::save_data(fs, path, fb->buf, fb->len);
    #endif

    // Publish photo to MQTT
    mqtt.publish_image(fb->buf, fb->len);

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

    #ifdef SAVE_PHOTO_SD_CARD
        // Setup SD Card
        while(!CardHandler::init_memory_card())
            delay(1000);

        fs::FS &fs = SD_MMC;
        CardHandler::reset_card(fs);
    #endif

    // Setup WiFi
    mqtt.connect_wifi();

    // Setup MQTT
    mqtt.connect_mqtt();

}

void loop() {
    // Check if connected to WiFi
    if(!mqtt.connect_to_wifi())
        mqtt.connect_wifi();

    // Check if connected to MQTT
    if(!mqtt.connected_to_mqtt())
        mqtt.connect_mqtt();

    program_life();

    delay(CAPTURE_RATE);
}
