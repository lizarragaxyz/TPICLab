// P8: sintonia PI por modelo (SIMC) y comparacion con la sintonia manual
#include <TPICLab.h>

TPICLab lab;
PID pid(10.0, 50.0, 1.0);             // sintonia manual de la practica P6
float sp = 30.0;                      // referencia (C)
float Kp = 0.5;                       // modelo FOPDT de la practica P7
float tau = 120.0;
float theta = 10.0;
float tauc = 30.0;                    // constante de tiempo deseada en lazo cerrado (s)
float iae = 0.0;
float activo = 1.0;
bool primera = true;
float uc = 0.0;                       // ultimo instante de control

void aplicarSIMC(float) {
  pid.kc = tau / (Kp * (tauc + theta));
  float limite = 4.0 * (tauc + theta);
  pid.ti = tau < limite ? tau : limite;
  pid.td = 0.0;
  pid.reiniciar(lab.T1);
  iae = 0;
  if (lab.mensajes) {
    Serial.print(F("SIMC: Kc = "));  Serial.print(pid.kc, 2);
    Serial.print(F("  ti = "));      Serial.println(pid.ti, 1);
  }
}

void sintoniaManual(float) {
  pid.kc = 10;
  pid.ti = 50;
  pid.td = 1;
  pid.reiniciar(lab.T1);
  iae = 0;
}

void mostrarIAE(float) {
  if (lab.mensajes) {
    Serial.print(F("IAE = "));
    Serial.println(iae, 0);
  }
}

void detener(float) {
  activo = 0;
}

void setup() {
  lab.begin();                        // 20 muestras por segundo
  lab.parametro("sp", sp, 20, 60);
  lab.parametro("kp", Kp, 0.01, 10);
  lab.parametro("tau", tau, 10, 1000);
  lab.parametro("theta", theta, 0, 200);
  lab.parametro("tauc", tauc, 5, 300);
  lab.comando("simc", aplicarSIMC);
  lab.comando("manual", sintoniaManual);
  lab.comando("iae", mostrarIAE);
  lab.comando("stop", detener);

  // Se grafican 2 senales para que el Plotter del IDE se vea claro.
  // Quite el // de otra linea lab.senal para ver mas senales (y suba de nuevo).
  lab.senal("T1", lab.T1);
  lab.senal("SP", sp);
  // lab.senal("Q1/2", lab.Q1, 0.5);
  // lab.senal("Kc", pid.kc);
  // lab.senal("IAE/100", iae, 0.01);

  lab.programar(1, "manual");         // primero la sintonia de P6
  lab.programar(2, "sp 45");
  lab.programar(300, "iae");
  lab.programar(301, "sp 30");        // enfria
  lab.programar(540, "simc");         // despues la sintonia SIMC
  lab.programar(541, "sp 45");
  lab.programar(840, "iae");
  lab.programar(841, "stop");
}

void loop() {
  if (!lab.tick()) return;
  if (primera) {
    pid.reiniciar(lab.T1);
    primera = false;
  }
  if (lab.cada(uc, 1.0)) {                           // el PI corre una vez por segundo
    float op = pid.paso(sp, lab.T1, 1.0);
    if (activo == 0) op = 0.0;
    iae += fabs(sp - lab.T1) * 1.0;
    lab.calentador1(op);
  }
  lab.graficar();
}
