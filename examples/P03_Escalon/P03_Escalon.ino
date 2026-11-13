// P3: respuesta al escalon en lazo abierto
#include <TPICLab.h>

TPICLab lab;
float q1 = 0.0;                       // calentador 1 (%)
float q2 = 0.0;                       // calentador 2 (%)
float T0 = 0.0;                       // temperatura de referencia
float dT1 = 0.0;                      // aumento de T1 desde T0

void tomarBase(float) {
  T0 = lab.T1;
}

void detener(float) {
  q1 = 0;
  q2 = 0;
}

void setup() {
  lab.begin();                        // 20 muestras por segundo
  lab.parametro("q1", q1, 0, 60);
  lab.parametro("q2", q2, 0, 60);
  lab.comando("base", tomarBase);
  lab.comando("stop", detener);

  // Se grafican 2 senales para que el Plotter del IDE se vea claro.
  // Quite el // de otra linea lab.senal para ver mas senales (y suba de nuevo).
  // lab.senal("T1", lab.T1);
  // lab.senal("T2", lab.T2);
  lab.senal("Q1", lab.Q1);
  lab.senal("dT1", dT1);

  lab.programar(30, "base");          // referencia antes del escalon
  lab.programar(31, "q1 40");         // escalon
  lab.programar(600, "stop");         // enfriamiento
}

void loop() {
  if (!lab.tick()) return;
  dT1 = lab.T1 - T0;
  lab.calentador1(q1);
  lab.calentador2(q2);
  lab.graficar();
}
