// P5: control ON-OFF con histeresis de los dos calentadores
#include <TPICLab.h>

TPICLab lab;
float sp1 = 45.0;                     // referencia de T1 (C)
float sp2 = 40.0;                     // referencia de T2 (C)
float h = 0.5;                        // banda de histeresis (C)
float lazo1 = 1.0;                    // 1: lazo activo, 0: apagado
float lazo2 = 1.0;
float uc = 0.0;                       // ultimo instante de control

float onoff(float T, float sp, float qPrev) {
  if (T < sp - h) return 100.0;
  if (T > sp + h) return 0.0;
  return qPrev;                                      // dentro de la banda: sin cambio
}

void detener(float) {
  lazo1 = 0;
  lazo2 = 0;
}

void setup() {
  lab.begin();                        // 20 muestras por segundo
  lab.parametro("sp1", sp1, 20, 60);
  lab.parametro("sp2", sp2, 20, 60);
  lab.parametro("h", h, 0, 5);
  lab.parametro("lazo1", lazo1, 0, 1);
  lab.parametro("lazo2", lazo2, 0, 1);
  lab.comando("stop", detener);

  // Se grafican 2 senales para que el Plotter del IDE se vea claro.
  // Quite el // de otra linea lab.senal para ver mas senales (y suba de nuevo).
  lab.senal("T1", lab.T1);
  lab.senal("SP1", sp1);
  // lab.senal("Q1/10", lab.Q1, 0.1);
  // lab.senal("T2", lab.T2);
  // lab.senal("SP2", sp2);
  // lab.senal("Q2/10", lab.Q2, 0.1);

  lab.programar(300, "h 2");          // banda grande
  lab.programar(480, "h 0.2");        // banda chica
  lab.programar(660, "sp1 40");       // cambio de referencia
  lab.programar(840, "lazo2 0");      // se apaga el lazo 2
  lab.programar(960, "stop");
}

void loop() {
  if (!lab.tick()) return;
  if (lab.cada(uc, 1.0)) {            // el control corre una vez por segundo
    float q1 = lazo1 ? onoff(lab.T1, sp1, lab.Q1) : 0.0;
    float q2 = lazo2 ? onoff(lab.T2, sp2, lab.Q2) : 0.0;
    lab.calentador1(q1);
    lab.calentador2(q2);
  }
  lab.graficar();
}
