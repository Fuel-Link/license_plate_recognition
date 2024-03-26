#pragma once

//! \attention for WiFi include to always be first
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiCredentials.h>
#include <ArduinoJson.h>
#include <ESPNtpClient.h>
#include <aWOT.h>
#include <utils.h>

/*
    ##########################################################################
    ############                  Definitions                     ############
    ##########################################################################
*/
//! Attention: Please refer to the Readme file as to configure the WiFi and MQTT credentials.

//! NTP configurations
#define SHOW_TIME_PERIOD 1000
#define NTP_TIMEOUT 5000
#define NTP_TIMEZONE TZ_Europe_London

//! 
#define API_SERVER_PORT 80
#define API_SERVER_IP IPAddress(0,0,0,0)
/*
    ##########################################################################
    ############               CommsHandler declaration                 ############
    ##########################################################################
*/
/*! \class CommsHandler
    \brief Handles interactions with the CommsHandler.
*/
class CommsHandler {
   private:
    //!< WiFi and MQTT clients
    WiFiClient espClient;
    PubSubClient mqttClient;
    WiFiServer apiServer; 
    Application app;

    //! URL or Host name of the API service
    const char* IN_TOPIC = "plate-reader/in/org.eclipse.ditto:1b183fb5-0942-469e-8475-927d7058e201";
    const char* OUT_TOPIC = "plate-reader/out/org.eclipse.ditto:1b183fb5-0942-469e-8475-927d7058e201";
    const char* IMAGE_TOPIC = "plate-reader/imageCaptured/org.eclipse.ditto:1b183fb5-0942-469e-8475-927d7058e201";
    const char* THING_ID = "1b183fb5-0942-469e-8475-927d7058e201";

    //! NTP configurations
    const PROGMEM char* NTP_SERVER = "pool.ntp.org";
    boolean syncEventTriggered = false; // True if a time even has been triggered
    NTPEvent_t ntpEvent; // Last triggered event

   public:
    //! \brief Constructor for CommsHandler class.
    CommsHandler();

    //! \brief Destructor for CommsHandler class.
    ~CommsHandler();

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

    /*! \brief Start the API server.
    */
    void start_api_server();

    /*! \brief Process the API request. Present in the `main.cpp` file.
        \param request Request to be processed.
        \param response Response to be sent.
    */
    static void process_api_request(Request &request, Response &response);

    /*! \brief Publish an image to the MQTT broker.
        \param imageId Image ID.
        \param imageURL URL of the image.
        \return True if the image was published, false otherwise.
    */
    bool publish_image(timeval& imageTime, String imageURL);

    /*  \brief Listen to requests from API clients.
    */
    void listen_to_api_clients();

    /*! \brief Callback function for MQTT messages. Present in the `main.cpp` file.
        \param topic Topic of the message.
        \param payload Payload of the message.
        \param length Length of the message.
        \note This function should be located in the main.cpp file, as to 
            access the processing functions of other modules.
    */
    static void mqtt_message_callback(char* topic, byte* payload, unsigned int length);

    /*! \brief Initialize the NTP client.
    */
    void initialize_ntp_client();

    /*! \brief Process the NTP event.
        \param ntpEvent NTP event to be processed.
    */
    void process_sync_event (NTPEvent_t ntpEvent);

    /*! \brief Get the current time in milliseconds.
        \param currentTime Current time.
        \return Current time in milliseconds.
    */
    long get_time_in_ms(timeval& currentTime);

    /*! \brief Get the current time in string format.
        \param currentTime Current time.
        \return Current time in string format, according to ISO 8601.
        \example "2024-03-24T12:30:00Z"
    */
    String get_time_string(timeval& currentTime);

    /*! \brief Get the current time in year.
        \param currentTime Current time.
        \return Current year.
    */
    int get_time_year(timeval& currentTime);
};