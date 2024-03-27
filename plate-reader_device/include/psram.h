#pragma once

#include <Arduino.h>

#ifdef MQTT_MAX_PACKET_SIZE
    #define MAX_PHOTO_SIZE MQTT_MAX_PACKET_SIZE
#else
    #define MAX_PHOTO_SIZE 20000
#endif

//#define DEBUG

class PSRAMHandler{
  private:
    uint8_t *PSRAMptr = nullptr;    //create a global pointer to the array to send, so all functions have access
    uint32_t occupiedPSRAM = 0;
  public:
    PSRAMHandler(){};
    ~PSRAMHandler(){ destroy(); };

    /*
     *  Test if PSRAM is available
    */
    bool test();

    /*
     *  Allocate PSRAM memory
     *      @param amount: amount of memory to allocate
    */
    bool allocate(uint32_t amount);

    /*
     *  De-allocate PSRAM memory
    */
    bool destroy();

    /*
     *  Store data in PSRAM
     *      @param data: data to store
     *      @param size: size of the data
    */
    bool store(uint8_t *data, uint32_t size);

     /*
     *  Store data in PSRAM
     *      @param data: data to store
     *      @param size: size of the data
    */
    bool store(const char * data, size_t size);

    /*
     *  Reset PSRAM memory
    */
    bool reset();

    /*
     *  Get the pointer to the PSRAM memory
     *      @param offset: offset to the memory
    */
    uint8_t* get_mem_ptr(uint32_t offset = 0){ return PSRAMptr + offset; };

    /*
     *  Get the size of the PSRAM memory
    */
    uint32_t get_mem_size(){ return occupiedPSRAM; };

    uint32_t availablePSRAM;
    uint32_t allocatedPSRAM = 0;
};

class PSRAMStream : public Stream {
public:
    PSRAMStream(char* buffer) : buffer(buffer), position(0) {}

    size_t write(uint8_t data) {
        buffer[position++] = data;
        return 1;
    }

    int available() { return 0; }
    int read() { return 0; }
    int peek() { return 0; }
    void flush() {}

private:
    char* buffer;
    size_t position;
};