#include "utils.h"

String get_image_path(long imageId){
    return String(IMAGE_SDCARD_PATH) + "/" + String(imageId) + ".jpg";
}