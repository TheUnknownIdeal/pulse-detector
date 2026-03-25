# MAX30102 Pulse Sensor Experiment

This project reads photoplethysmography (PPG) data from a **MAX30102 heart rate sensor** using an **Arduino Uno**.
The Arduino streams the infrared (IR) signal over serial, and a Python script logs the data to a CSV file for analysis.

The goal of the project is to experiment with **heartbeat detection algorithms** using the raw IR time series from the sensor.

![Circuit Setup](images/device.jpg)

## Hardware

* Arduino Uno
* GY-MAX30102 pulse sensor
* USB connection to computer
* 500 ohm resistors 2pcs
* LED 2 pcs
* LCD 1602 
* 220 ohm resistor
* 10k ohm resistor
* 2 4.7k ohm resistors 
* 10k ohm potentiometer
* hook-up wires
* breadboard


### Wiring

| Sensor | Arduino |
| ------ | ------- |
| VIN    | 3.3V    |
| GND    | GND     |
| SDA    | A4      |
| SCL    | A5      |

## Software

Two programs are used:

**Arduino sketch**

1. Reads red and infared samples (from the tip of your finger with MAX30102 sensor)
2. Processes the data using leaky bucket algorithm
3. From the processed data, it computes whether a pulse is detected by search for a peak in the periodic signal
4. From pulse time stamps it compute the period of the pulse, and again applies the leaky bucket algorithm to the period raw period calculations
5. On the onset of a new period (after a heartbeat is detected) maximum and minimum readings for both ir and red samples are reset

Example output:

time,IR
0.000,52341
0.010,52355
0.020,52410

## Running the project

1. Upload the Arduino sketch to the board.
2. Connect the sensor and place a finger over it.

```
python3 logger.py
```

4. Data will be saved to `pulse.csv`.

## Future work

* Implement heartbeat detection
* Compute BPM from detected peaks
* Visualize pulse waveform

## License

MIT License