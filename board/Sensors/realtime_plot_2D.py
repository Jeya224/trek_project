import serial
import matplotlib.pyplot as plt
from collections import deque

# --- paramètres ---
COM_PORT = 'COM7'        # changer selon ton PC
BAUD_RATE = 115200
BUFFER_SIZE = 100        # nombre de points à afficher

# --- ouverture du port série ---
ser = serial.Serial(COM_PORT, BAUD_RATE)

# --- créer des deque pour stocker les dernières valeurs ---
x_data = deque(maxlen=BUFFER_SIZE)
y_data = deque(maxlen=BUFFER_SIZE)
z_data = deque(maxlen=BUFFER_SIZE)
a_data = deque(maxlen=BUFFER_SIZE)
t_data = deque(maxlen=BUFFER_SIZE)

# --- configuration du plot ---
plt.ion()
fig, ax = plt.subplots(figsize=(8, 4))
line_x, = ax.plot([], [], 'r', label='Acc X')
line_y, = ax.plot([], [], 'g', label='Acc Y')
line_z, = ax.plot([], [], 'b', label='Acc Z')
line_a, = ax.plot([], [], 'k', label='Magnitude')
ax.set_xlabel('Samples')
ax.set_ylabel('Acceleration')
ax.legend()
ax.set_title('Accélérations en temps réel')

print("Starting data collection...")
count = 0
while True:
    line_raw = ser.readline().decode('utf-8').strip()
    parts = line_raw.split()
    if len(parts) == 3:
        try:
            dX, dY, dZ = map(float, parts)
            x_data.append(dX)
            y_data.append(dY)
            z_data.append(dZ)
            #a_data.append(a)
            t_data.append(count)

            # mise à jour des lignes
            # line_x.set_data(t_data, x_data)
            # line_y.set_data(t_data, y_data)
            # line_z.set_data(t_data, z_data)
            line_a.set_data(x_data, y_data)

            # mise à jour automatique de l’échelle
            ax.relim()
            ax.autoscale_view()

            plt.pause(0.01)
        except:
            continue
    count += 1