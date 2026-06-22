#ifndef _RAIN_SENSOR_H_
#define _RAIN_SENSOR_H_

#include <mega328p.h>

// Dinh nghia chan cam bien mua cam vao D6
#define RAIN_PIN 6

void Rain_Sensor_Init(void) {
    // Thiet lap chan D6 la Input
    DDRD &= ~(1 << RAIN_PIN);
    // Bat tro keo len (Pull-up)
    PORTD |= (1 << RAIN_PIN);
}

// Ham tra ve 1 neu dang mua, tra ve 0 neu khong mua
unsigned char Rain_Check(void) {
    // Cam bien MH-RD thuong tra ve muc LOW khi co nuoc
    if (!(PIND & (1 << RAIN_PIN))) {
        return 1; 
    }
    return 0;
}

#endif