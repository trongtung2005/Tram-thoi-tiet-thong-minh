#ifndef _DHT11_LITE_H_
#define _DHT11_LITE_H_

#include <mega328p.h>
#include <delay.h>

// Ð?nh nghia chân D5
#define DHT_PIN 5 
#define DHT_DIR DDRD
#define DHT_PORT PORTD
#define DHT_PIN_IN PIND

unsigned char hum = 0, temp = 0;

// Hàm tr? v? 1 n?u thành công, 0 n?u b? l?i ho?c l?ng dây
unsigned char DHT11_Read(void) {
    unsigned char i, j;
    unsigned char data[5] = {0, 0, 0, 0, 0};
    unsigned int timeout = 0;

    // 1. G?i tín hi?u Start
    DHT_DIR |= (1 << DHT_PIN);       // C?u hình Output
    DHT_PORT &= ~(1 << DHT_PIN);     // Kéo xu?ng LOW
    delay_ms(20);                    // Ð?i ít nh?t 18ms
    DHT_PORT |= (1 << DHT_PIN);      // Kéo lên HIGH
    delay_us(30);                    // Ð?i 20-40us
    DHT_DIR &= ~(1 << DHT_PIN);      // Chuy?n sang Input d? d?c

    // 2. Ch? DHT11 ph?n h?i (Kéo xu?ng LOW)
    timeout = 0;
    while(DHT_PIN_IN & (1 << DHT_PIN)) {
        timeout++; delay_us(1);
        if(timeout > 100) return 0;  // L?i: Không th?y ph?n h?i (L?ng dây)
    }

    // Ch? DHT11 kéo lên HIGH
    timeout = 0;
    while(!(DHT_PIN_IN & (1 << DHT_PIN))) {
        timeout++; delay_us(1);
        if(timeout > 100) return 0;  // L?i: C?m bi?n b? treo
    }

    // Ch? DHT11 kéo xu?ng LOW d? b?t d?u g?i d? li?u
    timeout = 0;
    while(DHT_PIN_IN & (1 << DHT_PIN)) {
        timeout++; delay_us(1);
        if(timeout > 100) return 0;  // L?i: Quá th?i gian ch?
    }

    // 3. Ð?c 40 bit (5 byte) d? li?u
    for(i = 0; i < 5; i++) {
        for(j = 0; j < 8; j++) {
            // Ch? ph?n LOW k?t thúc
            timeout = 0;
            while(!(DHT_PIN_IN & (1 << DHT_PIN))) {
                timeout++; delay_us(1);
                if(timeout > 100) return 0; 
            }

            // Ðo d? dài c?a HIGH d? xác d?nh bit 0 hay 1
            delay_us(40); // Ch? 40us. N?u v?n HIGH thì là bit 1, n?u dã LOW thì là bit 0
            
            if(DHT_PIN_IN & (1 << DHT_PIN)) {
                data[i] |= (1 << (7 - j)); // Ðánh d?u là bit 1
                
                // Ch? n?t ph?n HIGH c?a bit 1 k?t thúc
                timeout = 0;
                while(DHT_PIN_IN & (1 << DHT_PIN)) {
                    timeout++; delay_us(1);
                    if(timeout > 100) return 0;
                }
            }
        }
    }

    // 4. Ki?m tra mã l?i (Checksum) d? d?m b?o tính chính xác
    if((unsigned char)(data[0] + data[1] + data[2] + data[3]) == data[4]) {
        hum = data[0];
        temp = data[2];
        return 1; // Ð?c thành công
    }
    
    return 0; // L?i: D? li?u b? nhi?u sai Checksum
}
#endif