#include "EEPROM.h"
#include <fstream>

EEPROM_ EEPROM;

void EEPROM_::load() {
    std::ifstream f("mock_eeprom.bin", std::ios::binary);
    if (f) {
        f.read((char*)memory, 512);
    }
}

void EEPROM_::save() {
    std::ofstream f("mock_eeprom.bin", std::ios::binary);
    if (f) {
        f.write((char*)memory, 512);
    }
}
