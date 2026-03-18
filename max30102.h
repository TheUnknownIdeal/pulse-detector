#ifndef MAX30102_H
#define MAX30102_H

#include <Arduino.h> // Gives you access to byte, HIGH, LOW, etc.
#include <Wire.h>


class MAX30102 {
    public:

        MAX30102();

        void setupSensor();

        void writeRegister();
        uint8_t readRegister();

        void fullRead();




    private:

        uint8_t _address; // Device addres
        TwoWire* _ic2port; // The arduino I2C port

        // FIFO Registers
        static const uint8_t _fifo_data = 0x07;
        static const uint8_t _fifo_wr_ptr = 0x04;
        static const uint8_t _fifo_rd_ptr = 0x06;

        static const uint8_t _ovf_counter = 0x05;

        // Interrupt registers
        static const uint8_t _intr_status_1 = 0x00;
        static const uint8_t _intr_status_2 = 0x01;

        // config regitsters
        static const uint8_t _fifo_config = 0x08;
        static const uint8_t _mode_config = 0x0;
        static const uint8_t _spo2_config = 0x0A;
        static const uint8_t _led1_pa = 0x0C; // red LED current
        static const uint8_t _led2_pa = 0x0D; // ir LED current



        void _fullFIFO();

        uint8_t _nsamples;

        uint32_t _red[32];
        uint32_t _ir[32];




        

};



#endif