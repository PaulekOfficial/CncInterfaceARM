//
// Created by pawel on 03/06/2023.
//

#include <sstream>
#include "eeprom.h"

void eeprom::write(unsigned int address_, uint8_t data) {
    uint8_t d[2]= {(uint8_t) address_, data};

    bool successfullyWriten = false;

    while (!successfullyWriten) {

        switch (i2c_write_blocking(i2c_inst, address, d, sizeof(d) / sizeof(uint8_t), false)) {
            case PICO_ERROR_GENERIC:
                printf("[EEPROM] addr not acknowledged!\n");
                break;
            case PICO_ERROR_TIMEOUT:
                printf("[EEPROM] timeout!\n");
                break;
            default:
                //printf("[EEPROM] wrote successfully %lu bytes!\n", 2);
                break;
        }

        // Writing in I2C EEPROM takes ~5ms (even if I2C writing already done)
        busy_wait_ms(5);

        // Verify
        uint8_t verify = read(address_);
        if (verify == data) {
            successfullyWriten = true;
        }
    }
}

uint8_t eeprom::read(unsigned int address_) {
    uint8_t readData = 0xFF;

    while (readData == 0xFF) {
        uint8_t d[1] = {(uint8_t) address_};

        switch (i2c_write_blocking(i2c_inst, address, d, sizeof(d) / sizeof(uint8_t), true)) {
            case PICO_ERROR_GENERIC:
                printf("[EEPROM] addr not acknowledged!\n");
            case PICO_ERROR_TIMEOUT:
                printf("[EEPROM] timeout!\n");
                break;
            default:
                //printf("[EEPROM] wrote successfully %lu bytes!\n", sizeof d);
                break;
        }
        busy_wait_ms(100);

        i2c_read_blocking(i2c_inst, address, &readData, 1, false);
        busy_wait_ms(100);
    }

    return readData;
}

eeprom::eeprom(uint8_t address_, i2c_inst_t *i2CInst_) {
    address = address_;
    i2c_inst = i2CInst_;
}

int eeprom::writeString(unsigned int address_, std::string data_) {
    for (int i = 0; i <= data_.size(); ++i) {
        eeprom::write(address_ + i, data_[i]);
    }

    // verify write
//    auto str = readString(address_, address_ + data_.size() + 1);
//    if (str != data_) {
//        return writeString(address_, data_);
//    }

    return address_ + data_.size() + 1;
}

std::string eeprom::readString(unsigned int address_) {
    std::stringstream stream;

    uint counter = 0;
    while (true) {
        uint8_t i = read(address_ + counter);
//        if (i == 0xFF) {
//            return readString(address_);
//        }
        stream << (char) i;

        if (i == '\0') {
            break;
        }

        counter++;
    }
    read(address_);

    return stream.str();
}

std::string eeprom::readString(unsigned int address_, uint size_) {
    std::stringstream stream;

    uint counter = 0;
    while (counter < size_) {
        uint8_t i = read(address_);
//        if (i == 0xFF) {
//            return readString(address_, size_);
//        }
        stream << (char) i;

        counter++;
    }
    read(address_);

    return stream.str();
}

void eeprom::writeBoolean(unsigned int address_, bool value) {
    write(address_, value ? 0x01 : 0x00);
}

bool eeprom::readBoolean(unsigned int address_) {
    uint8_t value = read(address_);
    return value == 0x01;
}

uint8_t *eeprom::read(unsigned int address_, uint size_) {
    uint8_t *readData = nullptr;

    uint8_t d[1] = {(uint8_t) address_};

    switch (i2c_write_blocking(i2c_inst, address, d, sizeof(d) / sizeof(uint8_t), true)) {
        case PICO_ERROR_GENERIC:
            printf("[EEPROM] addr not acknowledged!\n");
        case PICO_ERROR_TIMEOUT:
            printf("[EEPROM] timeout!\n");
            break;
        default:
            //printf("[EEPROM] wrote successfully %lu bytes!\n", sizeof d);
            break;
    }
    busy_wait_ms(5);

    i2c_read_blocking(i2c_inst, address, readData, size_, false);

    return readData;
}


