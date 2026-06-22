#ifndef _WIND_DIR_H_
#define _WIND_DIR_H_

#include <mega328p.h>

// Khai bao cac chuoi truc tiep vao bo nho RAM (SRAM) de an toan tuyet doi cho con tro
char dir_N[]  = "N  (Bac)";
char dir_E[]  = "E  (Dong)";
char dir_S[]  = "S  (Nam)";
char dir_W[]  = "W  (Tay)";
char dir_NE[] = "NE (Dong Bac)";
char dir_SE[] = "SE (Dong Nam)";
char dir_SW[] = "SW (Tay Nam)";
char dir_NW[] = "NW (Tay Bac)";
char dir_err[]= "Dang xac dinh...";

// --- BIEN NHO LUU TRANG THAI ---
// Luu lai huong gio hop le cuoi cung. Mac dinh ban dau la "Dang xac dinh..."
char* last_valid_dir = dir_err;

// Ham khoi tao cac chan doc huong gio
void Wind_Dir_Init(void)
{
    // 1. Cau hinh cac chan PC0, PC1, PC2, PC3 la ngo vao (Input)
    DDRC &= ~((1 << DDC0) | (1 << DDC1) | (1 << DDC2) | (1 << DDC3));
    
    // 2. Bat dien tro keo len (Pull-up Resistor) cho cac chan nay
    PORTC |= (1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC3);
    
    // Reset lai bien nho khi he thong khoi dong
    last_valid_dir = dir_err; 
}

// Ham doc va tra ve con tro chua chuoi ky tu chi huong gio
char* Wind_Dir_Read(void)
{
    // Doc trang thai ca port C vao 1 bien tam de xu ly cho dong bo va nhanh
    unsigned char pins = PINC;

    // QUAN TRONG: Phai kiem tra cac huong LAI (2 chan cung LOW) TRUOC
    if (!(pins & (1 << PINC0)) && !(pins & (1 << PINC1))) {
        last_valid_dir = dir_NE;
    }
    else if (!(pins & (1 << PINC2)) && !(pins & (1 << PINC1))) {
        last_valid_dir = dir_SE;
    }
    else if (!(pins & (1 << PINC2)) && !(pins & (1 << PINC3))) {
        last_valid_dir = dir_SW;
    }
    else if (!(pins & (1 << PINC0)) && !(pins & (1 << PINC3))) {
        last_valid_dir = dir_NW;
    }
    
    // Sau do moi kiem tra cac huong CHINH (Chi 1 chan LOW)
    else if (!(pins & (1 << PINC0))) {
        last_valid_dir = dir_N;
    }
    else if (!(pins & (1 << PINC1))) {
        last_valid_dir = dir_E;
    }
    else if (!(pins & (1 << PINC2))) {
        last_valid_dir = dir_S;
    }
    else if (!(pins & (1 << PINC3))) {
        last_valid_dir = dir_W;
    }

    // NEU TAT CA CAC CHAN DEU HIGH (Nam cham nam o diem mu giua cac cam bien):
    // Vi dieu khien se bo qua toan bo khoi if-else o tren, 
    // khong thay doi last_valid_dir, va cu the tra ve gia tri cu.

    return last_valid_dir;
}

#endif