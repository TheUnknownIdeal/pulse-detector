#include <iostream>
#include "Arduino.h"
#include "Processing.h"


SignalStream::SignalStream() : _ir{0}, _red{0} {

    _i = 0;
    _filled = 0;
}


void SignalStream::update(uint32_t& ir, uint32_t& red) {

    _add_value(ir, red);   
}


// print all data for testing
void SignalStream::print_stream() {
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



void SignalStream::_add_value(uint32_t& ir, uint32_t& red) {

    if (++_i >= _len) _i = 0;

    _ir[_i] = ir;
    _red[_i] = red;

    if (_filled < _len) _filled++;

}
