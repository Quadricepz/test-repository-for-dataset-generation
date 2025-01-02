#include "setup.h"
#include "TCS34725_utils.h"
#include "pico/stdlib.h"

tcs34725 rgb_sensor;


void setup() {
    stdio_init_all();
    rgb_sensor.begin();
}