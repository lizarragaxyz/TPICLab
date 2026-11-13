# TPICLab

Biblioteca de Arduino para la **TPIC-Lab V1** (Thermal Plant Identification & Control Lab), de Oak-Robotics.

Mide los sensores de temperatura, maneja los calentadores y el LED, grafica en el Serial Plotter del Arduino IDE,
lee comandos por el puerto serie y corre un programa automatico. Incluye 10 practicas como ejemplos.

## Requisitos

| Que | Detalle |
|---|---|
| Hardware | TPIC-Lab V1 montada sobre un **Arduino Uno** (R3), con su fuente de 5 V CC / 2 A (centro positivo) y su cable USB |
| Computadora | Windows, macOS o Linux con un puerto USB libre |
| Software | **Arduino IDE 1.8.x** (probado con la 1.8.57). La version 2.x no se ha probado |
| Controlador USB | Solo si el Arduino es clon con chip CH340: instalar su controlador |
| Biblioteca | Esta: `TPICLab` |

Placa en el IDE: *Arduino Uno*. Serial Plotter a **115200 baudios**.

## Descarga

- Ultima version (ZIP): <https://github.com/lizarragaxyz/TPICLab/archive/refs/heads/main.zip>
- Versiones publicadas: <https://github.com/lizarragaxyz/TPICLab/releases>
- Clonar: `git clone https://github.com/lizarragaxyz/TPICLab.git`

## Instalacion

**Opcion A (recomendada): desde el IDE**

1. Descargue el ZIP (enlace de arriba). No lo descomprima.
2. En el Arduino IDE: *Sketch* -> *Include Library* -> *Add .ZIP Library...*
3. Elija el ZIP descargado.
4. Aparece el mensaje "Library added to your libraries".

**Opcion B: a mano**

1. Descomprima el ZIP.
2. Copie la carpeta dentro de `Documentos\Arduino\libraries\` y llamela `TPICLab`.
3. Cierre y vuelva a abrir el Arduino IDE.

La carpeta debe contener directamente `library.properties` y la carpeta `src`. Si ya tenia una carpeta `TPICLab`
antigua, borrela antes de instalar para no mezclar versiones.

## Comprobar la instalacion

*File* -> *Examples* -> *TPICLab*: deben aparecer `Diagnostico` y las practicas `P01_Verificacion` a `P10_Balance`.
Abra `P01_Verificacion` y presione *Verify*: debe compilar sin errores.

## Uso minimo

```cpp
#include <TPICLab.h>

TPICLab lab;
float q1 = 0.0;

void setup() {
  lab.begin();
  lab.parametro("q1", q1, 0, 60);     // comando "q1 valor"
  lab.senal("T1", lab.T1);            // grafica T1
  lab.programar(10, "q1 30");         // a los 10 s: q1 30
}

void loop() {
  if (!lab.tick()) return;            // 20 veces por segundo
  lab.calentador1(q1);
  lab.graficar();
}
```

## Practicas

| Num. | Practica | Nivel | Ejemplo |
|---|---|---|---|
| P1 | Verificacion de la TPIC-Lab V1 | Basico | `P01_Verificacion` |
| P2 | Lectura de temperatura y ruido | Basico | `P02_Ruido` |
| P3 | Respuesta al escalon | Basico | `P03_Escalon` |
| P4 | Acoplamiento termico | Basico | `P04_Acoplamiento` |
| P5 | Control ON-OFF | Intermedio | `P05_ON_OFF` |
| P6 | Control PID | Intermedio | `P06_PID` |
| P7 | Identificacion del modelo | Intermedio | `P07_Identificacion` |
| P8 | Sintonia PI por modelo (SIMC) | Avanzado | `P08_SIMC` |
| P9 | PID de dos calentadores | Avanzado | `P09_PID_dual` |
| P10 | Modelo por balance de energia | Avanzado | `P10_Balance` |

Los documentos de cada practica (PDF) estan en el material del taller, no en esta biblioteca.

## Estructura

```text
TPICLab/
  library.properties   datos de la biblioteca para el IDE
  keywords.txt         resaltado de palabras en el IDE
  src/TPICLab.h        la biblioteca
  examples/            Diagnostico y P01 a P10
```

## Seguridad

Los calentadores y los disipadores se calientan mucho. No los toque durante ni justo despues de cada practica y no
deje la TPIC-Lab V1 funcionando sin vigilancia. La biblioteca corta los calentadores si una temperatura pasa de
`limite` (70 C por defecto).

## Licencia

MIT. Vea [LICENSE](LICENSE).
