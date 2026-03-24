#include <Wire.h>
#include <LiquidCrystal.h> 

#include "PulseSensor.h"
#include "Processing.h"


// LCD PIN numbers defined here
const int rs = 12, en = 11, d4 = 8, d5 = 7, d6 = 6, d7 = 5;

// initialize the library by associating any needed LCD interface pin
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


// Defining the LED pin numbers
int finger_led = 10; // This is the pi that shows the light

int beat_led = 9; // This LED represents your heartbeat
uint32_t hb_diode_dur = 30000; // Blink duration of pulse detecting diode
uint32_t hb_tsp = 0; // timestamp of last detected hearbeat (used for timing the shutdown of beat_led)



// The object that controls MAX30102
myMAX30102 pulseSensor(&Wire, 0x57); // Pass the address of the Wire object and the I2C address

// INT pin on MAX30102
#define SENSOR_INTERRUPT 3

volatile byte interrupt_pin = HIGH;

uint32_t last_temp_sample_us = 0;
const uint32_t temp_sample_interval_us = 1000000; // every second



// constants used for timing MAX30102 sampling
uint32_t last_sample_us = 0; // <- Records time of previous sample ("us" microseconds)
const uint32_t sample_interval_us = 10000; // 10,000 us = 10ms (100Hz) <- SAMPLING RATE




// The object for processing data
SignalStream sensor_data; 



void setup() {
    // Set up LEDs
    pinMode(beat_led, OUTPUT); // Blinks when pulse is detected
    pinMode(finger_led, OUTPUT); // Blinks when valid IR data is returned from MAX30102 (i.e. a finger is detected)
    pinMode(finger_led, OUTPUT); // Blinks when valid IR data is returned from MAX30102 (i.e. a finger is detected)

    Serial.begin(115200);
    delay(1000);

    Wire.begin();
    Serial.println("Starting Scan...");

    // Check if the sensor is even responding to a ping
    Wire.beginTransmission(0x57);
    if (Wire.endTransmission() == 0) {
        Serial.println("Sensor Detected at 0x57!");
    } else {
        Serial.println("Sensor NOT found on I2C bus.");
    }


    Serial.println("Configuring MAX30102...");
    pulseSensor.setupSensor(); // set up max30102 driver
    Serial.println("Configuration Successful!");

    attachInterrupt(digitalPinToInterrupt(SENSOR_INTERRUPT), loadTemp, FALLING);

    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);

    lcd.print("Config done.");

    
}

void loop() {


    // First check if it is time to turn off heartbeat diode
    long current_ms = micros();

    if (current_ms - hb_tsp > hb_diode_dur) {
        digitalWrite(beat_led, LOW);
    }

    // Check if it is temperature sampling time
    if (current_ms - last_temp_sample_us >= temp_sample_interval_us) {
        last_temp_sample_us = current_ms;
        pulseSensor.orderTemp();
        Serial.println("temperature ordered");
    }

    // Check if it is IR and RED sampling time
    if (current_ms - last_sample_us >= sample_interval_us) {
        // This calls _fullFIFO() internally and calculates averages

        last_sample_us = current_ms;

        uint32_t redAvg, irAvg; // Variables IR and RED samples 
        uint8_t n; //variable holding the number of samples returned per call

        pulseSensor.fullRead(redAvg, irAvg, n);


        // Check if the finger is on the sensor
        if (irAvg < pulseSensor.threshold)  {
            digitalWrite(finger_led, LOW);
            

        } else { // Finger is on the sensor

            // Only execute analysis if finger is on sensor
            digitalWrite(finger_led, HIGH);

            bool hb = false; // flag that tells whether pulse was detected
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

            float BPM = 6000.0 / t;


            if (hb) {
                digitalWrite(beat_led, HIGH);
                hb_tsp = current_ms; // record hearbeat time
                
                int16_t wholeDegree, milliDegrees;

                temp_int_to_float(sensor_data.temp, wholeDegree, milliDegrees);

                float tempurature = (float)wholeDegree + (float)milliDegrees / 1000.0;

            
                
                lcd.setCursor(0, 0);
                lcd.print("BPM: ");
                lcd.print(BPM,1 );
                lcd.print("   ");

                lcd.setCursor(0, 1);
                lcd.print("Temp: ");
                lcd.print(tempurature, 1);
                lcd.print(" C  ");
                

            }
            
            /*
            Serial.print("IR:");
            Serial.print(nIR);
            Serial.print(" IR_DC:");
            Serial.print(current_slope);

            Serial.print(" RED:");
            Serial.print(nRED);
            Serial.print(" RED_DC:");
            Serial.print(RED_DC);
            Serial.println();
            
            Serial.print(" BPM:");
            Serial.print(6000.0 / t);
            Serial.print(" SpO2:");
            Serial.print(SpO2);
            Serial.println();*/
            
            

        }
    }
}

// INterrupt function
void loadTemp() {
    // Load new temp into "sensor_data"
    int16_t localTemp;
    pulseSensor.getTemp(localTemp);
    sensor_data.temp = localTemp;
    //Serial.println(localTemp);
    
}