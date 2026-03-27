#ifndef VT100_H
#define VT100_H

#include "Arduino.h"

class VT100 {
public:
    void begin(Serial_* s) { this->s = s; }
    void clearScreen() { s->println("[CLEAR]"); }
    void setCursorPosition(int x, int y) { s->print("[SET CURSOR "); s->print(x); s->print(","); s->print(y); s->println("]"); }
    void print(const char* val) { s->print(val); }
    void print(int val) { s->print(val); }
    void print(double val) { s->print(val); }

private:
    Serial_* s;
};

#endif
