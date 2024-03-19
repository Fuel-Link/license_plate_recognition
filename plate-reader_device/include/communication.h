#pragma once

//! \attention for WiFi include to always be first
#include <Arduino_JSON.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <message.h>

/*
    ##########################################################################
    ############                  Definitions                     ############
    ##########################################################################
*/
//! SSID of your internet enabled WiFi network
const char WIFI_SSID[] = "YOUR_WIFI_SSID";  // CHANGE IT
//! Password of your internet enabled WiFi network
const char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";  // CHANGE IT
//! URL or Host name of the API service
const String HOST_NAME = "https://api.platerecognizer.com/v1";
//! Path of the POST request for the plate reader
const String IMAGE_POST_PATH = "/plate-reader";

/*
    ##########################################################################
    ############               WebAPI declaration                 ############
    ##########################################################################
*/
/*! \class WebAPI
    \brief Handles interactions with the web API.
*/
class WebAPI {
   private:
    //!< HTTPClient instance for making HTTP requests.
    HTTPClient http;
    //!< Flag to indicate if WiFi is connected.
    bool wifiConnected = false;

   public:
    //! \brief Constructor for WebAPI class.
    WebAPI();

    //! \brief Destructor for WebAPI class.
    ~WebAPI();

    /*! \brief Connect to the WiFi network.
    */
    void connect_wifi();

    /*! \brief Post discovery information to the API.
        \param nodeId ID of the node.
        
        \return True if post is successful, false otherwise.
    */
    bool post_image_to_read(uint16_t nodeId);
};