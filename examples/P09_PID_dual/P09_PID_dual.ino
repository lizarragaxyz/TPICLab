// P9: control PID de los dos calentadores al mismo tiempo
#include <TPICLab.h>

TPICLab lab;
PID pid1(10.0, 50.0, 1.0);            // lazo 1
PID pid2(20.0, 50.0, 1.0);            // lazo 2: mas ganancia (calentador mas debil)
float sp1 = 45.0;                     // referencias (C)
float sp2 = 35.0;
float ti = 50.0;
float td = 1.0;
float activo = 1.0;
bool primera = true;
float uc = 0.0;                       // ultimo instante de control

void intercambiar(float) {
  float x = sp1;
  sp1 = sp2;
  sp2 = x;
}

void detener(float) {
  activo = 0;
}

void setup() {
  lab.begin();                        // 20 muestras por segundo
  lab.parametro("sp1", sp1, 20, 60);
  lab.parametro("sp2", sp2, 20, 60);
  lab.parametro("kc1", pid1.kc, 0, 100);
  lab.parametro("kc2", pid2.kc, 0, 100);
  lab.parametro("ti", ti, 1, 1000);
  lab.parametro("td", td, 0, 100);
  lab.comando("swap", intercambiar);
  lab.comando("stop", detener);

  // Se grafican 2 senales para que el Plotter del IDE se vea claro.
  // Quite el // de otra linea lab.senal para ver mas senales (y suba de nuevo).
  lab.senal("T1", lab.T1);
  // lab.senal("SP1", sp1);
  lab.senal("T2", lab.T2);
  // lab.senal("SP2", sp2);
  // lab.senal("Q1/2", lab.Q1, 0.5);
  // lab.senal("Q2/2", lab.Q2, 0.5);

  lab.programar(300, "swap");         // se invierten las referencias
  lab.programar(600, "kc2 10");       // mismo Kc en los dos lazos
  lab.programar(900, "swap");
  lab.programar(1200, "stop");
}

void loop() {
  if (!lab.tick()) return;
  if (primera) {
    pid1.reiniciar(lab.T1);
    pid2.reiniciar(lab.T2);
    primera = false;
  }
  if (lab.cada(uc, 1.0)) {                           // los PID corren una vez por segundo
    pid1.ti = ti;  pid2.ti = ti;
    pid1.td = td;  pid2.td = td;
    float q1 = pid1.paso(sp1, lab.T1, 1.0);
    float q2 = pid2.paso(sp2, lab.T2, 1.0);
    if (activo == 0) {
      q1 = 0.0;
      q2 = 0.0;
    }
    lab.calentador1(q1);
    lab.calentador2(q2);
  }
  lab.graficar();
}
