#include <Wire.h>

#include "PulseSensor.h"
#include "Processing.h"



int finger_led = 5; // This is the pi that shows the light
int beat_led = 6; // This LED represents your heartbeat

// Pass the address of the Wire object and the I2C address
myMAX30102 pulseSensor(&Wire, 0x57); 

long HB_delay = 100;

// "sensor_data" houses and processes data
SignalStream sensor_data(HB_delay);

uint32_t redAvg, irAvg; // Variables to hold our results
uint8_t n; //variable holding the number of samples returned per call

void setup() {
    // Set up LEDs
    pinMode(beat_led, OUTPUT); // This is the Heartbit LED pin
    pinMode(finger_led, OUTPUT);

    Serial.begin(115200);
    while (!Serial); // Wait for Serial Monitor (optional, good for Leonardo/ESP32)

    Serial.begin(115200);
    while (!Serial); // Wait for Serial Monitor (optional, good for Leonardo/ESP32)

    Wire.begin();
    Serial.begin(115200);

    Serial.println("Configuring MAX30102...");

    pulseSensor.setupSensor(); // set up max30102 driver

    
}

void loop() {
    // This calls _fullFIFO() internally and calculates averages
    pulseSensor.fullRead(redAvg, irAvg, n);

    long current_time = millis();

    if (current_time - sensor_data.hb_stamp > HB_delay) {
        digitalWrite(beat_led, LOW);
    }

    bool hb = false;

    int32_t current_slope;

    int64_t nIR_nRED = sensor_data.rolling(irAvg, redAvg, (uint16_t)20, hb, current_time,current_slope);
    //int64_t nIR_nRED = sensor_data.update(irAvg, redAvg, (uint16_t)20, hb, current_time, current_slope);

    int32_t nRED = (int32_t)(nIR_nRED & 0xFFFFFFFF);
    int32_t nIR = (int32_t)(nIR_nRED >> 32);

    if (hb) {
        digitalWrite(beat_led, HIGH);

    }


    
    Serial.print("ir:");
    Serial.print(nIR);
    Serial.print(" ");
    Serial.print("red:");
    Serial.print(nRED);
    Serial.print(" n:");
    Serial.print(current_slope);
    Serial.println();

    // Small delay so we don't spam the I2C bus too fast
    // The sensor is set to 100Hz, so 20-50ms is a good polling rate
    delay(10); 
}