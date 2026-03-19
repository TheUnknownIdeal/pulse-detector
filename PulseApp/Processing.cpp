//#include <iostream>
#include "Arduino.h"
#include "Processing.h"


SignalStream::SignalStream() : _hb{0} {

    t = 0;
    _c1 = 0;

    _irDC = 0;
    _redDC = 0;
    
}

int64_t SignalStream::rolling(uint32_t& ir, uint32_t& red, uint16_t dp, bool& hb, int32_t& slope) {
    /*
    "dp": means depth, "how many previous raw samples should be averaged to create new processed sample"
    "hb": boolean variable for if heartbeat is present. 0 - no, 1 - yes
    "current_time": A timestamp used to compare with time of previous heartbeat
    */
    _updateP(ir, red);

    // Calculate Delta detect heartbeat
    slope = _write_new_delta(_ir, hb);

    // We cast redDiff to uint32_t to "neutralize" the sign bit before the OR
    int64_t processed_pair = ((int64_t)_ir << 32) | (uint32_t)_red;

    return processed_pair;    
}

// And Negative switching delta is used to detect heartbeats
int32_t SignalStream::_write_new_delta(int32_t new_val, bool& hb) {

    // Calculate new Delta
    int32_t delta = (int32_t)new_val - (int32_t)_hb[_w - 1];

    // Shift values in registers
    // 2. Shift the history buffer 
    // We move values 'right' to make room for the new one at [0]
    for (uint16_t i = _w - 1; i > 0; i--) {
        _hb[i] = _hb[i - 1];
    }
    _hb[0] = new_val; // Update the 'newest' slot

    // _c1 is counting (in samples) the duration of the current period

    // Log heartbeat if peak is detected and hb_delay time has elapsed
    if ( (delta < 0) && (_last_delta > 0) && (_c1 > hb_delay)) {
        
        hb = true;
        
        // LEaky Bucket FTW
        t += _c1 / (_wt + 1);
        t -= t / (_wt + 1);

        _c1 = 0; // reset period counter after it has been recorded

    } else {
        hb = false;
        _c1++; // Add to period counter
    }
    _last_delta = delta;

    return delta;
}
    

void SignalStream::_get_DC(uint32_t& ir, uint32_t& red) {

    //_c2 is another sample counter used for saving samples mid period

    // Don't track until a period is established;
    if (t == 0) return;

    // LEaky Bucket FTW
    _irDC += ir / (t + 1);
    _redDC += red / (t + 1);

    _irDC -= _irDC / (t + 1); // period +1 denominator keeps size of bucket at period
    _redDC -= _redDC / (t + 1);

}


void SignalStream::_updateP(uint32_t& ir, uint32_t& red)  {

    // LEaky Bucket FTW
    _ir += ir / (_wAC + 1);
    _red += red / (_wAC + 1);

    _ir -= _ir / (_wAC + 1); // period +1 denominator keeps size of bucket at period
    _red -= _red / (_wAC + 1);
    
}

void SignalStream::_spo2_compute() {

    // Compute ACs
    uint32_t _redAC_amplitude = abs((int32_t)_red - (int32_t)_redDC);
    uint32_t _irAC_amplitude = abs((int32_t)_ir - (int32_t)_irDC);


    // 1. Safety check to avoid "Division by Zero"
    if (_irDC == 0 || _redDC == 0 || _irAC_amplitude == 0) return;


    // 2. Use floats for the ratio to keep the decimals
    // Note: You need the AMPLITUDE (Peak-to-Peak) of the AC, not just a snapshot
    float R = ((float)_redAC_amplitude / _redDC) / ((float)_irAC_amplitude / _irDC);

    // 3. Apply the empirical linear approximation
    // SpO2 = A - B*R  (Commonly 104 - 17*R)
    float current_spo2 = 104.0 - (17.0 * R);

    // 4. Bound the result (You can't have 110% oxygen)
    if (current_spo2 > 100.0) current_spo2 = 100.0;
    if (current_spo2 < 50.0)  current_spo2 = 50.0;

    // 5. Save to your uint16_t (Fixed Point)
    spo2 = (uint16_t)(current_spo2 * 100); 
}

