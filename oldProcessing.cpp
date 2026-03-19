//#include <iostream>
#include "Arduino.h"
#include "Processing.h"


SignalStream::SignalStream(long& hbDelay) : _ir{0}, _red{0}, _hb{0} {

    hb_delay = hbDelay;

    _i = 0;
}


int64_t SignalStream::update(uint32_t& ir, uint32_t& red, uint16_t dp, bool& hb, long& current_time,int32_t& slope) {

    _add_value(ir, red);

    // Get global average (DC)
    uint32_t irDC, redDC;
    _getAvg(irDC, redDC, (uint16_t)_len);

    // Get rolling average
    uint32_t irRol, redRol;
    _getAvg(irRol, redRol, dp);

    
    

    // 1. Calculate the signed differences (AC components)
    int32_t irDiff  = (int32_t)irRol - (int32_t)irDC;
    int32_t redDiff = (int32_t)redRol - (int32_t)redDC;

    // Calculate slope and heartbeat
    slope = _write_new_slope(irDiff, hb, current_time);

    // We cast redDiff to uint32_t to "neutralize" the sign bit before the OR
    int64_t normalized_pair = ((int64_t)irDiff << 32) | (uint32_t)redDiff;

    return normalized_pair;

    
}

int64_t SignalStream::rolling(uint32_t& ir, uint32_t& red, uint16_t dp, bool& hb, long& current_time, int32_t& slope) {
    /*
    "dp": means depth, "how many previous raw samples should be averaged to create new processed sample"
    "hb": boolean variable for if heartbeat is present. 0 - no, 1 - yes
    "current_time": A timestamp used to compare with time of previous heartbeat
    */

    _add_value(ir, red);

    // Get rolling average
    uint32_t irRol, redRol;
    _getAvg(irRol, redRol, dp);

    // Calculate slope and heartbeat
    slope = _write_new_slope((int32_t)irRol, hb, current_time);

    // We cast redDiff to uint32_t to "neutralize" the sign bit before the OR
    int64_t processed_pair = ((int64_t)irRol << 32) | (uint32_t)redRol;

    return processed_pair;    
}

int32_t SignalStream::_write_new_slope(int32_t new_val, bool& hb, long& timestamp) {

    // Calculate new slope
    int32_t slope = (int32_t)new_val - (int32_t)_hb[_n - 1];

    // Shift values in registers
    // 2. Shift the history buffer 
    // We move values 'right' to make room for the new one at [0]
    for (uint16_t i = _n - 1; i > 0; i--) {
        _hb[i] = _hb[i - 1];
    }
    _hb[0] = new_val; // Update the 'newest' slot

    // Log heartbeat if peak is detected and hb_delay time has elapsed
    if ( (slope < 0) && (_last_slope > 0) && (timestamp - hb_stamp > hb_delay)) {
        hb = true;
        hb_stamp = timestamp;
    } else {
        hb = false;
    }
    _last_slope = slope;

    return slope;

}

void SignalStream::_getAvg(uint32_t& irAvg, uint32_t& redAvg, uint16_t dp)  {

    /*
    "dp": "depth" - How many samples we will take for the rolling average
        - Will be set to buffer size if a depth greater than buffer size is given
    */

    uint64_t redSum = 0;
    uint64_t irSum = 0;

    if (dp > _len) dp = _len;


    // 3. Sum only up to that limit
    for (uint16_t j = 0; j < dp; j++) {

        int16_t i = _i - 1 - j;
        while (i < 0) i += _len;

        redSum += _red[i];
        irSum += _ir[i];

    }
    redAvg = (uint32_t)(redSum / dp);
    irAvg = (uint32_t)(irSum / dp);
}
// print all data for testing
/*

void SignalStream::print_stream() { // iostream version
    std::cout << "i, ir, red, <--- _i "<< std::endl;
    for (uint16_t i = 0; i < _len; i++) {
        std::cout << i << ", " << _ir[i] << ", " << _red[i];
        
        // Mark where the current 'write pointer' is
        if (i == _i) {
            std::cout << " <--- _i";
        }
        
        // This is the part that was missing the <<
        std::cout << std::endl; 
    }

}
*/
void SignalStream::print_stream() { // Arduino serial monitor version
    Serial.println("i, ir, red, <--- _i ");
    for (uint16_t i = 0; i < _len; i++) {
    
        Serial.print(i);
        Serial.print(", ");
        Serial.print(_ir[i]);
        Serial.print(", ");
        Serial.print(_red[i]);
        
        // Mark where the current 'write pointer' is
        if (i == _i) {
            Serial.print(" <--- _i");
        }
        Serial.println();
    }

}



void SignalStream::_add_value(uint32_t& ir, uint32_t& red) {

    _ir[_i] = ir;
    _red[_i++] = red;

    if (_i >= _len) _i = 0;

}
