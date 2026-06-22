#ifndef _HALL_SPEED_H_
#define _HALL_SPEED_H_

#include <mega328p.h>

// --- CAC THONG SO CO KHI CUA BANH XE DO GIO ---
#define PULSES_PER_REV  4      // So xung tren 1 vong quay (Do ban dung 4 cam bien Hall)
#define WHEEL_RADIUS    0.09   // Ban kinh cua banh xe do gio (Don vi: met). 
#define CALIB_FACTOR    1.18   // He so bu tru ma sat cua truc quay
#define PI              3.14159

// --- BIEN TOAN CUC LUU TRU KET QUA ---
volatile unsigned int wind_pulse_count = 0;
volatile unsigned int wind_rpm = 0;
volatile float wind_speed_ms = 0.0;

// Bien luu trang thai chan de loc suon xuong cho ngat PCINT
unsigned char last_wind_state = 0; 

// Ham khoi tao Ngat PCINT0 va Timer1
void Hall_Speed_Init(void)
{
    // 1. CAU HINH NGAT PCINT CHO CHAN D8 (Tuong ung PB0)
    // C?u hình PB0 là Input
    DDRB &= ~(1 << 0);
    // B?t di?n tr? kéo lên n?i cho PB0
    PORTB |= (1 << 0);

    // Kích ho?t ng?t Pin Change cho nhóm PORTB (PCIE0)
    PCICR |= (1 << 0); 
    // Kích ho?t m?t n? ng?t riêng cho chân PB0 (PCINT0)
    PCMSK0 |= (1 << 0);

    // Luu tr?ng thái ban d?u c?a chân D8 d? làm m?c so sánh
    last_wind_state = PINB & (1 << 0);


    // 2. CAU HINH TIMER1 (16-bit) TAO CHU KY 1 GIAY (Gi? nguyên)
    TCCR1A = 0x00; 
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
    
    OCR1AH = (15624 >> 8);
    OCR1AL = (15624 & 0xFF);
    
    TCNT1H = 0;
    TCNT1L = 0;
    
    TIMSK1 |= (1 << OCIE1A);
    
    #asm("sei")
}

// =======================================================
// TRINH PHUC VU NGAT (ISR) 
// =======================================================

// ISR 1: Ngat PCINT0 (Chay khi co su thay doi logic tren Port B)
interrupt [PC_INT0] void pin_change_isr(void)
{
    // Ð?c tr?ng thái logic hi?n t?i c?a chân D8 (PB0)
    unsigned char current_state = PINB & (1 << 0); 
    
    // Logic l?c su?n xu?ng: Ch? tang d?m khi chân chuy?n t? HIGH (1) xu?ng LOW (0)
    if (!current_state && last_wind_state) {
        wind_pulse_count++;
    }
    
    // C?p nh?t l?i tr?ng thái cho l?n ng?t ti?p theo
    last_wind_state = current_state;
}

// ISR 2: Ngat Timer1 (Chay chuan xac moi 1 giay)
interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{
    // 1. Tinh so vong quay tren phut (RPM)
    wind_rpm = (wind_pulse_count * 60) / PULSES_PER_REV;
    
    // 2. Tinh van toc gio (m/s) 
    wind_speed_ms = (((float)wind_rpm * 2.0 * PI * WHEEL_RADIUS) / 60.0) * CALIB_FACTOR * 13.0;
    
    // 3. Reset bien dem 
    wind_pulse_count = 0;
}

#endif