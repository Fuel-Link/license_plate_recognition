#pragma once

//! Disclaimer - Some functions were made with help of SDMMC_Tests.ino from the ESP32 library

#include "FS.h"
#include "SD_MMC.h"
#include "esp_log.h"

namespace CardHandler{
    extern bool configDone;

    /**
     *  Initialize the memory card
     *      \return bool - `true` if the memory card was initialized successfully, `false` otherwise
    */
    bool init_memory_card();

    /**
     *  Save data to the memory card
     *      \param fs: the file system
     *      \param path: the path to the file
     *      \param data: the data to be saved
     *      \param size: the size of the data
     *      \return bool - `true` if the data was saved successfully, `false` otherwise
     */
    bool save_data(fs::FS& fs, char* path, uint8_t* data, int size);

    /**
     *  Append data to the memory card
     *      \param fs: the file system
     *      \param path: the path to the file
     *      \param data: the data to be appended
     *      \param size: the size of the data
     *      \return bool - `true` if the data was appended successfully, `false` otherwise
     */
    bool append_data(fs::FS& fs, char* path, uint8_t* data, int size);

    /**
     *  Read data from the memory card
     *      \param fs: the file system
     *      \param path: the path to the file
     *      \return File - the file containing the data
     *      \attention If the file doesn't exist, an empty file is returned, which can be 
     *          checked with file.size() == 0
     *      \note The file must be closed with file.close()
     */
    File read_data(fs::FS& fs, char* path);

    /**
     *  Create a directory in the memory card
     *      \param fs: the file system
     *      \param path: the path to the directory
     *      \return bool - `true` if the directory was created successfully, `false` otherwise
     */
    bool create_directory(fs::FS& fs, const char* path);

    /**
     *  Remove a file from the memory card
     *      \param fs: the file system
     *      \param path: the path to the file
     *      \return bool - `true` if the file was removed successfully, `false` otherwise
     */
    bool reset_card(fs::FS& fs);

    /**
     *  Remove a file from the memory card
     *      \param fs: the file system
     *      \param path: the path to the file
     *      \return bool - `true` if the file was removed successfully, `false` otherwise
    */
    bool delete_file(fs::FS &fs, const char * path);

    /**
     *  Rename a file in the memory card
     *      \param fs: the file system
     *      \param currentPath: the current path to the file
     *      \param renamedPath: the new path to the file
     *      \return bool - `true` if the file was renamed successfully, `false` otherwise
    */
    bool rename_file(fs::FS &fs, const char * currentPath, const char * renamedPath);

    /**
     *  Remove a directory from the memory card
     *      \param fs: the file system
     *      \param path: the path to the directory
     *      \return bool - `true` if the directory was removed successfully, `false` otherwise
    */
    bool remove_directory(fs::FS &fs, const char * path);

    /**
     *  Print the file names in a directory
     *      \param fs: the file system
     *      \param dirname: the path to the directory
     *      \param levels: the number of levels to list
    */
    void print_directory(fs::FS &fs, const char * dirname, uint8_t levels);

    /**
     *  Test the file I/O (speed of writing and reading of files)
     *      \param fs: the file system
     *      \param path: the path to the file
    */
    void test_file_IO(fs::FS &fs, const char * path);

    /**
     *  Terminate the memory card
     *      \return bool - `true` if the memory card was terminated successfully, `false` otherwise
     */
    bool terminate();
};