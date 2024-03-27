#pragma once

#include <Arduino.h>

/**
 *  Compute the correct image path to the image present in the SD card
 *      \param imageId: the image id
*/
String get_image_path(long imageId);

