#pragma once

//! \attention for WiFi include to always be first
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiCredentials.h>
#include <ArduinoJson.h>
#include <psram.h>

/*
    ##########################################################################
    ############                  Definitions                     ############
    ##########################################################################
*/
//! Attention: Please refer to the Readme file as to configure the WiFi and MQTT credentials.

//! URL or Host name of the API service
const char* IN_TOPIC = "ditto-tutorial/my.test:octopus";
const char* OUT_TOPIC = "ditto-tutorial/my.test:octopus";
const char* THING_ID = "octopus";

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
    boolean connect_to_wifi();

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