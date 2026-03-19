#include <Wire.h>

#include "PulseSensor.h"
#include "Processing.h"



int finger_led = 5; // This is the pi that shows the light
int beat_led = 6; // This LED represents your heartbeat

// Pass the address of the Wire object and the I2C address
myMAX30102 pulseSensor(&Wire, 0x57); 

long hb_tsp = 0;
long hb_diode_dur = 20;


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

    // This calls _fullFIFO() internally and calculates averages
    pulseSensor.fullRead(redAvg, irAvg, n);


    // First check if it is time to turn off heartbeat diode
    long current_time = millis();

    if (current_time - hb_tsp > hb_diode_dur) {
        digitalWrite(beat_led, LOW);
    }

    // Check if the finger is on the sensor
    if (irAvg < pulseSensor.threshold)  {
        digitalWrite(finger_led, LOW);
        

    } else {

        // Only execute analysis if finger is on sensor
        digitalWrite(finger_led, HIGH);

        // record hearbeat time
        hb_tsp = current_time;

        bool hb = false;

        int32_t current_slope;

        int64_t nIR_nRED = sensor_data.rolling(irAvg, redAvg, (uint16_t)20, hb, current_slope);
        //int64_t nIR_nRED = sensor_data.update(irAvg, redAvg, (uint16_t)20, hb, current_time, current_slope);

        int32_t nRED = (int32_t)(nIR_nRED & 0xFFFFFFFF);
        int32_t nIR = (int32_t)(nIR_nRED >> 32);

        if (hb) {
            digitalWrite(beat_led, HIGH);

        }
        /*
        Serial.print("ir:");
        Serial.print(nIR);
        Serial.print(" ");
        Serial.print("red:");
        Serial.print(nRED);
        */
        Serial.print(" delta:");
        Serial.print(current_slope);
        Serial.print(" t:");
        Serial.print(sensor_data.period);
        Serial.println();
    }

    

    // Small delay so we don't spam the I2C bus too fast
    // The sensor is set to 100Hz, so 20-50ms is a good polling rate
    delay(10); 
}