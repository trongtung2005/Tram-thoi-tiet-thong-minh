#ifndef _RTC_HW111_H_
#define _RTC_HW111_H_

#include <mega328p.h>
#include "TWI_Lib2.h"

// Dia chi I2C cua chip tren module HW111 Tiny (Mac dinh la 0x68)
#define HW111_ADDR 0x68

// Cac bien toan cuc luu tru thoi gian de ham main goi
unsigned char rtc_sec, rtc_min, rtc_hour;
unsigned char rtc_day, rtc_date, rtc_month, rtc_year;

// Ham chuyen doi BCD (Nhi phan ma Thap phan) sang Thap phan thong thuong
unsigned char BCD2Dec(unsigned char bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// Ham chuyen doi Thap phan sang BCD (Dung khi cai dat gio)
unsigned char Dec2BCD(unsigned char dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// Ham doc Gio, Phut, Giay tu HW111 Tiny
void HW111_Read_Time() {
    char data[3];
    
    // Vo hieu hoa ngat I2C de doc kieu Polling an toan nhat
    TWI_Interupt_Disable(); 
    
    TWI_Start();
    TWI_SLA_RW(HW111_ADDR, 0);  // Ghi
    TWI_Send_Byte(0x00);        // Tro toi thanh ghi Giay (0x00)
    
    TWI_Start();                // Repeated Start de chuyen sang doc
    TWI_SLA_RW(HW111_ADDR, 1);  // Doc
    
    // Doc 3 byte (Giay, Phut, Gio) roi gui NACK de dung
    TWI_Read_Array(data, 3);
    TWI_Stop();
    
    TWI_Interupt_Enable();
    
    // Giai ma BCD sang so nguyen
    rtc_sec  = BCD2Dec(data[0]);
    rtc_min  = BCD2Dec(data[1]);
    rtc_hour = BCD2Dec(data[2] & 0x3F); // Che do 24h
}

// Ham doc Thu, Ngay, Thang, Nam
void HW111_Read_Date() {
    char data[4];
    
    TWI_Interupt_Disable();
    TWI_Start();
    TWI_SLA_RW(HW111_ADDR, 0);
    TWI_Send_Byte(0x03);        // Tro toi thanh ghi Thu (0x03)
    
    TWI_Start();
    TWI_SLA_RW(HW111_ADDR, 1);
    
    TWI_Read_Array(data, 4);
    TWI_Stop();
    TWI_Interupt_Enable();
    
    rtc_day   = BCD2Dec(data[0]);
    rtc_date  = BCD2Dec(data[1]);
    rtc_month = BCD2Dec(data[2] & 0x7F);
    rtc_year  = BCD2Dec(data[3]);
}

// Ham cai dat thoi gian moi cho HW111 Tiny
void HW111_Set_Time(unsigned char h, unsigned char m, unsigned char s) {
    unsigned char data[4];
    data[0] = 0x00;         // Con tro toi thanh ghi Giay
    data[1] = Dec2BCD(s);   // Giay
    data[2] = Dec2BCD(m);   // Phut
    data[3] = Dec2BCD(h);   // Gio (Che do 24h)
    
    // Ham TWI_Master_Send tu dong tao Start, Stop va gui mang
    TWI_Master_Send(HW111_ADDR, data, 4); 
}

// Ham cai dat Ngay Thang
void HW111_Set_Date(unsigned char day, unsigned char date, unsigned char month, unsigned char year) {
    unsigned char data[5];
    data[0] = 0x03;
    data[1] = Dec2BCD(day);   // Thu (1-7)
    data[2] = Dec2BCD(date);  // Ngay
    data[3] = Dec2BCD(month); // Thang
    data[4] = Dec2BCD(year);  // Nam (0-99)
    
    TWI_Master_Send(HW111_ADDR, data, 5);
}

#endif