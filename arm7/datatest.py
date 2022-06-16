import serial
import os,io,sys
data = b"exec:teste"
print(data)
ser = serial.Serial(sys.argv[1],38400)
ser.write(data)
ser.close()
