#ifndef VT100_VISUALIZER_H
#define VT100_VISUALIZER_H

#include "VT100.h"

class VT100Visualizer {
public:
    VT100Visualizer(VT100& vt) : vt100(vt) {}

    void drawHeader(const char* title) {
        vt100.setCursorPosition(2, 1);
        vt100.setBold(true);
        vt100.setForeground(VT100::BLUE);
        vt100.print(">>> ");
        vt100.setForeground(VT100::WHITE);
        vt100.print(title);
        vt100.setForeground(VT100::BLUE);
        vt100.print(" <<<");
        vt100.setBold(false);
    }

    void drawProgressBar(int x, int y, int width, float percentage, int color) {
        vt100.setCursorPosition(x, y);
        vt100.setForeground(VT100::WHITE);
        vt100.print("[");
        int filled = (int)(width * percentage / 100.0);
        vt100.setForeground(color);
        for (int i = 0; i < width; i++) {
            if (i < filled) vt100.print("█");
            else vt100.print("░");
        }
        vt100.setForeground(VT100::WHITE);
        vt100.print("]");
    }

    void drawFan(int x, int y, int state, int rpm) {
        const char frames[] = {'-', '\\', '|', '/'};
        vt100.setCursorPosition(x, y);
        vt100.setForeground(rpm > 0 ? VT100::GREEN : VT100::RED);
        vt100.setBold(true);
        if (rpm > 0) {
            vt100.print("(");
            vt100.print(frames[state % 4]);
            vt100.print(")");
        } else {
            vt100.print("(X)");
        }
        vt100.setBold(false);
    }

    void drawFlame(int x, int y, int level, int tick) {
        vt100.setCursorPosition(x, y);
        if (level < 100) {
            vt100.setForeground(VT100::BLUE);
            vt100.print("  .  ");
            return;
        }

        vt100.setForeground(tick % 2 == 0 ? VT100::RED : VT100::YELLOW);
        if (level > 500) {
            vt100.print(" (W) ");
        } else {
            vt100.print("  v  ");
        }
    }

    void drawBorder(int x1, int y1, int x2, int y2, int color) {
        vt100.setForeground(color);
        for (int x = x1; x <= x2; x++) {
            vt100.setCursorPosition(x, y1); vt100.print("=");
            vt100.setCursorPosition(x, y2); vt100.print("=");
        }
        for (int y = y1; y <= y2; y++) {
            vt100.setCursorPosition(x1, y); vt100.print("|");
            vt100.setCursorPosition(x2, y); vt100.print("|");
        }
        vt100.setCursorPosition(x1, y1); vt100.print("+");
        vt100.setCursorPosition(x2, y1); vt100.print("+");
        vt100.setCursorPosition(x1, y2); vt100.print("+");
        vt100.setCursorPosition(x2, y2); vt100.print("+");
    }

    void drawGraph(int x, int y, int w, int h, int* data, int dataSize, int maxVal, int color) {
        vt100.setForeground(color);
        for (int i = 0; i < w && i < dataSize; i++) {
            int val = (int)(h * data[i] / (float)maxVal);
            if (val < 0) val = 0;
            if (val > h) val = h;
            for (int j = 1; j <= h; j++) {
                vt100.setCursorPosition(x + i, y + h - j + 1);
                if (j <= val) vt100.print("#"); // Renderer will map to block
                else vt100.print(" ");
            }
        }
    }

    void drawScrubberArt(int x, int y, int fanRpm, int pumpDuty) {
        vt100.setForeground(VT100::WHITE);
        vt100.setCursorPosition(x, y);   vt100.print("  [EXHAUST]  ");
        vt100.setCursorPosition(x, y+1); vt100.print("      v      ");
        vt100.setForeground(VT100::BLUE);
        vt100.setCursorPosition(x, y+2); vt100.print("  |~~~~~~~|  ");
        vt100.setCursorPosition(x, y+3); vt100.print("  |   S   |  ");
        vt100.setCursorPosition(x, y+4); vt100.print("  |_______|  ");

        // Fluid flow indicators
        int tick = (millis() / 200) % 4;
        vt100.setCursorPosition(x - 2, y + 3); vt100.print(tick == 0 ? "> " : ">>");
        vt100.setCursorPosition(x + 11, y + 3); vt100.print(tick == 2 ? "> " : ">>");

        drawFan(x + 4, y + 5, (int)(millis() / 100), fanRpm);

        vt100.setForeground(VT100::WHITE);
        vt100.setCursorPosition(x + 13, y + 3);
        vt100.print("PUMP:");
        const char pumpFrames[] = {'-', '/', '|', '\\'};
        vt100.setForeground(pumpDuty > 0 ? VT100::GREEN : VT100::RED);
        vt100.print(pumpFrames[tick]);
    }

    void drawFurnaceArt(int x, int y, int heat) {
        vt100.setForeground(VT100::WHITE);
        vt100.setCursorPosition(x, y);   vt100.print("  _______  ");
        vt100.setCursorPosition(x, y+1); vt100.print(" |       | ");
        vt100.setCursorPosition(x, y+2); vt100.print(" | [   ] | ");
        vt100.setCursorPosition(x, y+3); vt100.print(" |_______| ");

        // Flame in the middle
        int tick = (int)(millis() / 200);
        drawFlame(x + 4, y + 2, heat, tick);
    }

private:
    VT100& vt100;
};

#endif
