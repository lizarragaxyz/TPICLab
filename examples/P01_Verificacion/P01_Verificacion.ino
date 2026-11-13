// P1: verificacion de la TPIC-Lab V1 (LED, calentadores y sensores)
#include <TPICLab.h>

TPICLab lab;
float led = 0.0;                      // LED HOT (%)
float q1 = 0.0;                       // calentador 1 (%)
float q2 = 0.0;                       // calentador 2 (%)
float T10 = 0.0;                      // temperaturas iniciales
float T20 = 0.0;
float dT1 = 0.0;                      // aumento de cada sensor
float dT2 = 0.0;
bool primera = true;

void detener(float) {
  led = 0;
  q1 = 0;
  q2 = 0;
}

void setup() {
  lab.begin();                        // 20 lecturas por segundo
  lab.ledAuto = false;                // aqui el LED lo manda el parametro "led"

  lab.parametro("led", led, 0, 100);
  lab.parametro("q1", q1, 0, 30);     // maximo 30 % en esta practica
  lab.parametro("q2", q2, 0, 30);
  lab.comando("stop", detener);

  // Se grafican 2 senales para que el Plotter del IDE se vea claro.
  // Quite el // de otra linea lab.senal para ver mas senales (y suba de nuevo).
  lab.senal("dT1", dT1);
  lab.senal("dT2", dT2);
  // lab.senal("Q1/10", lab.Q1, 0.1);    // potencias a escala menor para ver las temperaturas
  // lab.senal("Q2/10", lab.Q2, 0.1);

  // Programa automatico (puede interrumpirlo con comandos)
  lab.programar(3, "led 100");
  lab.programar(8, "led 0");
  lab.programar(10, "q1 20");
  lab.programar(70, "q1 0");
  lab.programar(75, "q2 20");
  lab.programar(135, "stop");
}

void loop() {
  if (!lab.tick()) return;            // veinte veces por segundo
  if (primera) {                      // temperatura inicial de cada sensor
    T10 = lab.T1;
    T20 = lab.T2;
    primera = false;
  }
  dT1 = lab.T1 - T10;
  dT2 = lab.T2 - T20;
  lab.calentador1(q1);
  lab.calentador2(q2);
  lab.ledHot(led);
  lab.graficar();
}
