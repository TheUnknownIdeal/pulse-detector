//#include <iostream>
#include "Arduino.h"
#include "Processing.h"


SignalStream::SignalStream() : _hb{0} {

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

    // Check heartbeat and update _bt bucket
    delta = _write_new_delta(_bIR, hb);

    // package IR and RED into int64_t
    int64_t processed_pair = ((int64_t)IR << 32) | (uint32_t)RED;

    return processed_pair;    
}


// And Negative switching delta is used to detect heartbeats
int32_t SignalStream::_write_new_delta(int32_t new_val, bool& hb) {

    // Calculate new Delta
    int32_t delta = (int32_t)new_val - (int32_t)_hb[_w - 1];

    // We move values 'right' to make room for the new one at [0]
    for (uint16_t i = _w - 1; i > 0; i--) {
        _hb[i] = _hb[i - 1];
    }
    _hb[0] = new_val; // Update the 'newest' slot

    _ct += 1;

    // Log heartbeat if peak is detected and hb_delay time has elapsed
    if ( (delta < 0) && (_last_delta > 0) && (_ct > hb_delay)) {
        
        hb = true; // Toggle heartbeat flag on

        // update t bucket
        _bt += _ct;
        _bt -= _bt / (_wt + 1);

    
        // Do SpO2 computation
        _update_SpO2();
        _minIR= 0xFFFFFFFF;
        _minRED = 0xFFFFFFFF;// reset min and max
        _maxIR = 0;
        _maxRED = 0;

        _ct = 0; // reset period counter after it has been recorded

    } else {
        hb = false;
    }
    _last_delta = delta;

    return delta;
}

void SignalStream::_update_SpO2() {

    // Get data
    uint32_t ir,red, irDC, redDC;
    getP(ir,red);
    getDC(irDC, redDC);

    // Compute AC amplitudes
    uint32_t _redAC_amplitude = _maxRED - _minRED;
    uint32_t _irAC_amplitude = _maxIR - _minIR;

    //Serial.println(_redAC_amplitude);

    //uint32_t _redAC_amplitude = abs((int32_t)red - (int32_t)redDC);
    //uint32_t _irAC_amplitude = abs((int32_t)ir - (int32_t)irDC);

    //Serial.println(_redAC_amplitude);
    //Serial.println(_irAC_amplitude);

    // 1. Safety check to avoid "Division by Zero"
    if (irDC == 0 || redDC == 0 || _irAC_amplitude == 0) return;

    // 2. Use floats for the ratio to keep the decimals
    // Note: You need the AMPLITUDE (Peak-to-Peak) of the AC, not just a snapshot
    float R = ((float)_redAC_amplitude / redDC) / ((float)_irAC_amplitude / irDC);

    // 3. Apply the empirical linear approximation
    // SpO2 = A - B*R  (Commonly 104 - 17*R)
    float current_spo2 = 110.0 - (25.0 * R);

    // 4. Bound the result (You can't have 110% oxygen)
    if (current_spo2 > 100.0) current_spo2 = 100.0;
    if (current_spo2 < 50.0)  current_spo2 = 50.0;

    // 5. Save to your uint16_t (Fixed Point)
    float spo2 = (uint16_t)(current_spo2 * 100); 

    // Add to bucket
    _bSpO2 += spo2;
    _bSpO2 -= _bSpO2/(_wSpO2 + 1);
}


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
    SpO2 = _bSpO2 / _wSpO2;
}
