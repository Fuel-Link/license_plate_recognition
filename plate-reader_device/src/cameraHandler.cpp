#include "cameraHandler.h"

bool CameraHandler::configDone = false;

bool CameraHandler::init_camera(){
    if(configDone){
        Serial.println("Error: Camera configuration already performed");
        return false;
    }

    Serial.println("Starting Camera initialization");

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 5;
    config.pin_d1 = 18;
    config.pin_d2 = 19;
    config.pin_d3 = 21;
    config.pin_d4 = 36;
    config.pin_d5 = 39;
    config.pin_d6 = 34;
    config.pin_d7 = 35;
    config.pin_xclk = 0;
    config.pin_pclk = 22;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_sccb_sda = 26;
    config.pin_sccb_scl = 27;
    config.pin_pwdn = 32;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;

    //Select lower framesize if the camera doesn't support PSRAM
    if (psramFound())
    {
        Serial.println(" - Camera working with PSRAM");
        config.frame_size = FRAMESIZE_SVGA;      //FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA, XUGA == 100K+, SVGA = 25K+
        config.jpeg_quality = 10;                //10-63 lower number means higher quality
        config.fb_count = 2;
    }
    else
    {
        Serial.println(" - Camera not working with PSRAM");
        config.frame_size = FRAMESIZE_XGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }


    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf(" - Error: Camera initialization failed with error 0x%x", err);
        return false;
    }
    
    sensor_t * s = esp_camera_sensor_get();
    s->set_brightness(s, 0);     // -2 to 2
    s->set_contrast(s, 0);       // -2 to 2
    s->set_saturation(s, 0);     // -2 to 2
    s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
    s->set_wb_mode(s, 1);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
    s->set_aec2(s, 1);           // 0 = disable , 1 = enable
    s->set_ae_level(s, 0);       // -2 to 2
    s->set_aec_value(s, 450);    // 0 to 1200
    s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
    s->set_agc_gain(s, 0);       // 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
    s->set_bpc(s, 1);            // 0 = disable , 1 = enable
    s->set_wpc(s, 0);            // 0 = disable , 1 = enable
    s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
    s->set_lenc(s, 0);           // 0 = disable , 1 = enable
    s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
    s->set_vflip(s, 0);          // 0 = disable , 1 = enable
    s->set_dcw(s, 1);            // 0 = disable , 1 = enable
    s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
    
    Serial.println("Camera successfully configured");
    Serial.println();
    configDone = true;
    return true;
}

camera_fb_t* CameraHandler::take_photo(){
    if(!configDone){
        Serial.println("Error: Camera initialization not performed");
        return nullptr;
    }
    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Error: Failed to capture photo");
      return nullptr;
    }

    return fb;

}
