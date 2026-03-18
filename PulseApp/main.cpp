#include <iostream>
#include <cmath>
#include "Arduino.h"

#include "Processing.h"

const double PI = 3.141592653589793;

int main() {

    std::cout << "Starting Pulse Sensor Test..." << std::endl;

    uint32_t offset = 100000;
    uint32_t amplitude = 25000;

    uint32_t red, ir;

    SignalStream sensor_data{};

    //sensor_data.print_stream();


    // 2. Simulate feeding data (like a simple ramp or sine wave)
    for (uint32_t i = 0; i < 350; i++) {

        double phase = ((double)i/300);//*2.0*PI;

        red = amplitude*sin(phase) + offset;
        ir = amplitude*sin(phase+ (PI/4)) + offset;
        //std::cout << "Sample " << i << " | Red: " << red << " | IR: " << ir << std::endl;
        
        sensor_data.update(ir, red);
        

    }

    sensor_data.print_stream();

    std::cout << "Test Finished." << std::endl;
    
}