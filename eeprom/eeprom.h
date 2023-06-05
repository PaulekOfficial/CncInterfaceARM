//
// Created by pawel on 03/06/2023.
//

#ifndef INTERFACE_PICO_EEPROM_H
#define INTERFACE_PICO_EEPROM_H


#include <cstddef>
#include <cstdio>
#include <hardware/i2c.h>
#include <string>

#define NUMBER_OF_BLOCKS 128

class eeprom {
private:
    uint8_t address;
    i2c_inst_t *i2c_inst;

public:
    eeprom(uint8_t address_, i2c_inst_t *i2CInst_);

    void write(unsigned int address_, uint8_t data);

    int writeString(unsigned int address_, std::string data_);

    void writeBoolean(unsigned int address_, bool value);

    bool readBoolean(unsigned int address_);

    std::string readString(unsigned int address_);

    std::string readString(unsigned int address_, uint size_);

    uint8_t read(unsigned int address_);

    uint8_t* read(unsigned int address_, uint size_);
};


#endif //INTERFACE_PICO_EEPROM_H
