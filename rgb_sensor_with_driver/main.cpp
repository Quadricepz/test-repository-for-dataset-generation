#include "pico/stdlib.h"
#include "TCS34725_utils.h"
#include <iostream>
#include "setup.h"

// #define DEBUG 


const uint16_t MAX_SAMPLES = 900;

extern tcs34725 rgb_sensor;


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
                // sleep_ms(200); // delay is already added 
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
