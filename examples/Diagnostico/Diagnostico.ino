// Diagnostico de la TPIC-Lab V1: no usa la biblioteca.
// Parpadea el LED HOT y grafica las lecturas crudas de los sensores.
// Abra el Serial Plotter a 9600 baudios.

const int PIN_LED = 9;
bool encendido = false;
unsigned long ultimo = 0;

void setup() {
  Serial.begin(9600);
  analogReference(EXTERNAL);          // igual que la biblioteca
  pinMode(PIN_LED, OUTPUT);
}

void loop() {
  if (millis() - ultimo >= 500) {
    ultimo = millis();
    encendido = !encendido;
    analogWrite(PIN_LED, encendido ? 50 : 0);       // LED HOT parpadea
    int a0 = analogRead(A0);                        // TMP1: a 25 C debe dar cerca de 233
    int a2 = analogRead(A2);                        // TMP2
    Serial.print("A0:");
    Serial.print(a0);
    Serial.print(",A2:");
    Serial.print(a2);
    Serial.print(",T1:");
    Serial.println((a0 * (3300.0 / 1024.0) - 500.0) / 10.0);
  }
}
