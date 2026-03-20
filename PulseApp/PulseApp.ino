#include <Wire.h>

#include "PulseSensor.h"
#include "Processing.h"



int finger_led = 5; // This is the pi that shows the light
int beat_led = 6; // This LED represents your heartbeat

// Pass the address of the Wire object and the I2C address
myMAX30102 pulseSensor(&Wire, 0x57); 

uint32_t hb_tsp = 0;
uint32_t hb_diode_dur = 20000;

uint32_t last_sample_us = 0;
const uint32_t sample_interval_us = 10000; // 10,000 us = 10ms (100Hz)


// "sensor_data" houses and processes data
SignalStream sensor_data;

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


    // First check if it is time to turn off heartbeat diode
    long current_ms = micros();

    if (current_ms - hb_tsp > hb_diode_dur) {
        digitalWrite(beat_led, LOW);
    }

    // Check if it is the corect sampling time
    if (current_ms - last_sample_us >= sample_interval_us) {
        // This calls _fullFIFO() internally and calculates averages

        last_sample_us = current_ms;

        pulseSensor.fullRead(redAvg, irAvg, n);


        // Check if the finger is on the sensor
        if (irAvg < pulseSensor.threshold)  {
            digitalWrite(finger_led, LOW);
            

        } else { // Finger is on the sensor

            // Only execute analysis if finger is on sensor
            digitalWrite(finger_led, HIGH);

            bool hb = false;

            int32_t current_slope;

            uint64_t nIR_nRED = sensor_data.update(irAvg, redAvg, hb, current_slope);


            int32_t nRED = (int32_t)(nIR_nRED & 0xFFFFFFFF);
            int32_t nIR = (int32_t)(nIR_nRED >> 32);

            uint32_t IR_DC, RED_DC;
            sensor_data.getDC(IR_DC, RED_DC);

            uint16_t t;
            sensor_data.getT(t);

            float SpO2;
            sensor_data.getSpO2(SpO2);


            if (hb) {
                digitalWrite(beat_led, HIGH);
                hb_tsp = current_ms; // record hearbeat time

            }
            

            Serial.print("IR:");
            Serial.print(nIR);
            Serial.print(" IR_DC:");
            Serial.print(IR_DC);

            /*Serial.print(" RED:");
            Serial.print(redAvg);
            Serial.print(" RED_DC:");
            Serial.print(RED_DC);
            Serial.println();*/
            
            Serial.print(" BPM:");
            Serial.print(6000.0 / t);
            Serial.print(" SpO2:");
            Serial.print(SpO2);
            Serial.println();
            
            

        }
    }
}