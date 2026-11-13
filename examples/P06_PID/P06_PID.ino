// P6: control PID de T1 con el calentador 1
#include <TPICLab.h>

TPICLab lab;
float sp = 30.0;                      // referencia (C)
float kc = 10.0;                      // ganancia (%/C)
float ti = 50.0;                      // tiempo integral (s)
float td = 1.0;                       // tiempo derivativo (s)
float perfil = 0.0;                   // 1: sigue el perfil de referencia
float terminos = 0.0;                 // 1: grafica P, I y D
float activo = 1.0;                   // 0: calentador apagado

float ierr = 0.0;                     // integral acumulada
float Tprev = 0.0;                    // medicion anterior
bool primera = true;
float iae = 0.0;                      // integral del error absoluto (C*s)
float uc = 0.0;                       // ultimo instante de control
float op = 0.0;                       // salida del PID (%)
float P = 0.0;
float D = 0.0;

float referencia(float s) {           // perfil de 15 minutos
  if (s < 60)  return 25.0;
  if (s < 360) return 50.0;
  if (s < 660) return 30.0;
  return 40.0;
}

void reiniciar(float) {
  ierr = 0;
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
  lab.parametro("kc", kc, 0, 100);
  lab.parametro("ti", ti, 1, 1000);
  lab.parametro("td", td, 0, 100);
  lab.parametro("perfil", perfil, 0, 1);
  lab.parametro("terminos", terminos, 0, 1);
  lab.parametro("activo", activo, 0, 1);
  lab.comando("reset", reiniciar);
  lab.comando("iae", mostrarIAE);
  lab.comando("stop", detener);

  // Se grafican 2 senales para que el Plotter del IDE se vea claro.
  // Quite el // de otra linea lab.senal para ver mas senales (y suba de nuevo).
  lab.senal("T1", lab.T1);
  lab.senal("SP", sp);
  // lab.senal("Q1/2", lab.Q1, 0.5);
  // lab.senalSi("P/10", P, 0.1, &terminos);
  // lab.senalSi("I/10", ierr, 0.1, &terminos);
  // lab.senalSi("D/10", D, 0.1, &terminos);
  // lab.senal("IAE/100", iae, 0.01);

  lab.programar(10, "sp 40");
  lab.programar(300, "sp 30");
  lab.programar(500, "kc 20");
  lab.programar(700, "kc 5");
  lab.programar(900, "kc 10");
  lab.programar(900, "terminos 1");
  lab.programar(1100, "stop");
}

void loop() {
  if (!lab.tick()) return;
  if (primera) {
    Tprev = lab.T1;
    primera = false;
  }
  if (lab.cada(uc, 1.0)) {                           // el PID corre una vez por segundo
    if (perfil != 0) sp = referencia(lab.t);
    float error = sp - lab.T1;

    P = kc * error;
    ierr += (kc / ti) * error * 1.0;
    D = -kc * td * (lab.T1 - Tprev) / 1.0;           // derivada de la medicion
    op = P + ierr + D;
    if (op < 0.0 || op > 100.0) {                    // anti-windup
      ierr -= (kc / ti) * error * 1.0;
      op = constrain(op, 0.0, 100.0);
    }
    if (activo == 0) op = 0.0;
    Tprev = lab.T1;
    iae += fabs(error) * 1.0;
    lab.calentador1(op);
  }
  lab.graficar();
}
