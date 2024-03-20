#pragma once

//! \attention for WiFi include to always be first
#include <WiFi.h>
#include <PubSubClient.h>

/*
    ##########################################################################
    ############                  Definitions                     ############
    ##########################################################################
*/
//! SSID of your internet enabled WiFi network
const char WIFI_SSID[] = "YOUR_WIFI_SSID";  // CHANGE IT
//! Password of your internet enabled WiFi network
const char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";  // CHANGE IT
//! IP address of the MQTT broker
const char MQTT_SERVER_IP[] = "YOUR_MQTT_BROKER_IP_ADDRESS";    // CHANGE IT
//! Port of the MQTT broker
const int MQTT_SERVER_PORT = 1883;
//! URL or Host name of the API service
const String IMAGE_CHANNEL = "raw-image";

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