#pragma once

#include "FS.h"
#include "SD_MMC.h"
#include "esp_log.h"

namespace CardHandler{
    extern bool configDone;
    bool init_memory_card();
    bool save_data(fs::FS& fs, char* path, uint8_t* data, int size);
    bool append_data(fs::FS& fs, char* path, uint8_t* data, int size);
    File read_data(fs::FS& fs, char* path);
    bool create_directory(fs::FS& fs, const char* path);
    bool reset_card(fs::FS& fs);
    bool terminate();
};