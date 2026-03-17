#ifndef MAX30102_H
#define MAX30102_H

#include <Arduino.h> // Gives you access to byte, HIGH, LOW, etc.
#include <Wire.h>


class MAX30102 {
    public:

    private:
        static const byte fifo_data = 0x07;
        

};



Define

REG_MODE_CONFIG = 0x09
REG_FIFO_DATA   = 0x07
REG_SPO2_CONFIG = 0x0A

#endif