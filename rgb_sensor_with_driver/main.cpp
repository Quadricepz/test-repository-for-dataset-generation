#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "Adafruit_TCS34725.h"
#include <iostream>

// #define DEBUG 


#define MAX_SAMPLES 900

// Magic numbers for the TCS34725 from the DN40 application note
#define TCS34725_R_Coef 0.136
#define TCS34725_G_Coef 1.000
#define TCS34725_B_Coef -0.444
#define TCS34725_GA 1.0
#define TCS34725_DF 310.0
#define TCS34725_CT_Coef 3810.0
#define TCS34725_CT_Offset 1391.0

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
    cratio = static_cast<float>(ir) / static_cast<float>(c);

    saturation = ((256 - atime) > 63) ? 65535 : 1024 * (256 - atime);
    saturation75 = (atime_ms < 150) ? (saturation - saturation / 4) : saturation;
    isSaturated = (atime_ms < 150 && c > saturation75) ? true : false;
    cpl = (atime_ms * againx) / (TCS34725_GA * TCS34725_DF);
    maxlux = 65535 / (cpl * 3);

    lux = (TCS34725_R_Coef * static_cast<float>(r_comp) + TCS34725_G_Coef * static_cast<float>(g_comp) + TCS34725_B_Coef * static_cast<float>(b_comp)) / cpl;
    ct = TCS34725_CT_Coef * static_cast<float>(b_comp) / static_cast<float>(r_comp) + TCS34725_CT_Offset;
}

tcs34725 rgb_sensor;

void setup() {
    stdio_init_all();
    rgb_sensor.begin();
}

int main() {
    setup();
    
    while (true)
    {
        int counter = 0;
        while (!stdio_usb_connected())
        {
            sleep_ms(200);
        }

        // print the head of CSV
        std::printf("Red,Green,Blue\n");

        while (stdio_usb_connected())
        {
            if (counter < MAX_SAMPLES)
            {
                rgb_sensor.getData();

                #ifdef DEBUG
                // std::printf("%dx,%dms,0x%x,%u,%u,%u,%u,%u,%.2f,%u,%u,%s,%.2f,%.2f,%u,%u,%u,%u,%.2f,%.2fK\n", 
                //             rgb_sensor.againx, rgb_sensor.atime_ms, rgb_sensor.atime, 
                //             rgb_sensor.r, rgb_sensor.g, rgb_sensor.b, rgb_sensor.c, 
                //             rgb_sensor.ir, rgb_sensor.cratio, rgb_sensor.saturation, 
                //             rgb_sensor.saturation75, rgb_sensor.isSaturated ? "*SATURATED*" : "", 
                //             rgb_sensor.cpl, rgb_sensor.maxlux, rgb_sensor.r_comp, 
                //             rgb_sensor.g_comp, rgb_sensor.b_comp, rgb_sensor.c_comp, 
                //             rgb_sensor.lux, rgb_sensor.ct);
                std::printf("Gain: %dx Time: %dms (0x%x)\n", rgb_sensor.againx, rgb_sensor.atime_ms, rgb_sensor.atime);
                std::printf("Raw R: %u G: %u B: %u C: %u\n", rgb_sensor.r, rgb_sensor.g, rgb_sensor.b, rgb_sensor.c);
                std::printf("IR: %u CRATIO: %.2f Sat: %u Sat75: %u %s\n", rgb_sensor.ir, rgb_sensor.cratio, rgb_sensor.saturation, rgb_sensor.saturation75, rgb_sensor.isSaturated ? "*SATURATED*" : "");
                std::printf("CPL: %.2f Max lux: %.2f\n", rgb_sensor.cpl, rgb_sensor.maxlux);
                std::printf("Compensated R: %u G: %u B: %u C: %u\n", rgb_sensor.r_comp, rgb_sensor.g_comp, rgb_sensor.b_comp, rgb_sensor.c_comp);
                std::printf("Lux: %.2f CT: %.2fK\n\n", rgb_sensor.lux, rgb_sensor.ct);
                #endif
                #ifndef DEBUG
                std::printf("%u,%u,%u\n", rgb_sensor.r_comp, rgb_sensor.g_comp, rgb_sensor.b_comp);
                #endif
                counter++;
                sleep_ms(200);
            } else {

                std::printf("################################## THIS IS THE END. COPY ALL THE ABOVE! ###################################\n");
                while (stdio_usb_connected())
                {
                    // wait until USB connection is reconnected
                    sleep_ms(100); 
                }

            }
            
           
        }
        
        
    }

    return 0;
    
}
