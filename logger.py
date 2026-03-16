import serial

ser = serial.Serial('/dev/ttyACM0',115200)

with open("pulse.csv","w") as f:
    while True:
        line = ser.readline().decode().strip()
        print(line)
        f.write(line+"\n")