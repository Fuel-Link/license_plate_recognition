#include "cardHandler.h"

bool CardHandler::configDone = false;

bool CardHandler::init_memory_card(){
    if(configDone){
        Serial.println("Error: Card initialization already performed");
        return false;
    }

    Serial.println("Initializing SD card: ");

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

    Serial.print(" - SD_MMC Card Type: ");
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
    Serial.printf(" - SD_MMC Card Size: %lluMB\n", cardSize);

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

bool CardHandler::create_directory(fs::FS &fs, const char* path){
    if(!configDone){
        Serial.println("Error: Card initialization not performed");
        return File();
    }

    Serial.println(" - Creating directory: " + String(path));

    if(!fs.mkdir(path)){
        Serial.println("Error: Creation of directory " + String(path) + " failed");
        return false;
    }

    Serial.println();
    return true;
}

bool CardHandler::delete_file(fs::FS &fs, const char * path){
    if(!configDone){
        Serial.println("Error: Card initialization not performed");
        return File();
    }

    if(!fs.remove(path)){
        Serial.println("Error: Deletion of file " + String(path) + " failed");
        return false;
    }   
    return true;
}

bool CardHandler::rename_file(fs::FS &fs, const char * currentPath, const char * renamedPath){
    if(!configDone){
        Serial.println("Error: Card initialization not performed");
        return File();
    }

    if(!fs.rename(currentPath, renamedPath)){
        Serial.println("Error: Renaming of file " + String(currentPath) + " to " + String(renamedPath) + " failed");
        return false;
    }   
    return true;
}

bool CardHandler::remove_directory(fs::FS &fs, const char * path){
    if(!configDone){
        Serial.println("Error: Card initialization not performed");
        return File();
    }

    if(!fs.rmdir(path)){
        Serial.println("Error: Deletion of directory " + String(path) + " failed");
        return false;
    }   
    return true;
}

void CardHandler::print_directory(fs::FS &fs, const char * dirname, uint8_t levels){
    if(!configDone){
        Serial.println("Error: Card initialization not performed");
        return;
    }

    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                print_directory(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void CardHandler::test_file_IO(fs::FS &fs, const char * path){
    if(!configDone){
        Serial.println("Error: Card initialization not performed");
        return;
    }

    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

bool CardHandler::reset_card(fs::FS& fs){
    if (!configDone) {
        Serial.println("Error: Card initialization not performed");
        return false;
    }

    Serial.println(" - Resetting card:"); 

    File root = fs.open("/");
    File file = root.openNextFile();
    
    while (file) {
        if (file.isDirectory()) {
            file = root.openNextFile();
            continue;
        }

        Serial.print("  - Deleting file: ");
        Serial.println(file.name());

        // As per the source code of the SD_MMC library,
        // all filenames need to start with a '/'
        char result[strlen(file.name()) + 2]; // +1 for null-terminator and '/'
        sprintf(result, "/%s", file.name());

        fs.remove(result);

        file.close();
        file = root.openNextFile();
    }
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




