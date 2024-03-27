#pragma once

#include <Arduino.h>
#include "esp_camera.h"

namespace CameraHandler{
    extern bool configDone;
    
    /**
     * Initialze the camera
    */
    bool init_camera();

    /**
     *  Take a photo
     *      \return camera_fb_t* - pointer to the photo
     *      \note The photo pointer is present in the PSRAM
     *      \attention The photo must be released with esp_camera_fb_return
    */
    camera_fb_t* take_photo();

};