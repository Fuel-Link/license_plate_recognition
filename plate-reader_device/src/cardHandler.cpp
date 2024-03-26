#include "cardHandler.h"

bool CardHandler::configDone = false;

bool CardHandler::init_memory_card(){
    if(configDone){
        Serial.println("Error: Card initialization already performed");
        return false;
    }

    // "/sdcard", true
    if (!SD_MMC.begin()) {
        Serial.println("Error: SD card initialization failed");
        return false;

    }

    uint8_t cardType = SD_MMC.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("Error: Unknown SD card type");
        return false;
    }

    Serial.print("SD_MMC Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);

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
        Serial.println("Error: Failed to open file for reading");
        return File();
    }
    return photoFile;
}

bool CardHandler::create_directory(fs::FS &fs, char* path){
    if(!configDone){
        Serial.println("Error: Card initialization not performed");
        return File();
    }

    if(!fs.mkdir(path)){
        Serial.println("Error: Creation of directory " + String(path) + " failed");
        return false;
    }
    return true;
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




