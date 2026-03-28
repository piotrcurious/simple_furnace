#ifndef TICKER_H
#define TICKER_H

#include <stdint.h>
#include <functional>
#include <vector>

class Ticker {
public:
    typedef void (*callback_t)(void);

    void attach(float seconds, callback_t callback) {
        attach_ms(seconds * 1000, callback);
    }

    void attach_ms(uint32_t ms, callback_t callback) {
        this->ms = ms;
        this->callback = callback;
        this->last_run = 0; // Or current millis()
    }

    void detach() {
        this->callback = nullptr;
    }

    void update(uint32_t current_ms) {
        if (callback && (current_ms - last_run >= ms)) {
            callback();
            last_run = current_ms;
        }
    }

private:
    uint32_t ms;
    callback_t callback = nullptr;
    uint32_t last_run = 0;
};

#endif
