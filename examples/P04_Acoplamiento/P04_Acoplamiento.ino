// P4: coeficiente de acoplamiento termico entre los dos calentadores
#include <TPICLab.h>

TPICLab lab;
float q1 = 0.0;                       // calentador 1 (%)
float q2 = 0.0;                       // calentador 2 (%)
float T10 = 0.0;                      // temperaturas base
float T20 = 0.0;
float dT1 = 0.0;                      // aumentos desde la base
float dT2 = 0.0;
float k100 = 0.0;                     // acoplamiento x 100

void tomarBase(float) {
  T10 = lab.T1;
  T20 = lab.T2;
}

void detener(float) {
  q1 = 0;
  q2 = 0;
}

void setup() {
  lab.begin();
  lab.parametro("q1", q1, 0, 60);
  lab.parametro("q2", q2, 0, 60);
  lab.comando("base", tomarBase);
  lab.comando("stop", detener);

  // Se grafican 2 senales para que el Plotter del IDE se vea claro.
  // Quite el // de otra linea lab.senal para ver mas senales (y suba de nuevo).
  lab.senal("dT1", dT1);
  lab.senal("dT2", dT2);
  // lab.senal("Q1/2", lab.Q1, 0.5);
  // lab.senal("k_x100", k100);

  lab.programar(30, "base");
  lab.programar(31, "q1 60");
  lab.programar(500, "stop");
}

void loop() {
  if (!lab.tick()) return;
  dT1 = lab.T1 - T10;
  dT2 = lab.T2 - T20;
  float k = 0.0;                                     // dT vecino / dT propio
  if (q1 > 0 && q2 == 0 && dT1 > 2.0) k = dT2 / dT1;
  if (q2 > 0 && q1 == 0 && dT2 > 2.0) k = dT1 / dT2;
  k100 = k * 100.0;
  lab.calentador1(q1);
  lab.calentador2(q2);
  lab.graficar();
}
