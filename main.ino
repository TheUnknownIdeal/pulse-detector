#include <Wire.h>

#include "MAX30102.h"



int yellowLED = 5; // The LED pin

// Pass the address of the Wire object and the I2C address
MAX30102 pulseSensor(&Wire, 0x57); 

uint32_t redAvg, irAvg; // Variables to hold our results

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(yellowLED, OUTPUT);

    Serial.begin(115200);
    while (!Serial); // Wait for Serial Monitor (optional, good for Leonardo/ESP32)

    Serial.begin(115200);
    while (!Serial); // Wait for Serial Monitor (optional, good for Leonardo/ESP32)

    Wire.begin();
    Serial.begin(115200);

    Serial.println("Configuring MAX30102...");
    pulseSensor.setupSensor(); 
    
    Serial.println("Sensor Online. Place finger on sensor.");
}

void loop() {
    // This calls _fullFIFO() internally and calculates averages
    pulseSensor.fullRead(redAvg, irAvg);

    // Only print if we actually got samples
    if (redAvg > 0) {
        Serial.print("Red Avg: ");
        Serial.print(redAvg);
        Serial.print("\tIR Avg: ");
        Serial.println(irAvg);
    }

    // Small delay so we don't spam the I2C bus too fast
    // The sensor is set to 100Hz, so 20-50ms is a good polling rate
    delay(50); 
}