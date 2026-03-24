#include "PulseSensor.h"
#include <Wire.h>

myMAX30102::myMAX30102(TwoWire* i2cport, uint8_t address) : _ir{0}, _red{0} {
    _i2cPort = i2cport;  // The pointer starts pointing at "nothing"
    _address = address;     // The standard address for this chip
    _nsamples = 0;

}

void myMAX30102::setupSensor() {

    // Reset
    writeRegister(_mode_config, 0x40);
    delay(100);

    // Clear FIFO pointers
    writeRegister(_fifo_wr_ptr, 0x00);
    writeRegister(_ovf_counter, 0x00);
    writeRegister(_fifo_rd_ptr, 0x00);

    // FIFO config:
    // sample average = 1, FIFO rollover disabled, almost full = 0
    writeRegister(_fifo_config, 0x0F);

    // SpO2 mode = 0x03 (enables RED + IR)
    writeRegister(_mode_config, 0x03);

    writeRegister(_intr_status_2, 0x02); // enable die temp interrupt
    writeRegister(_intr_status_1, 0x00); // disable all other interrupts



    // SPO2 config:
    // ADC range = 4096 nA (01)
    // sample rate = 100 Hz (001)
    // pulse width = 411 us / 18-bit (11)
    // binary: 01 001 11 = 0x27

    writeRegister(_spo2_config, 0b0101011);

    // LED pulse amplitudes
    //writeRegister(_led1_pa, 0x24); // RED current
    writeRegister(_led1_pa, 0b00101000); // RED current
    writeRegister(_led2_pa, 0x24); // IR current

    // Read interrupt status registers once to clear them
    readRegister(_intr_status_1);
    readRegister(_intr_status_2);

}

void myMAX30102::writeRegister(uint8_t reg, uint8_t value) {
    _i2cPort->beginTransmission(_address);
    _i2cPort->write(reg);
    _i2cPort->write(value);
    _i2cPort->endTransmission();
}

uint8_t myMAX30102::readRegister(uint8_t reg) {
    _i2cPort->beginTransmission(_address);
    _i2cPort->write(reg);
    _i2cPort->endTransmission(false); // repeated start

    _i2cPort->requestFrom(_address, (uint8_t)1);
    if (_i2cPort->available()) {
        return _i2cPort->read();
    }
    return 0;
}

void myMAX30102::readFIFO(uint32_t &red, uint32_t &ir) {
    _i2cPort->beginTransmission(_address);
    _i2cPort->write(_fifo_data);
    _i2cPort->endTransmission(false);

    // In SpO2 mode, one sample = 3 bytes RED + 3 bytes IR
    _i2cPort->requestFrom(_address, (uint8_t)6);

    uint8_t redMSB = _i2cPort->read();
    uint8_t redMid = _i2cPort->read();
    uint8_t redLSB = _i2cPort->read();
    uint8_t irMSB  = _i2cPort->read();
    uint8_t irMid  = _i2cPort->read();
    uint8_t irLSB  = _i2cPort->read();

    red = ((uint32_t)redMSB << 16) | ((uint32_t)redMid << 8) | redLSB;
    ir  = ((uint32_t)irMSB  << 16) | ((uint32_t)irMid  << 8) | irLSB;

    // MAX30102 samples are 18-bit, so keep only lower 18 bits
    red &= 0x3FFFF;
    ir  &= 0x3FFFF;
}

void myMAX30102::fullRead(uint32_t& redAvg, uint32_t& irAvg, uint8_t& n) {

    // Load Fifo data into 
    _fullFIFO();

    if (_nsamples == 0 || _buffer_size == 0) {
        redAvg = 0; irAvg = 0; n = 0;
        return;
    }

    // condition ? va1 if condition true : val if condition is false
    uint8_t limit = (_nsamples < _buffer_size) ? _nsamples : _buffer_size;

    uint64_t redSum = 0;
    uint64_t irSum = 0;

    // 3. Sum only up to that limit
    for (uint8_t i = 0; i < limit; i++) {
        redSum += _red[i];
        irSum += _ir[i];
    }

    // 4. Calculate final values
    redAvg = (uint32_t)(redSum / limit);
    irAvg = (uint32_t)(irSum / limit);
    n = _nsamples;
}


void myMAX30102::_fullFIFO() {


    // First clear memory
    memset(_red,0,sizeof(_red));
    memset(_ir,0,sizeof(_ir));

    uint8_t front_ptr = readRegister(_fifo_wr_ptr);
    uint8_t back_ptr = readRegister(_fifo_rd_ptr);

    // Calculate how many samples are waiting
    int numSamples = front_ptr - back_ptr;
    if (numSamples < 0) numSamples += 32; // Handle wrap-around


    _nsamples = (uint8_t)numSamples;

    // Set up FIFO data register
    _i2cPort->beginTransmission(_address);
    _i2cPort->write(_fifo_data);
    _i2cPort->endTransmission(false);

    for (uint8_t i= 0; i < numSamples; i++) {

        // request 6 bytes
        _i2cPort->requestFrom(_address, (uint8_t)6);

        uint8_t redMSB = _i2cPort->read();
        uint8_t redMid = _i2cPort->read();
        uint8_t redLSB = _i2cPort->read();
        uint8_t irMSB  = _i2cPort->read();
        uint8_t irMid  = _i2cPort->read();
        uint8_t irLSB  = _i2cPort->read();

        if (i < _buffer_size)   {
            _red[i] = ((redMSB << 16) | (redMid << 8) | redLSB) & 0x3FFFF;
            _ir[i]  = ((irMSB  << 16) | (irMid  << 8) | irLSB) & 0x3FFFF;
        }
    }
    // reset the overflow counter
    writeRegister(_ovf_counter, 0x00);
}


// The command requests a die temperature reading from MAX30102
// After some time, the temp will show up in registers "_temp_int" and "_temp_frac"
// The temperature can be gotten with the "getTemp" function.
void myMAX30102::orderTemp() {
    writeRegister(_temp_config,0x01);
}


// Before reading temp "writeRegister(_temp_config,0x01);", must be done
// Do this funct during the interrupt
void myMAX30102::getTemp(int16_t& tempInSixteenths) {

    // Acknowledge interrupt by reading interrupt status:
    uint8_t status = readRegister(_intr_status_2);
    
     // Command sensor to read temperature;
    int8_t temp_int = readRegister(0x1F);   // int is 8 bits
    uint8_t temp_frac = readRegister(0x20); // frac is only 4 bits

    tempInSixteenths = ((int16_t)temp_int << 4) | (temp_frac & 0x0F); // the byte mask prevents the fraction from being negative

    return;
}