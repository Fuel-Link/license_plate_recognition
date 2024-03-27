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

#ifndef JSON_URL_PATH
#define JSON_URL_PATH "/swagger.json"
#endif

#ifndef DOCS_URL_PATH
#define DOCS_URL_PATH "/swaggerUI"
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
PSRAMHandler psramAPI;

//! Swagger stuff. They are divided into parts because of dinamically setting the correct IP address
const char * swaggerJSONPart1 = "{\"swagger\":\"2.0\",\"info\":{\"description\":\"License-plate reader IOT device API server, used for interacting directly with the device functions.\",\"version\":\"1.0.0\",\"title\":\"License-plate reader\"},\"host\":\"";
const char * swaggerJSONPart2 = "\",\"tags\":[{\"name\":\"Images\",\"description\":\"Getting the recorded images\"}],\"paths\":{\"/images\":{\"get\":{\"tags\":[\"Images\"],\"summary\":\"Endpoint for downloading the last recorded image from the onboard SD card\",\"description\":\"\",\"operationId\":\"getLastRecordedImage\",\"responses\":{\"200\":{\"description\":\"If present, the last recorded image\",\"schema\":{\"$ref\":\"#/definitions/image\"}},\"404\":{\"description\":\"In case of the requested image not being present\"}}}},\"/images/{id}\":{\"get\":{\"tags\":[\"Images\"],\"summary\":\"Endpoint for downloading a saved image from the onboard SD card\",\"description\":\"\",\"operationId\":\"getImageById\",\"parameters\":[{\"name\":\"id\",\"in\":\"path\",\"description\":\"ID of the image to retrieve\",\"required\":true,\"type\":\"string\"}],\"responses\":{\"200\":{\"description\":\"If present, the requested image\",\"schema\":{\"$ref\":\"#/definitions/image\"}},\"404\":{\"description\":\"In case of the requested image not being present\"}}}}},\"definitions\":{\"image\":{\"type\":\"string\"}}}";
const char * swaggerUIPart1 = "<!DOCTYPE html><html><head> <meta charset=\"UTF-8\"> <meta http-equiv=\"x-ua-compatible\" content=\"IE=edge\"> <title>Swagger UI</title> <link href='https://cdnjs.cloudflare.com/ajax/libs/meyer-reset/2.0/reset.min.css' media='screen' rel='stylesheet' type='text/css'/> <link href='https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/2.2.10/css/screen.css' media='screen' rel='stylesheet' type='text/css'/> <script>if (typeof Object.assign !='function'){(function (){Object.assign=function (target){'use strict'; if (target===undefined || target===null){throw new TypeError('Cannot convert undefined or null to object');}var output=Object(target); for (var index=1; index < arguments.length; index++){var source=arguments[index]; if (source !==undefined && source !==null){for (var nextKey in source){if (Object.prototype.hasOwnProperty.call(source, nextKey)){output[nextKey]=source[nextKey];}}}}return output;};})();}</script> <script src='https://cdnjs.cloudflare.com/ajax/libs/jquery/1.8.0/jquery-1.8.0.min.js' type='text/javascript'></script> <script>(function(b){b.fn.slideto=function(a){a=b.extend({slide_duration:\"slow\",highlight_duration:3E3,highlight:true,highlight_color:\"#FFFF99\"},a);return this.each(function(){obj=b(this);b(\"body\").animate({scrollTop:obj.offset().top},a.slide_duration,function(){a.highlight&&b.ui.version&&obj.effect(\"highlight\",{color:a.highlight_color},a.highlight_duration)})})}})(jQuery); </script> <script>jQuery.fn.wiggle=function(o){var d={speed:50,wiggles:3,travel:5,callback:null};var o=jQuery.extend(d,o);return this.each(function(){var cache=this;var wrap=jQuery(this).wrap(' <div class=\"wiggle-wrap\"></div>').css(\"position\",\"relative\");var calls=0;for(i=1;i<=o.wiggles;i++){jQuery(this).animate({left:\"-=\"+o.travel},o.speed).animate({left:\"+=\"+o.travel*2},o.speed*2).animate({left:\"-=\"+o.travel},o.speed,function(){calls++;if(jQuery(cache).parent().hasClass('wiggle-wrap')){jQuery(cache).parent().replaceWith(cache);}if(calls==o.wiggles&&jQuery.isFunction(o.callback)){o.callback();}});}});}; </script> <script src='https://cdnjs.cloudflare.com/ajax/libs/jquery.ba-bbq/1.2.1/jquery.ba-bbq.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/handlebars.js/4.0.5/handlebars.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/lodash-compat/3.10.1/lodash.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/backbone.js/1.1.2/backbone-min.js' type='text/javascript'></script> <script>Backbone.View=(function(View){return View.extend({constructor: function(options){this.options=options ||{}; View.apply(this, arguments);}});})(Backbone.View); </script> <script src='https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/2.2.10/swagger-ui.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.10.0/highlight.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.10.0/languages/json.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/json-editor/0.7.28/jsoneditor.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/marked/0.3.6/marked.min.js' type='text/javascript'></script> <script type=\"text/javascript\">$(function (){url=\"http://";
const char * swaggerUIPart2 = "/swagger.json\"; hljs.configure({highlightSizeThreshold: 5000}); window.swaggerUi=new SwaggerUi({url: url, dom_id: \"swagger-ui-container\", supportedSubmitMethods: ['get', 'post', 'put', 'delete', 'patch'], validatorUrl: null, onComplete: function(swaggerApi, swaggerUi){}, onFailure: function(data){log(\"Unable to Load SwaggerUI\");}, docExpansion: \"none\", jsonEditor: false, defaultModelRendering: 'schema', showRequestHeaders: false, showOperationIds: false}); window.swaggerUi.load(); function log(){if ('console' in window){console.log.apply(console, arguments);}}}); </script></head> <body class=\"swagger-section\"><div id='header'><div class=\"swagger-ui-wrap\"> <a id=\"logo\" href=\"http://swagger.io\"><img class=\"logo__img\" alt=\"swagger\" height=\"30\" width=\"30\" src=\"https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/2.2.10/images/logo_small.png\"/><span class=\"logo__title\">swagger</span></a><form id='api_selector'></form></div></div><div id=\"message-bar\" class=\"swagger-ui-wrap\" data-sw-translate>&nbsp;</div><div id=\"swagger-ui-container\" class=\"swagger-ui-wrap\"></div></body></html>";

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
    if(strcmp(request.path(), IMAGE_URL_PATH) == 0) {
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
        if(!psram.allocate(READ_FILE_BUFFER_SIZE)){
            response.sendStatus(500);
            return;
        }

        // Continuously read and send the image in the buffer
        int bytesRead = 0;
        do{
            bytesRead = file.readBytes((char*) psram.get_mem_ptr(), READ_FILE_BUFFER_SIZE);
            response.write(psram.get_mem_ptr(), bytesRead);
        }while (bytesRead > 0);

        // Free the memory
        psram.destroy();
        file.close();

        // Finish response
        response.end();

    } else if(strcmp(request.path(), JSON_URL_PATH) == 0){
        response.set("Content-Type", "application/json");
        response.set("Connection", "close");

        String wifiStr = WiFi.localIP().toString();
        const char* ipAddress = wifiStr.c_str();

        response.write((uint8_t *) swaggerJSONPart1, strlen(swaggerJSONPart1));
        response.write((uint8_t *) ipAddress, wifiStr.length());
        response.write((uint8_t *) swaggerJSONPart2, strlen(swaggerJSONPart2));

        response.end();

    } else if(strcmp(request.path(), DOCS_URL_PATH) == 0){
        response.set("Content-Type", "text/html");
        response.set("Connection", "close");

        String wifiStr = WiFi.localIP().toString();
        const char* ipAddress = wifiStr.c_str();

        response.write((uint8_t *) swaggerUIPart1, strlen(swaggerUIPart1));
        response.write((uint8_t *) ipAddress, wifiStr.length());
        response.write((uint8_t *) swaggerUIPart2, strlen(swaggerUIPart2));

        response.end();

    } else {
        response.sendStatus(400);
        return;
    }
}

/*
    ##########################################################################
    ############                  Shared Code                     ############
    ##########################################################################
*/

/*
void despatcher_api_fN(void* pvParameters){
    CommsHandler *comms = (CommsHandler*) pvParameters;
    while(true){
        comms->listen_to_api_clients();
        delay(10);
    }
}*/

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
}
