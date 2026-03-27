# MAX30102 Pulse Detector Experiment

This project reads photoplethysmography (PPG) data from a **MAX30102 heart rate sensor** using an **Arduino Uno**.
The Arduino streams the infrared (IR) signal over serial, and a Python script logs the data to a CSV file for analysis.

The goal of the project is to experiment with **heartbeat detection algorithms** using the raw IR time series from the sensor.

![Circuit Setup]()

<figure>
  <img src="images/device.jpg" alt="Pulse Detector Device">
  <figcaption>
    <b>Figure 1:</b> The pulse detector device in operation.</i>
  </figcaption>
</figure>

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

## Schematic

<figure>
  <img src="images/schematic.svg" alt="Circuit Diagram">
  <figcaption>
    <b>Figure 2:</b> The pulse detector schematic.</i>
  </figcaption>
</figure>

## Software

**Operational Overview**

1. Reads red and infared samples (from the tip of your finger with MAX30102 sensor)
2. Processes the data using leaky bucket algorithm
3. From the processed sample, it checks if a new maximum or minimum red and infrred values have been acquired. If so, it records them
4. Additionally, from the processed sample it 
5. On the onset of a new period (after a heartbeat is detected) maximum and minimum readings for both ir and red samples are reset

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