#pragma once

//! \attention for WiFi include to always be first
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiCredentials.h>
#include <ArduinoJson.h>
#include <psram.h>
#include "arduino_base64.hpp"

/*
    ##########################################################################
    ############                  Definitions                     ############
    ##########################################################################
*/
//! Attention: Please refer to the Readme file as to configure the WiFi and MQTT credentials.

/*
    ##########################################################################
    ############               MQTTHandler declaration                 ############
    ##########################################################################
*/
/*! \class MQTTHandler
    \brief Handles interactions with the MQTTHandler.
*/
class MQTTHandler {
   private:
    //!< MQTTHandler instance for making HTTP requests.
    WiFiClient espClient;
    PubSubClient mqttClient;
    PSRAMHandler psram;

    //! URL or Host name of the API service
    const char* IN_TOPIC = "plate-reader/in/org.eclipse.ditto:9b4f7c51-56ea-4445-b76e-70a427b1d8c1";
    const char* OUT_TOPIC = "plate-reader/out/org.eclipse.ditto:9b4f7c51-56ea-4445-b76e-70a427b1d8c1";
    const char* IMAGE_TOPIC = "plate-reader/imageCaptured/org.eclipse.ditto:9b4f7c51-56ea-4445-b76e-70a427b1d8c1";
    const char* THING_ID = "9b4f7c51-56ea-4445-b76e-70a427b1d8c1";

   public:
    //! \brief Constructor for MQTTHandler class.
    MQTTHandler();

    //! \brief Destructor for MQTTHandler class.
    ~MQTTHandler();

    /*! \brief Connect to the WiFi network.
    */
    void connect_wifi();

    /*! \brief Check if connected to the WiFi network.
        \return True if connected, false otherwise.
    */
    boolean connected_to_wifi();

    /*! \brief Connect to the MQTT broker.
    */
    void connect_mqtt();

    /*! \brief Check if connected to the MQTT broker.
        \return True if connected, false otherwise.
    */
    boolean connected_to_mqtt();

    /*! \brief publish an image to a channel.
        \param path Path to the image to be posted.
        \param size Size of the image to be posted.        
        \return True if post is successful, false otherwise.
    */
    bool publish_image(uint8_t *data, uint32_t size);

    /*! \brief Callback function for MQTT messages.
        \param topic Topic of the message.
        \param payload Payload of the message.
        \param length Length of the message.
        \note This function should be located in the main.cpp file, as to 
            access the processing functions of other modules.
    */
    static void message_callback(char* topic, byte* payload, unsigned int length);
};