#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "Adafruit_TCS34725.h"
#include "TCS34725_utils.h"
#include <iostream>

// Magic numbers for the TCS34725 from the DN40 application note
#define TCS34725_R_Coef 0.136
#define TCS34725_G_Coef 1.000
#define TCS34725_B_Coef -0.444
#define TCS34725_GA 1.0
#define TCS34725_DF 310.0
#define TCS34725_CT_Coef 3810.0
#define TCS34725_CT_Offset 1391.0


const tcs34725::tcs_agc tcs34725::agc_lst[] = {
    { TCS34725_GAIN_60X, TCS34725_INTEGRATIONTIME_700MS,     0, 20000 }, // agc_lst[0]
    { TCS34725_GAIN_60X, TCS34725_INTEGRATIONTIME_154MS,  4990, 63000 },
    { TCS34725_GAIN_16X, TCS34725_INTEGRATIONTIME_154MS, 16790, 63000 },
    { TCS34725_GAIN_4X,  TCS34725_INTEGRATIONTIME_154MS, 15740, 63000 },
    { TCS34725_GAIN_1X,  TCS34725_INTEGRATIONTIME_154MS, 15740, 0 }
};

tcs34725::tcs34725() : agc_cur(1), isAvailable(false), isSaturated(false) {} // direct initialization not function

bool tcs34725::begin() {
    tcs = Adafruit_TCS34725(agc_lst[agc_cur].at, agc_lst[agc_cur].ag);
    if ((isAvailable = tcs.begin())) {
        setGainTime();
    }
    return isAvailable;
}

void tcs34725::setGainTime() {
    tcs.setGain(agc_lst[agc_cur].ag);
    tcs.setIntegrationTime(agc_lst[agc_cur].at);
    atime = static_cast<uint16_t>(agc_lst[agc_cur].at);
    atime_ms = static_cast<uint16_t>((256 - atime) * 2.4);
    switch (agc_lst[agc_cur].ag) {
        case TCS34725_GAIN_1X:
            againx = 1;
            break;
        case TCS34725_GAIN_4X:
            againx = 4;
            break;
        case TCS34725_GAIN_16X:
            againx = 16;
            break;
        case TCS34725_GAIN_60X:
            againx = 60;
            break;
    }
}

void tcs34725::getData() {
    tcs.getRawData(&r, &g, &b, &c);
    while (1) {
        if (agc_lst[agc_cur].maxcnt && c > agc_lst[agc_cur].maxcnt) {
            agc_cur++;
        } else if (agc_lst[agc_cur].mincnt && c < agc_lst[agc_cur].mincnt) {
            agc_cur--;
        } else {
            break;
        }

        setGainTime();
        sleep_ms(static_cast<uint16_t>((256 - atime) * 2.4 * 2)); // shock absorber
        tcs.getRawData(&r, &g, &b, &c);
        break;
    }

    ir = (r + g + b > c) ? (r + g + b - c) / 2 : 0;
    r_comp = r - ir;
    g_comp = g - ir;
    b_comp = b - ir;
    c_comp = c - ir;
    
    #ifdef DEBUG
    cratio = static_cast<float>(ir) / static_cast<float>(c);

    saturation = ((256 - atime) > 63) ? 65535 : 1024 * (256 - atime);
    saturation75 = (atime_ms < 150) ? (saturation - saturation / 4) : saturation;
    isSaturated = (atime_ms < 150 && c > saturation75) ? true : false;
    cpl = (atime_ms * againx) / (TCS34725_GA * TCS34725_DF);
    maxlux = 65535 / (cpl * 3);

    lux = (TCS34725_R_Coef * static_cast<float>(r_comp) + TCS34725_G_Coef * static_cast<float>(g_comp) + TCS34725_B_Coef * static_cast<float>(b_comp)) / cpl;
    ct = TCS34725_CT_Coef * static_cast<float>(b_comp) / static_cast<float>(r_comp) + TCS34725_CT_Offset;
    #endif
}



