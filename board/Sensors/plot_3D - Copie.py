import matplotlib.pyplot as plt
import numpy as np
import serial
import matplotlib.pyplot as plt

# --- paramètres ---
COM_PORT = 'COM7'        # changer selon ton PC
BAUD_RATE = 115200
BUFFER_SIZE = 1000        # nombre de points à afficher

# --- ouverture du port série ---
ser = serial.Serial(COM_PORT, BAUD_RATE)

x_data, y_data, z_data = [], [], []
current_size = 0

print("Starting data collection...")
while current_size < BUFFER_SIZE:
    line_raw = ser.readline().decode('utf-8').strip()
    parts = line_raw.split()
    if len(parts) == 3:
        try:
            dX, dY, dZ = map(float, parts)
            x_data.append(dX)
            y_data.append(dY)
            z_data.append(dZ)
        except:
            continue
    current_size += 1
    print(current_size)
print("Data collection finished.")
print(x_data[50:100], y_data[50:100], z_data[50:100])

# --- configuration du plot 3D ---
ax = plt.figure().add_subplot(projection='3d')

# Prepare arrays x, y, z

ax.plot(x_data, y_data, z_data, label='trajectory')
ax.set_xlabel('x')
ax.set_ylabel('y')
ax.set_zlabel('z')
ax.legend()

plt.show()