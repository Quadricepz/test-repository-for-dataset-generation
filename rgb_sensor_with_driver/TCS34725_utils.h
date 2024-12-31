#ifndef TCS34725_UTILS_H
#define TCS34725_UTILS_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "Adafruit_TCS34725.h"
#include <iostream>

class tcs34725 {
public:
    tcs34725();
    bool begin();
    void getData();

    bool isAvailable, isSaturated;
    uint16_t againx, atime, atime_ms;
    uint16_t r, g, b, c;
    uint16_t ir;
    uint16_t r_comp, g_comp, b_comp, c_comp;
    uint16_t saturation, saturation75;
    float cratio, cpl, ct, lux, maxlux;

private:
    struct tcs_agc {
        tcs34725Gain_t ag;
        tcs34725IntegrationTime_t at;
        uint16_t mincnt;
        uint16_t maxcnt;
    };
    static const tcs_agc agc_lst[];
    uint16_t agc_cur;

    void setGainTime();
    Adafruit_TCS34725 tcs;
};

#endif // TCS34725_UTILS_H