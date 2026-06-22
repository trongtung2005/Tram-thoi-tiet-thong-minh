#ifndef _ADCLIB_H_
#define _ADCLIB_H_

#include <mega328p.h>
#include <delay.h>

// Dien ap tham chieu (AVCC = 5V, can le phai)
#define ADC_VREF_TYPE ((0<<REFS1) | (1<<REFS0) | (0<<ADLAR))

// Ham khoi tao chi can chay 1 lan o setup
void ADC_init(void)
{
    // Bat ADC, chon Prescaler = 128 de co ADC clock = 125 kHz
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    
    // Cho mach ADC on dinh
    delay_ms(10);
}

// Doc gia tri tu kenh bat ky (0-7)
unsigned int ADC_read(unsigned char input)
{
    // Gioi han kenh dau vao tu 0 den 7 de tranh loi bo nho
    input = input & 0x07;
    
    // Chon dien ap tham chieu va kenh ADC
    ADMUX = ADC_VREF_TYPE | input;
    
    // Tat chuc nang Digital Input cua chan dang doc de giam nhieu
    DIDR0 |= (1 << input); 
    
    // Bat dau qua trinh chuyen doi ADC
    ADCSRA |= (1<<ADSC);
    
    // Cho den khi qua trinh chuyen doi hoan tat (Co ADIF bat len 1)
    while ((ADCSRA & (1<<ADIF)) == 0);
    
    // Xoa co bao hoan tat bang cach ghi so 1
    ADCSRA |= (1<<ADIF);
    
    return ADCW;  
}

#endif