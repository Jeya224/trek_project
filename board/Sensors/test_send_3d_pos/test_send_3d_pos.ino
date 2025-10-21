// Arduino - Continuous 3D helix coordinates via Serial

float r = 1.0;           // rayon de la spirale
float z = -2.0;          // position Z initiale
float dz = 0.01;         // incrément Z à chaque point
float theta = 0.0;       // angle initial
float dtheta = 0.1;      // incrément d'angle

void setup() {
  Serial.begin(115200);
}

void loop() {
  // calcule x et y
  float x = r * sin(theta);
  float y = r * cos(theta);

  // envoie x, y, z séparés par un espace
  Serial.print(x, 4); Serial.print(" ");
  Serial.print(y, 4); Serial.print(" ");
  Serial.println(z, 4);

  // incrémente theta et z pour le prochain point
  theta += dtheta;
  z += dz;

  delay(50);  // contrôle la vitesse d'envoi
}
