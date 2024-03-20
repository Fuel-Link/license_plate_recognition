#include "cardHandler.h"

bool CardHandler::configDone = false;

bool CardHandler::init_memory_card(){
    if(configDone){
        Serial.println("Error: Card initialization already performed");
        return false;
    }
    // Mount SD card
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("Error: SD card initialization failed");
        return false;
    }

    uint8_t cardType = SD_MMC.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("Error: Unknown SD card type");
        return false;
    }

    configDone = true;
    return true;
}

bool CardHandler::save_data(fs::FS& fs, char* path, uint8_t* data, int size){
    if(!configDone){
        Serial.println("Error: Card initialization not performed");
        return false;
    }

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }
    file.write(data, size);
    file.close();
    return true;
}

bool CardHandler::append_data(fs::FS& fs, char* path, uint8_t* data, int size){
    if(!configDone){
        Serial.println("Error: Card initialization not performed");
        return false;
    }

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }
    file.write(data, size);
    file.close();
    return true;
} 

File CardHandler::read_data(fs::FS& fs, char* path){
    if(!configDone){
        Serial.println("Error: Card initialization not performed");
        return File();
    }

    File photoFile = SD_MMC.open(path);
    if (!photoFile) {
        Serial.println("Failed to open file for reading");
        return File();
    }
    return photoFile;
}

bool CardHandler::reset_card(fs::FS& fs){
    if (!configDone) {
        Serial.println("Error: Card initialization not performed");
        return false;
    }

    File root = fs.open("/");
    File file = root.openNextFile();
    
    while (file) {
        if (file.isDirectory()) {
            file = root.openNextFile();
            continue;
        }

        Serial.print("Deleting file: ");
        Serial.println(file.name());

        // As per the source code of the SD_MMC library,
        // all filenames need to start with a '/'
        char result[strlen(file.name()) + 2]; // +1 for null-terminator and '/'
        sprintf(result, "/%s", file.name());

        fs.remove(result);

        file.close();
        file = root.openNextFile();
    }
    Serial.println();
    return true;
}

bool CardHandler::terminate(){
    if (!configDone) {
        Serial.println("Error: Card initialization not performed");
        return false;
    }
    SD_MMC.end();
    configDone = false;
    return true;
}




