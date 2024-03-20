#pragma once

#include <Arduino.h>
#include "esp_camera.h"

namespace CameraHandler{
    extern bool configDone;
    bool init_camera();
    camera_fb_t* take_photo();

};