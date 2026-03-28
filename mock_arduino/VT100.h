#ifndef VT100_H
#define VT100_H

#include "Arduino.h"

class VT100 {
public:
    void begin(Serial_* s) { this->s = s; }
    void clearScreen() { s->println("[CLEAR]"); }
    void setCursorPosition(int x, int y) { s->print("[SET CURSOR "); s->print(x); s->print(","); s->print(y); s->print("]"); }
    void setForeground(int color) { s->print("[FG "); s->print(color); s->print("]"); }
    void setBackground(int color) { s->print("[BG "); s->print(color); s->print("]"); }
    void setBold(bool bold) { s->print("[BOLD "); s->print(bold ? 1 : 0); s->print("]"); }
    void print(const char* val) { s->print(val); }
    void print(int val) { s->print(val); }
    void print(uint32_t val) { s->print((int)val); }
    void print(double val) { s->print(val); }

    // Constants for colors
    static const int RED = 31;
    static const int GREEN = 32;
    static const int YELLOW = 33;
    static const int BLUE = 34;
    static const int WHITE = 37;

private:
    Serial_* s;
};

#endif
