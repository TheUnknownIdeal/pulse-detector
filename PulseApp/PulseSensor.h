#ifndef PULSE_SENSOR_H
#define PULSE_SENSOR_H

#include <Arduino.h> // Gives you access to byte, HIGH, LOW, etc.
#include <Wire.h>


class myMAX30102 {
    public:

        myMAX30102(TwoWire* i2cport, uint8_t address);

        void setupSensor();

        void writeRegister(uint8_t reg, uint8_t value);
        uint8_t readRegister(uint8_t reg);

        void readFIFO(uint32_t &red, uint32_t &ir);

        void fullRead(uint32_t& redAvg, uint32_t& irAvg, uint8_t&n);

        void readTemp(float& temp);

        // An ir value below this threshold is considered a bad sample (no finger)
        static constexpr uint32_t threshold = 50000;

    private:

        uint8_t _address; // Device addres
        TwoWire* _i2cPort; // The arduino I2C port

        // FIFO Registers
        static const uint8_t _fifo_data = 0x07;
        static const uint8_t _fifo_wr_ptr = 0x04;
        static const uint8_t _fifo_rd_ptr = 0x06;

        static const uint8_t _ovf_counter = 0x05;

        // Interrupt registers
        static const uint8_t _intr_status_1 = 0x00;
        static const uint8_t _intr_status_2 = 0x01;

        // config registers
        static const uint8_t _fifo_config = 0x08;
        static const uint8_t _mode_config = 0x09;
        static const uint8_t _spo2_config = 0x0A;
        static const uint8_t _led1_pa = 0x0C; // red LED current
        static const uint8_t _led2_pa = 0x0D; // ir LED current

        // registers for temperature
        static const uint8_t _temp_int = 0x1F; // integer part of temperature reading
        static const uint8_t _temp_frac = 0x20; // fraction part
        static const uint8_t _temp_config = 0x21;


        void _fullFIFO();

        uint8_t _nsamples;

        
        static constexpr uint8_t _buffer_size = 1; // Use this to determine how many buffers you want

        uint32_t _red[_buffer_size];
        uint32_t _ir[_buffer_size];

};



#endif