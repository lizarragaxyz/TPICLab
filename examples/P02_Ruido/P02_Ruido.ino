// P2: lectura de temperatura y ruido de los sensores
#include <TPICLab.h>

TPICLab lab;
const int N = 30;                     // ventana para la media y la desviacion
float buf[N];
int idx = 0;
int llenos = 0;
float nPrev = 0.0;
float media = 0.0;
float desv10 = 0.0;                   // desviacion estandar x 10

void setup() {
  lab.begin();                        // 20 lecturas por segundo
  lab.filtro = 0.0;                   // sin filtro: aqui queremos ver el ruido
  lab.parametro("n", lab.muestras, 1, 100);   // muestras promediadas por lectura

  // Se grafican 2 senales para que el Plotter del IDE se vea claro.
  // Quite el // de otra linea lab.senal para ver mas senales (y suba de nuevo).
  lab.senal("T1", lab.T1);
  // lab.senal("T2", lab.T2);
  lab.senal("media", media);
  // lab.senal("desv_x10", desv10);

  lab.programar(60, "n 1");
  lab.programar(120, "n 50");
  lab.programar(180, "n 10");
}

void loop() {
  if (!lab.tick()) return;
  if (lab.muestras != nPrev) {                // cambio n: reinicia la ventana
    nPrev = lab.muestras;
    llenos = 0;
  }
  buf[idx] = lab.T1;
  idx = (idx + 1) % N;
  if (llenos < N) llenos++;

  float suma = 0.0;
  for (int i = 0; i < llenos; i++) suma += buf[i];
  media = suma / llenos;
  float var = 0.0;
  for (int i = 0; i < llenos; i++) var += sq(buf[i] - media);
  desv10 = 10.0 * sqrt(var / llenos);
  lab.graficar();
}
