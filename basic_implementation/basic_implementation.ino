/*
  Optical Heart Rate Detection (PBA Algorithm) using the MAX30105 Breakout
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 2nd, 2016
  https://github.com/sparkfun/MAX30105_Breakout

  This is a demo to show the reading of heart rate or beats per minute (BPM) using
  a Penpheral Beat Amplitude (PBA) algorithm.

  It is best to attach the sensor to your finger using a rubber band or other tightening
  device. Humans are generally bad at applying constant pressure to a thing. When you
  press your finger against the sensor it varies enough to cause the blood in your
  finger to flow differently which causes the sensor readings to go wonky.

  Hardware Connections (Breakoutboard to Arduino):
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = A4 (or SDA)
  -SCL = A5 (or SCL)
  -INT = Not connected

  The MAX30105 Breakout can handle 5V or 3.3V I2C logic. We recommend powering the board with 5V
  but it will also run at 3.3V.
*/

#include <Wire.h>
#include "MAX30105.h"

#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

int finger_led = 5; // This is the pi that shows the light


int beat_led = 6; // This LED represents your heartbeat

int pin_dur = 100; // in this variable we store data on if the pin is HIGH or Low

bool no_finger_flag; 

const byte mem_size = 100;
bool finger_memory[mem_size]; // An array containing finger flags
byte finger_spot = 0;


void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  // Set up sensor
  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

  pinMode(beat_led, OUTPUT);

  // This is the Heartbit LED pin
  pinMode(finger_led, OUTPUT);

  // initialize finger_memory
  for (byte x = 0 ; x < mem_size; x++)
  {
    finger_memory[x] = false;
  }

  //pinMOde

  Serial.println("Place your index finger on the sensor with steady pressure.");

  delay(2000);



  



}

void loop()
{
  
  long current_time = millis();

  // Turn off heartbeat pin if past duration
  if (current_time - lastBeat > pin_dur)
  {
    digitalWrite(beat_led, LOW);
  }
  
  
  // Load sensor data into "irValue"
  long irValue = particleSensor.getIR();

  if (irValue < 50000)
  {

    finger_memory[finger_spot] = false;
    digitalWrite(finger_led, LOW); 
  }

  else
  {
    finger_memory[finger_spot] = true;
    digitalWrite(finger_led, HIGH); 
  }

  finger_spot++;
  if (finger_spot >= mem_size) finger_spot = 0;

  // Check measuring status of device
  int total = 0;
  for (int i = 0; i < mem_size; i++)
  {
    total += finger_memory[i];
  }

  if (total < mem_size/2)
  {
    return;
  }
  

  



  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = current_time - lastBeat;
    lastBeat = current_time;

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {

      digitalWrite(beat_led, HIGH);

      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;

      Serial.print("IR=");
      Serial.print(irValue);
      Serial.print(", BPM=");
      Serial.print(beatsPerMinute);
      Serial.print(", Avg BPM=");
      Serial.print(beatAvg);
      Serial.println();

    }

    
  }  

  
}


