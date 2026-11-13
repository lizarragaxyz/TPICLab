// TPICLab.h: biblioteca de la TPIC-Lab V1
// Mide, calienta, grafica en el Serial Plotter, lee comandos y corre un programa
// automatico.
#ifndef TPICLAB_H
#define TPICLAB_H
#include <Arduino.h>
#include <string.h>
#include <stdlib.h>

// Pines de la TPIC-Lab V1
const int PIN_T1  = A0;               // sensor TMP1
const int PIN_T2  = A2;               // sensor TMP2
const int PIN_Q1  = 3;                // calentador 1 (PWM)
const int PIN_Q2  = 5;                // calentador 2 (PWM)
const int PIN_LED = 9;                // LED HOT (PWM)

typedef void (*Accion)(float valor);  // funcion que atiende un comando

class TPICLab {
 public:
  // ---- Datos que puede leer el sketch ----
  float T1 = 0.0;                     // temperatura 1 (C)
  float T2 = 0.0;                     // temperatura 2 (C)
  float Q1 = 0.0;                     // potencia aplicada al calentador 1 (%)
  float Q2 = 0.0;                     // potencia aplicada al calentador 2 (%)
  float t = 0.0;                      // tiempo desde el arranque (s)
  float dt = 1.0;                     // periodo de control (s)
  float limite = 70.0;                // corte de seguridad (C)
  float muestras = 10.0;              // lecturas promediadas en cada medicion
  bool ledAuto = true;                // el LED se enciende solo si hay calentadores
  bool bloqueado = false;             // true despues de superar el limite
  bool mensajes = false;              // true: avisos en texto (se leen en el Monitor Serie)
  float filtro = 1.0;                 // constante de tiempo del filtro de temperatura (s); 0 = sin filtro

  // ---- Arranque y ciclo ----
  void begin(unsigned long periodoMs = 50) {
    analogReference(EXTERNAL);
    Serial.begin(115200);
    periodo = periodoMs;
    dt = periodoMs / 1000.0;
    apagar();
    ultimo = millis();
    if (mensajes) Serial.println(F("# TPIC-Lab V1 lista."));
  }

  // Atiende comandos y devuelve true una vez por periodo, con T1 y T2 recien medidas
  bool tick() {
    leerSerie();
    if (millis() - ultimo < periodo) return false;
    ultimo += periodo;
    t += dt;
    correrPrograma();
    float n1 = leerT(PIN_T1);
    float n2 = leerT(PIN_T2);
    if (primera) {                                  // primera medicion: sin filtro
      T1 = n1;
      T2 = n2;
      primera = false;
    } else {                                        // filtro exponencial: suaviza los escalones del ADC
      float a = filtro / (filtro + dt);              // peso de la medicion anterior
      T1 = a * T1 + (1.0 - a) * n1;
      T2 = a * T2 + (1.0 - a) * n2;
    }
    if (!bloqueado && (T1 > limite || T2 > limite)) {
      bloqueado = true;
      apagar();
    }
    return true;
  }

  // true cada "segundos" s (para tareas mas lentas que el muestreo, como el control)
  bool cada(float &ultimo, float segundos) {
    if (t - ultimo >= segundos - 0.001) {
      ultimo = t;
      return true;
    }
    return false;
  }

  // ---- Calentadores y LED (0 a 100 %) ----
  // El 100 % es 90 % (Q1) o 50 % (Q2) de ciclo util.
  void calentador1(float pct) {
    Q1 = bloqueado ? 0.0 : constrain(pct, 0, 100);
    analogWrite(PIN_Q1, (int)(Q1 * 2.295));
    actualizarLed();
  }
  void calentador2(float pct) {
    Q2 = bloqueado ? 0.0 : constrain(pct, 0, 100);
    analogWrite(PIN_Q2, (int)(Q2 * 1.275));
    actualizarLed();
  }
  void ledHot(float pct) {
    analogWrite(PIN_LED, (int)(constrain(pct, 0, 100) * 0.5));
  }
  void apagar() {
    Q1 = 0.0;
    Q2 = 0.0;
    analogWrite(PIN_Q1, 0);
    analogWrite(PIN_Q2, 0);
    analogWrite(PIN_LED, 0);
  }

  // Temperatura en C: promedio de "muestras" lecturas del TMP36 (10 mV/C, 500 mV a 0 C)
  float leerT(int pin) {
    int n = (int)muestras;
    float suma = 0.0;
    for (int i = 0; i < n; i++) suma += analogRead(pin);
    float mV = (suma / n) * (3300.0 / 1024.0);
    return (mV - 500.0) / 10.0;
  }

  // ---- Registro de parametros, comandos y senales ----
  // Parametro: se cambia escribiendo "nombre valor"; queda limitado a [minimo, maximo]
  void parametro(const char *nombre, float &variable, float minimo, float maximo) {
    if (nParam < MAX_PARAM) {
      param[nParam].nombre = nombre;
      param[nParam].var = &variable;
      param[nParam].minimo = minimo;
      param[nParam].maximo = maximo;
      nParam++;
    } else {
      Serial.println(F("# error: demasiados parametros"));
    }
  }
  // Comando: se ejecuta al escribir "nombre" o "nombre valor"
  void comando(const char *nombre, Accion accion) {
    if (nCmd < MAX_CMD) {
      cmd[nCmd].nombre = nombre;
      cmd[nCmd].accion = accion;
      nCmd++;
    } else {
      Serial.println(F("# error: demasiados comandos"));
    }
  }
  // Senal: aparece en el Serial Plotter (valor * escala)
  void senal(const char *nombre, float &variable, float escala = 1.0) {
    senalSi(nombre, variable, escala, 0);
  }
  // Senal que solo se grafica mientras "visible" sea distinto de cero
  void senalSi(const char *nombre, float &variable, float escala, float *visible) {
    if (nSen < MAX_SEN) {
      sen[nSen].nombre = nombre;
      sen[nSen].var = &variable;
      sen[nSen].escala = escala;
      sen[nSen].visible = visible;
      nSen++;
    } else {
      Serial.println(F("# error: demasiadas senales"));
    }
  }
  // Programa automatico: a los "segundo" s se ejecuta el comando "nombre valor"
  void programar(unsigned int segundo, const char *comandoTexto) {
    if (nProg < MAX_PROG) {
      prog[nProg].segundo = segundo;
      prog[nProg].texto = comandoTexto;
      prog[nProg].hecho = false;
      nProg++;
    } else {
      Serial.println(F("# error: demasiadas lineas de programa"));
    }
  }

  // Envia todas las senales registradas al Serial Plotter
  void graficar() {
    bool primera = true;
    for (int i = 0; i < nSen; i++) {
      if (sen[i].visible && *sen[i].visible == 0.0) continue;
      if (!primera) Serial.print(',');
      Serial.print(sen[i].nombre);
      Serial.print(':');
      Serial.print(*sen[i].var * sen[i].escala, 2);
      primera = false;
    }
    if (bloqueado) {                              // aviso visible en la grafica
      Serial.print(F(",LIMITE:100"));
    }
    Serial.println();
  }

 private:
  static const int MAX_PARAM = 8;
  static const int MAX_CMD = 6;
  static const int MAX_SEN = 8;
  static const int MAX_PROG = 12;
  static const int LINEA = 32;

  struct Param { const char *nombre; float *var; float minimo; float maximo; };
  struct Cmd   { const char *nombre; Accion accion; };
  struct Sen   { const char *nombre; float *var; float escala; float *visible; };
  struct Prog  { unsigned int segundo; const char *texto; bool hecho; };

  Param param[MAX_PARAM];
  Cmd cmd[MAX_CMD];
  Sen sen[MAX_SEN];
  Prog prog[MAX_PROG];
  int nParam = 0, nCmd = 0, nSen = 0, nProg = 0;
  unsigned long periodo = 1000;
  unsigned long ultimo = 0;
  bool primera = true;
  char linea[LINEA];
  int nLinea = 0;

  void actualizarLed() {
    if (ledAuto) ledHot(Q1 + Q2 > 0 ? 100 : 0);
  }

  void leerSerie() {
    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        if (nLinea > 0) {
          linea[nLinea] = 0;
          nLinea = 0;
          procesar(linea);
        }
      } else if (nLinea < LINEA - 1) {
        linea[nLinea++] = c;
      }
    }
  }

  void correrPrograma() {
    char buf[LINEA];
    for (int i = 0; i < nProg; i++) {
      if (!prog[i].hecho && t >= prog[i].segundo) {
        prog[i].hecho = true;
        strncpy(buf, prog[i].texto, LINEA - 1);
        buf[LINEA - 1] = 0;
        procesar(buf);
      }
    }
  }

  static bool igual(const char *a, const char *b) {
    while (*a && *b) {
      char x = (*a >= 'A' && *a <= 'Z') ? *a + 32 : *a;
      char y = (*b >= 'A' && *b <= 'Z') ? *b + 32 : *b;
      if (x != y) return false;
      a++;
      b++;
    }
    return *a == 0 && *b == 0;
  }

  // Interpreta "nombre" o "nombre valor"
  void procesar(char *texto) {
    char *sp = strchr(texto, ' ');
    float v = 0.0;
    if (sp) {
      *sp = 0;
      v = atof(sp + 1);
    }
    for (int i = 0; i < nParam; i++) {
      if (igual(texto, param[i].nombre)) {
        *param[i].var = constrain(v, param[i].minimo, param[i].maximo);
        if (mensajes) {
          Serial.print(F("# "));
          Serial.print(param[i].nombre);
          Serial.print(F(" = "));
          Serial.println(*param[i].var);
        }
        return;
      }
    }
    for (int i = 0; i < nCmd; i++) {
      if (igual(texto, cmd[i].nombre)) {
        cmd[i].accion(v);
        return;
      }
    }
    if (igual(texto, "liberar")) {
      bloqueado = false;
      if (mensajes) Serial.println(F("# calentadores liberados"));
    } else if (igual(texto, "ayuda")) {
      ayuda();
    } else {
      if (mensajes) Serial.println(F("# comando desconocido. Escriba ayuda."));
    }
  }

  void ayuda() {
    Serial.println(F("# Parametros (nombre valor):"));
    for (int i = 0; i < nParam; i++) {
      Serial.print(F("#   "));
      Serial.print(param[i].nombre);
      Serial.print(F("  ["));
      Serial.print(param[i].minimo);
      Serial.print(F(" a "));
      Serial.print(param[i].maximo);
      Serial.print(F("]  ahora = "));
      Serial.println(*param[i].var);
    }
    Serial.print(F("# Comandos:"));
    for (int i = 0; i < nCmd; i++) {
      Serial.print(' ');
      Serial.print(cmd[i].nombre);
    }
    Serial.println(F(" liberar ayuda"));
  }
};

// Controlador PID con anti-windup (se explica en la practica P6)
struct PID {
  float kc;                           // ganancia (%/C)
  float ti;                           // tiempo integral (s)
  float td;                           // tiempo derivativo (s)
  float ierr;                         // integral acumulada
  float pvPrev;                       // medicion anterior
  float P, I, D;                      // terminos de la ultima salida

  PID(float kc_, float ti_, float td_) {
    kc = kc_;  ti = ti_;  td = td_;
    ierr = 0.0;  pvPrev = 0.0;  P = 0.0;  I = 0.0;  D = 0.0;
  }

  void reiniciar(float pv) {
    ierr = 0.0;
    pvPrev = pv;
  }

  // sp: referencia, pv: medicion, dt: periodo (s). Devuelve 0 a 100 %.
  float paso(float sp, float pv, float dt) {
    float error = sp - pv;
    ierr += (kc / ti) * error * dt;
    P = kc * error;
    I = ierr;
    D = -kc * td * (pv - pvPrev) / dt;            // derivada de la medicion
    pvPrev = pv;
    float op = P + I + D;
    if (op < 0.0 || op > 100.0) {                  // anti-windup
      ierr -= (kc / ti) * error * dt;
      op = constrain(op, 0.0, 100.0);
    }
    return op;
  }
};

#endif
