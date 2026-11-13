// P10: modelo por balance de energia calculado en el Arduino junto a la planta real
#include <TPICLab.h>

TPICLab lab;

// Parametros del modelo
float U = 10.0;                       // convec. (W/m2 K)
float alpha = 0.01;                   // potencia por cada 1 % de calentador (W)
const float m = 0.004;                // masa (kg)
const float Cp = 500.0;               // calor especifico (J/kg K)
const float A = 12.0e-4;              // area (m2)
const float emis = 0.9;               // emisividad
const float sigma = 5.67e-8;          // Stefan-Boltzmann

float q1 = 0.0;                       // calentador 1 (%)
float Ta = 296.0;                     // temperatura ambiente (K)
float Tm = 296.0;                     // temperatura del modelo (K)
float TmC = 23.0;                     // temperatura del modelo (C)
float error = 0.0;                    // T1 - modelo

void reiniciarModelo(float) {
  Ta = lab.T1 + 273.15;
  Tm = Ta;
}

void detener(float) {
  q1 = 0;
}

void setup() {
  lab.begin();                        // 20 muestras por segundo
  lab.parametro("q1", q1, 0, 60);
  lab.parametro("u", U, 1, 100);
  lab.parametro("alpha", alpha, 0.001, 0.1);
  lab.comando("reset", reiniciarModelo);
  lab.comando("stop", detener);

  // Se grafican 2 senales para que el Plotter del IDE se vea claro.
  // Quite el // de otra linea lab.senal para ver mas senales (y suba de nuevo).
  lab.senal("T1", lab.T1);
  lab.senal("Tmodelo", TmC);
  // lab.senal("Q1/2", lab.Q1, 0.5);
  // lab.senal("error", error);

  lab.programar(30, "reset");
  lab.programar(31, "q1 40");
  lab.programar(600, "stop");
}

void loop() {
  if (!lab.tick()) return;
  lab.calentador1(q1);

  // Un paso de Euler del balance de energia
  float conv = U * A * (Ta - Tm);
  float rad = emis * sigma * A * (Ta * Ta * Ta * Ta - Tm * Tm * Tm * Tm);
  float dTdt = (conv + rad + alpha * lab.Q1) / (m * Cp);
  Tm += dTdt * lab.dt;

  TmC = Tm - 273.15;
  error = lab.T1 - TmC;
  lab.graficar();
}
