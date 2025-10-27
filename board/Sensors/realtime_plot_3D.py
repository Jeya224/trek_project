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

# --- configuration du plot ---
plt.ion()  # mode interactif
ax = plt.figure().add_subplot(projection='3d')
line, = ax.plot([], [], [], label='trajectory')
ax.set_xlabel('x')
ax.set_ylabel('y')
ax.set_zlabel('z')
ax.set_title('3D Trajectory')
ax.legend()

# --- boucle de lecture et mise à jour ---
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
 
            # mise à jour du plot
            line.set_data(x_data, y_data)
            line.set_3d_properties(z_data)

            # mise à jour dynamique des axes
            ax.set_xlim(min(x_data), max(x_data))
            ax.set_ylim(min(y_data), max(y_data))
            ax.set_zlim(min(z_data), max(z_data))

            plt.pause(0.01)

        except:
            continue
    count += 1
    print(count)
