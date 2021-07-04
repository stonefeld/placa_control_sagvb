#include <WiFi.h>
// Tienen que agregar este archivo con las dos variables correspondientes a su red WiFi:
// const char* SSID = "SSID";
// const char* PASSWORD = "PASSWORD";
#include "WiFiConfig.h"

void setup() {
  // Inicializo la comunicacion serial.
  Serial.begin(115200);

  // Configuro el modulo WiFi.
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting to ");
  Serial.print(SSID);
  
  // Mientras no este conectado imprimo un mensaje de aviso y le dejo tiempo para que establezca la conexion.
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);

    if ((++i % 16) == 0) {
      Serial.print(" still trying to connect ");
    }
  }

  // En este punto se supone que la conexion a la red WiFi esta completa e imprimo la IP y el hostname del mismo.
  Serial.print("\nConnected. My IP address is: ");
  Serial.print(WiFi.localIP());
  Serial.print(". My hostname is: ");
  Serial.print(WiFi.getHostname());
}

void loop() {
  // Verifcar que la conexion siga estable.
  // if (WiFi.status() == WL_CONNECTED) {
  //   // En caso de no estar conectado tiene que guardar en un buffer las entradas y datos que registre
  //   // para enviarlos cuando recupere la conexion.
  //   // Por lo tanto aca va a realizar los request al servidor.
  // }

  // Leer los sensores RFID.
  // Si lee algo procesar la informacion y proceder con la logica correspondiente.
  // if () {
  //   //
  // }
}