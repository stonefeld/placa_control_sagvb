# Placa de Control para SAGVB.
Este es el código para la placa de control del sistema de estacionamiento de SAGVB.

## Tabla de contenidos
* [Requerimientos](#requerimientos)
  * [Software](#software)
  * [Hardware](#hardware)
* [Aclaraciones](#aclaraciones)
* [Instalación](#instalación)
* [Tareas](#tareas)
* [Imágenes](#imágenes)
* [Links](#links)

## Requerimientos
En esta sección les explico que necesitan para poder compilar el código.

### Software
* `Visual Studio Code`: Van a necesitar utilizar VSCode. Pueden usar cualquier otro, pero hay un plugin muy bien diseñado que se integra bien con VSCode.
* `PlatformIO IDE`: Es el plugin que les digo para VSCode.

### Hardware
* `Nodemcu-ESP32`: Es la placa ESP que vamos a utilizar.
* `Display LCD 16x2`: El display LCD, pero como vamos a utilizar el protocolo I2C para la comunicación con el ESP, en caso de no venir integrado necesitamos también:
  * `Adaptador I2C Display LCD PCF8754`: Que se suelda directamente a los pines del LCD.
* `Teclado Matricial 4x3`: Que en el código por el momento asumimos que es 4x4 para la prueba, pero cuando entendamos bien como funciona lo cambiamos a 4x3.
* `HR-SR04 Sensor de Distancia`

## Aclaraciones
Para poder compilar el código necesitan tener en cuenta los siguientes puntos:
* En el repositorio van a encontrar cinco carpetas:
  * [`.vscode`](https://github.com/stonefeld/placa_control_sagvb/tree/master/.vscode): Acá el único archivo que debería haber es el [`extensions.json`](https://github.com/stonefeld/placa_control_sagvb/tree/master/.vscode/extensions.json) que le informa a VSCode que estamos utilizando la extension de `platformio-ide`.
  * [`include`](https://github.com/stonefeld/placa_control_sagvb/tree/master/include): En esta carpeta van los header files (`.h`/`.hpp`) donde deberían ver un `README`, `LCDScanner.h`:
    * [`LCDScanner.h`](https://github.com/stonefeld/placa_control_sagvb/tree/master/include/LCDScanner.h): Es tan solo un simple header file con un namespace para diferenciar las funciones de otras funciones similares preexistentes, que en el caso ideal deberíamos utilizarlo una única vez para conocer el address del LCD para la comunicación I2C.
  * [`lib`](https://github.com/stonefeld/placa_control_sagvb/tree/master/lib): En esta carpeta es donde debemos escribir nuestras librerias personales para el proyecto. Por el momento debería haber una sola librería:
    * [`RDM6300`](https://github.com/stonefeld/placa_control_sagvb/tree/master/lib/RDM6300): El cual tiene la tarea de manejar el funcionamiento del lector de RFID. Para eso cree una clase para poder inicializar al lector de forma práctica y sencilla. Tiene verificación del código recibido ya que la estructura del mismo es la siguiente:
      ```
                  +----+--------------------------+----------+----+
      Estructura: | 02 | 10 ASCII Data Characters | Checksum | 03 |
                  +----+--------------------------+----------+----+
      Ejemplo:
        Tag ID: 62E3086CED
        Output data: 0x36, 0x32, 0x45, 0x33, 0x30, 0x38, 0x36, 0x43, 0x45, 0x44
        Checksum: (0x62) XOR (0xE3) XOR (0x08) XOR (0x6C) XOR (0xED) = 0x08
      ```
      Todo esto esta igualmente explicado en comentarios a través del código, por si quieren leerlo. Los únicos dos archivos que van a encontrar dentro de la librería son [`RDM6300.h`](https://github.com/stonefeld/placa_control_sagvb/tree/master/lib/RDM6300/RDM6300.h) y [`RDM6300.cpp`](https://github.com/stonefeld/placa_control_sagvb/tree/master/lib/RDM6300/RDM6300.cpp).
  * [`src`](https://github.com/stonefeld/placa_control_sagvb/tree/master/src): El único archivo que debería haber acá es el [`main.cpp`](https://github.com/stonefeld/placa_control_sagvb/tree/master/src/main.cpp) que, como en cualquier lenguaje basado en C, es el entry point de nuestro programa.
* Además de las carpetas, en el root del repositorio tenemos el [`.gitignore`](https://github.com/stonefeld/placa_control_sagvb/tree/master/.gitignore) y el [`platformio.ini`](https://github.com/stonefeld/placa_control_sagvb/tree/master/platformio.ini) que contiene la configuración de la plataforma para el proyecto donde tienen todas las especificaciones como la velocidad de comunicación serial, la velocidad de subida del programa, las librerias que utilizamos, el modelo de placa y el tipo de compilador.
  > Tengan en cuenta que cada vez que modifiquen el `platformio.ini`, la plataforma va a recompilar todo el proyecto de cero.

## Instalación
Para poder compilar el proyecto luego de haber hecho el `git clone` en la ubicación que querían, tan solo con abrir el VSCode y en *Projects & Configuration* deberían poder seleccionar *Add Existing* y elegir la carpeta donde se ubica el repositorio. Luego de eso el PlatformIO debería configurar las librerías necesarias para el funcionamiento del proyecto, si no es que las configura al momento del build.

Cuando tengan el proyecto abierto bajo PlatformIO, en el statusbar abajo del todo en VSCode debería aparecerles los íconos correspondientes para poder compilar, subir, limpiar y monitorear el proyecto. De todos modos pueden usar las mismas funcionalidades desde el apartado de extensiones de PlatformIO.

Tengan en cuenta que el proyecto lo dividí en dos secciones, ya que como vamos a utilizar comunicación por I2C para el LCD y necesitamos conocer el address. Por lo tanto, para evitar crear un proyecto nuevo, o tenerlo como un sketch aparte para subir, cree el siguiente define:
```cpp
// Descomentar para utilizar el sketch de analisis del address del lcd.
#define LCDSCANNER_ENABLED
```
Por lo tanto, cuando querramos compilar el código para conocer el address del LCD descomentamos esa línea de código. Una vez que lo conozcamos y lo hayamos reemplazado en el programa lo volvemos a comentar, y la sección del código correspondiente al mismo no será compilado para disminuir el espacio utilizado en la memoria flash.

El último punto a tomar en cuenta es el que respecta al uso de WiFi. Para esto deben agregar un archivo en la carpeta [`include`](https://github.com/stonefeld/placa_control_sagvb/tree/master/include) llamado `WiFiConfig.h` el cual debe contener las siguientes líneas de código:
```cpp
const char* SSID = "SSID";
const char* PASSWORD = "PASSWORD";
```
Está más que claro que en los strings correspondientes a cada variable deben escribir la información correspondiente a su red WiFi.
> Despreocupense que su información del WiFi no va a subirse a GitHub porque en el `.gitignore` aclaré que el archivo `WiFiConfig.h` lo ignore cuando haga los commits. Mientras no borren esas lineas del `.gitignore` y nombren al archivo exactamente como les dije no deberían preocuparse.

## Tareas
- [x] Configurar la plataforma.
- [x] Configurar el repositorio.
- [x] Soporte para WiFi.
- [ ] Soporte para realizar Requests HTTP por WiFi.
- [ ] Soporte para el socket TCP.
- [x] Soporte para RDM6300.
- [x] Soporte para LCD por comunicación I2C.
- [x] Soporte para teclado matricial 4x4.
- [ ] Cambiar configuración para soportar teclado matricial 4x3.
- [ ] Soporte para sensor de distancia HC-SR04.
- [ ] Programar la lógica que debe seguir el dispositivo en el sistema definitivo.
- [ ] Pensar ubicación y distribución de las placas.
- [ ] Ensamblar todo.
- [ ] Instalación en el club alemán.

## Imágenes
Acá dejamos imágenes que podrían ser útiles para utilizar posteriormente al momento del ensamblaje.

**ESP32 Pinout**

![ESP32 Pinout](https://i.pinimg.com/originals/c6/57/83/c657835e84aaf91832a770ea0d7d0767.jpg)

----

**ESP8266 Pinout**

![ESP8266 Pinout](https://www.teachmemicro.com/wp-content/uploads/2018/04/NodeMCUv3.0-pinout.jpg)

----

**RDM6300 Pinout**

![RDM6330 Pinout](https://circuitdigest.com/sites/default/files/inlineimages/u3/RDM6300-RFID-Reader-Module.jpg)

----

**Esquema de conexión ESP32 a LCD via I2C**

![Esquema de conexión ESP32 a LCD via I2C](https://www.electronics-lab.com/wp-content/uploads/2019/05/ESP32-LCD_bb.png)

## Links
Acá dejamos los links que corresponden a las guías definitivas que utilizamos para cada sección del programa para tenerlos a mano en caso de necesitarlos.

* [Conexión del ESP a WiFi](https://www.megunolink.com/articles/wireless/how-do-i-connect-to-a-wireless-network-with-the-esp32/).
* Para la configuración del RDM6300 me basé en [esta guía](https://circuitdigest.com/microcontroller-projects/interfacing-rdm6300-rfid-reader-module-interfacing-with-arduino-nano) y en el código de [este repositorio](https://github.com/arduino12/rdm6300/blob/master/src/rdm6300.cpp).
* [Datasheet del RMD6300](http://j5d2v7d7.stackpathcdn.com/wp-content/uploads/2015/09/RDM630-datasheet.pdf).
* [Tabla de caracteres ASCII](https://asciitable.com).
* [Guía de utilización del LCD 16x2 con comunicación por I2C](https://www.electronics-lab.com/project/using-16x2-i2c-lcd-display-esp32/).
* [Guía de configuración del KeyPad 4x4](https://diyi0t.com/keypad-arduino-esp8266-esp32/).