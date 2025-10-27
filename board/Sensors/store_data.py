import serial
import csv
import os

# nom du fichier CSV
csv_filename = "donnees_nicla.csv"

# chemin complet vers le dossier ./data
data_dir = "./data"
os.makedirs(data_dir, exist_ok=True)  # crée le dossier s’il n’existe pas

# chemin complet du fichier CSV
csv_path = os.path.join(data_dir, csv_filename)

# --- paramètres ---
COM_PORT = 'COM7'        # changer selon ton PC
BAUD_RATE = 115200
BUFFER_SIZE = 100        # nombre de points à afficher

# --- ouverture du port série ---
ser = serial.Serial(COM_PORT, BAUD_RATE)

print("Starting data collection...")

# ouverture du fichier CSV
with open(csv_path, mode='w', newline='') as file:
    writer = csv.writer(file)
    # en-tête
    writer.writerow(['count', 'x', 'y', 'z', 'delta_t'])

    count = 0
    while True:
        line_raw = ser.readline().decode('utf-8').strip()
        parts = line_raw.split()
        if len(parts) == 4:
            try:
                x, y, z, delta_t = map(float, parts)
                writer.writerow([count, x, y, z, delta_t])
            except:
                continue
        count += 1