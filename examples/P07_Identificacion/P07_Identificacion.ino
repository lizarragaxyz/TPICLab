// P7: identificacion en linea de un modelo FOPDT (metodo de dos puntos)
#include <TPICLab.h>

TPICLab lab;
const int NMAX = 300;                 // 300 muestras cada 2 s = 10 minutos
int16_t Tm[NMAX];                     // T1 en centesimas de grado
int n = 0;                            // muestras guardadas
bool enPrueba = false;
float u2 = 0.0;                       // ultimo instante de guardado
float q1 = 0.0;                       // escalon aplicado (%)
float T0 = 0.0;                       // temperatura antes del escalon
float dT1 = 0.0;
float kpV = 0.0;                      // resultados (se grafican al terminar)
float tauV = 0.0;
float thetaV = 0.0;
float listo = 0.0;                    // 1: ya hay modelo

// Primer instante (s) en que T1 alcanza "nivel"; interpola entre muestras
float tiempoDe(float nivel) {
  for (int i = 1; i < n; i++) {
    float a = Tm[i - 1] / 100.0;
    float b = Tm[i] / 100.0;
    if (b >= nivel) return 2.0 * (i - 1) + 2.0 * (nivel - a) / (b - a);
  }
  return -1.0;
}

void identificar() {
  if (n < 30) {
    if (lab.mensajes) Serial.println(F("# faltan datos (minimo 1 minuto)"));
    return;
  }
  float Tf = 0.0;
  for (int i = n - 10; i < n; i++) Tf += Tm[i] / 100.0;
  Tf /= 10.0;                                        // temperatura final (ultimos 20 s)
  float dT = Tf - T0;
  float t283 = tiempoDe(T0 + 0.283 * dT);
  float t632 = tiempoDe(T0 + 0.632 * dT);
  float tau = 1.5 * (t632 - t283);
  float theta = t632 - tau;
  kpV = dT / q1 * 100.0;                             // Kp x 100 para graficar
  tauV = tau;
  thetaV = theta;
  listo = 1.0;
  if (lab.mensajes) {                                // resultados en texto (Monitor Serie)
    Serial.print(F("Kp = "));     Serial.println(dT / q1, 3);
    Serial.print(F("tau = "));    Serial.println(tau, 1);
    Serial.print(F("theta = "));  Serial.println(theta, 1);
  }
}

void terminar() {
  enPrueba = false;
  q1 = 0.0;
}

void iniciar(float valor) {                          // comando q1: inicia el escalon
  q1 = constrain(valor, 1, 60);
  T0 = lab.T1;
  n = 0;
  Tm[n++] = (int16_t)(T0 * 100.0);
  u2 = lab.t;
  enPrueba = true;
}

void fin(float) {
  if (!enPrueba) return;
  identificar();
  terminar();
}

void detener(float) {
  terminar();
}

void setup() {
  lab.begin();                                       // 20 muestras por segundo; se guarda 1 cada 2 s
  lab.comando("q1", iniciar);
  lab.comando("fin", fin);
  lab.comando("stop", detener);

  // Se grafican 2 senales para que el Plotter del IDE se vea claro.
  // Quite el // de otra linea lab.senal para ver mas senales (y suba de nuevo).
  // lab.senal("T1", lab.T1);
  lab.senal("Q1", lab.Q1);
  lab.senal("dT1", dT1);
  // lab.senalSi("Kp_x100", kpV, 1.0, &listo);
  // lab.senalSi("tau", tauV, 1.0, &listo);
  // lab.senalSi("theta", thetaV, 1.0, &listo);

  lab.programar(30, "q1 40");                        // escalon
  lab.programar(570, "fin");                         // calcula el modelo
}

void loop() {
  if (!lab.tick()) return;
  dT1 = lab.T1 - T0;
  if (enPrueba && lab.cada(u2, 2.0)) {               // una muestra guardada cada 2 s
    if (n < NMAX) Tm[n++] = (int16_t)(lab.T1 * 100.0);
    if (lab.bloqueado || n >= NMAX) {                // limite o memoria llena
      identificar();
      terminar();
    }
  }
  lab.calentador1(q1);
  lab.graficar();
}
