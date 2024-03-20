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
#define MAX_PHOTO_SIZE 20000    // in bytes
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
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

/*
    ##########################################################################
    ############                  Shared Code                     ############
    ##########################################################################
*/

void program_life(){
    Serial.printf("Taking photo %d", photoCount);
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
        if(CardHandler::init_memory_card()){
            // Save photo to SD card
            fs::FS &fs = SD_MMC;
            char path[MAX_PHOTO_NAME_SIZE];
            sprintf(path, "/photo_%d.jpg", photoCount);
            CardHandler::save_data(fs, path, fb->buf, fb->len);
            CardHandler::terminate();
        }
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

    // Setup WiFi
    mqtt.connect_wifi();

    // Setup MQTT
    mqtt.connect_mqtt();

    // Setup Camera
    while(!CameraHandler::init_camera())
        delay(1000);

    #ifdef SAVE_PHOTO_SD_CARD
        // Setup SD Card
        while(!CardHandler::init_memory_card())
            delay(1000);
    #endif

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
