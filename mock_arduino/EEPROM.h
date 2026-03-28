#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>
#include <vector>

class EEPROM_ {
public:
    EEPROM_() {
        load();
    }
    uint8_t read(int addr) { return memory[addr]; }
    void write(int addr, uint8_t val) {
        memory[addr] = val;
        save();
    }

    template<typename T>
    void get(int addr, T& val) {
        memcpy(&val, &memory[addr], sizeof(T));
    }

    template<typename T>
    void put(int addr, const T& val) {
        memcpy(&memory[addr], &val, sizeof(T));
        save();
    }

    void load();
    void save();

private:
    uint8_t memory[512] = {0};
};

extern EEPROM_ EEPROM;

#endif
