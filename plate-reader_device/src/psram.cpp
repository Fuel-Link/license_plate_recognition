#include "psram.h"

bool PSRAMHandler::test(){
    if (psramInit()) {
        availablePSRAM = ESP.getFreePsram();
        Serial.printf("PSRAM is available with: %d bytes", availablePSRAM);
        Serial.println();
        return true;
    } else {
        Serial.println("PSRAM not available");
        return false;
    }
}

bool PSRAMHandler::allocate(uint32_t amount){
    if(!test())
        return false;
    if(amount > availablePSRAM){
        Serial.println("Not enough PSRAM");
        return false;
    }

    Serial.printf("Allocating %d bytes in PSRAM... ", amount);
    PSRAMptr = (uint8_t *) ps_malloc(amount * sizeof(uint8_t));

    if(PSRAMptr == nullptr){
        Serial.println("Error: Problem occurred allocating PSRAM");
        return false;
    }
    allocatedPSRAM = amount * sizeof(uint8_t);
    availablePSRAM = ESP.getFreePsram();
    Serial.println("Done");
    Serial.println((String)"PSRAM Size available: " + availablePSRAM);
    Serial.print("PSRAM array bytes allocated: ");
    Serial.println(allocatedPSRAM);
    Serial.println();
    return true;
}

bool PSRAMHandler::destroy(){
    if(allocatedPSRAM == 0) // nothing to destroy
        return true;

    Serial.printf("Destroying PSRAM array... ");
    free(PSRAMptr);

    Serial.println("Done");
    allocatedPSRAM = 0;
    return true;
}

bool PSRAMHandler::store(uint8_t *data, uint32_t size){
    if(allocatedPSRAM == 0){
        Serial.println("Error: PSRAM not allocated");
        return false;
    }
    if ((occupiedPSRAM + size) > allocatedPSRAM) {
        Serial.println("Error: Not enough space in the allocated PSRAM");
        return false;
    }
    
    memcpy(PSRAMptr + occupiedPSRAM, data, size);

    occupiedPSRAM += size;

    return true;
}

bool PSRAMHandler::reset(){
    if(allocatedPSRAM == 0){
        Serial.println("Error: PSRAM not allocated");
        return false;
    }
    occupiedPSRAM = 0;
    return true;
}