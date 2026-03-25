//#include <iostream>
#include "Arduino.h"
#include "Processing.h"


SignalStream::SignalStream() : _hb{0} {

    temp = 400; // 25 but shifted 4 places.

    _bt = 0;
    _ct = 0;

    _bIR = 0;
    _bRED = 0;

    _bIR_DC = 0;
    _bRED_DC = 0;

    _bSpO2 = 0;

    _minIR= 0xFFFFFFFF;
    _minRED = 0xFFFFFFFF;// reset min and max
    _maxIR = 0;
    _maxRED = 0;
   
}

uint64_t SignalStream::update(uint32_t& ir, uint32_t& red, bool& hb, int32_t& delta) {
    /*
    "hb": boolean variable for if heartbeat is present. 0 - no, 1 - yes
    */
    
    // UPdate P buckets
    _bIR += ir; _bIR -= _bIR / (_wAC + 1); 
    _bRED += red; _bRED -= _bRED / (_wAC + 1);

    // Compute P values
    uint32_t IR, RED;
    getP(IR, RED);

    // Update limtis
    if (IR > _maxIR) _maxIR = IR;
    if (RED > _maxRED) _maxRED = RED;
    if (IR < _minIR) _minIR = IR;
    if (RED < _minRED) _minRED = RED;


    // Update DC buckets with P values
    _bIR_DC += IR; _bIR_DC -= _bIR_DC / (_wDC + 1);
    _bRED_DC += RED; _bRED_DC -= _bRED_DC / (_wDC + 1);

    // Update t (period) counter
    _ct += 1;

    // Check heartbeat and update _bt bucket
    delta = _detect_HB(_bIR, hb);
    
    if (hb) {

        // update t bucket
        _bt += _ct;
        _bt -= _bt / (_wt + 1);

        _ct = 0; // reset period counter after it has been recorded

        // Do SpO2 computation
        _update_SpO2();

        // Rest period specific min and max values
        _minIR= 0xFFFFFFFF;
        _minRED = 0xFFFFFFFF;// reset min and max
        _maxIR = 0;
        _maxRED = 0;

    }

    // package IR and RED into int64_t
    int64_t processed_pair = ((int64_t)IR << 32) | (uint32_t)RED;

    return processed_pair;    
}


// Detecting a heartbeat from a negatively switching delta
int32_t SignalStream::_detect_HB(int32_t new_val, bool& hb) {

    // Calculate new Delta by subtracting new IR from oldest stored IR
    int32_t delta = (int32_t)new_val - (int32_t)_hb[_w - 1];

    // We move values 'right' to make room for the new one at [0]
    for (uint16_t i = _w - 1; i > 0; i--) {
        _hb[i] = _hb[i - 1];
    }
    _hb[0] = new_val; // Update the 'newest' slot

    // Log heartbeat if peak is detected and hb_delay time has elapsed
    if ( (delta < 0) && (_last_delta > 0) && (_ct > hb_delay)) {

        hb = true; // Toggle heartbeat flag on

    } else {
        hb = false;
    }
    _last_delta = delta; // Update newly calculated delta as last delta

    return delta;
}

// Sp02 Computing Function
void SignalStream::_update_SpO2() {

    // Get data
    uint32_t ir,red, irDC, redDC;
    getP(ir,red);
    getDC(irDC, redDC);

    // Compute AC amplitudes
    uint32_t _redAC_amplitude = _maxRED - _minRED;
    uint32_t _irAC_amplitude = _maxIR - _minIR;


    // Make sure no division by zero
    if (irDC == 0 || redDC == 0 || _irAC_amplitude == 0) return;

    // Compute R and use floats to keep decimal points
    float R = ((float)_redAC_amplitude / redDC) / ((float)_irAC_amplitude / irDC);

    // Apply the empirical linear approximation
    // SpO2 = A - B*R
    // A = 110 or 104, B = 25 or 17
    //float current_spo2 = 110.0 - (25.0 * R);
    float current_spo2 = 104.0 - (17.0 * R);

    




    // Bound the result between 50 % and 100%
    if (current_spo2 > 100.0) current_spo2 = 100.0;
    if (current_spo2 < 50.0)  current_spo2 = 50.0;

    // 5. Save to your uint16_t (Fixed Point)
    //float spo2 = (uint16_t)(current_spo2 * 100); 
    uint16_t spo2 = (uint16_t)(current_spo2 * 100); 

    // Add to bucket
    _bSpO2 += spo2;
    _bSpO2 -= _bSpO2/(_wSpO2 + 1);
}

// Functions for computing values from buckets
void SignalStream::getP(uint32_t& ir, uint32_t& red) {

    ir = _bIR/_wAC; 
    red = _bRED/_wAC; 
}


void SignalStream::getDC(uint32_t& ir, uint32_t& red) {

    ir = _bIR_DC/_wDC; 
    red = _bRED_DC/_wDC; 
}

void SignalStream::getT(uint16_t& t) {
    t = _bt / _wt;
}

void SignalStream::getSpO2(float& SpO2) {

    // Get Raw reading from bin
    SpO2 = _bSpO2 / _wSpO2;
    SpO2 /= 100;

    // compute temperature offset from (25 C)
    int16_t tempOffset = temp - 400; //subtract 25 C (400 in sixteenths) to get offset

    // Shift by 0.12 for each degree (16 units)
    float tempCorrection = (tempOffset / 16.0)* 0.12;

    SpO2 += tempCorrection;
    
}

void temp_int_to_float(int16_t tempInSixteenths, int16_t& wholeDegrees, int16_t& milliDegrees) {
     // 2. How to use it without floats:
    wholeDegrees = tempInSixteenths >> 4;          // Back to 25
    milliDegrees = (tempInSixteenths & 0xFF) * 625; // 8 * 625 = 5000 (0.5000 degrees)
}
