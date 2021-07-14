// Librerias.
#include <WiFi.h>
#include "ServerClient.h"
#include "RDM6300.h"
#include <LiquidCrystal_I2C.h>
#include "KeypadWrapper.h"
#include <Ultrasonic.h>

// Header Files.
// Tienen que agregar WiFiConfig.h en la carpeta include/ con las dos variables correspondientes a su red WiFi:
// const char* SSID = "SSID";
// const char* PASSWORD = "PASSWORD";
#include "WiFiConfig.h"
#include "Utils.h"

// Descomentar para utilizar el sketch de analisis del address del lcd.
// #define LCD_ADDRESS_SCANNER
#ifdef LCDSCANNER_ENABLED
#include "LCDScanner.h"
#endif

// (ENTRADA = 0, SALIDA = 1)
#define DIRECCION 0

// Declaro las variables para los requests al servidor.
const uint16_t HTTP_PORT   = 80;
const char*    HTTP_METHOD = "GET";
const char*    HOST_NAME   = "server9julio.ddns.net"; // Esto es solo conceptual, mas adelante va a haber que cambiarlo por la IP de la Raspberry.
const char*    PATH_NAME   = "/estacionamiento";
ServerClient client = ServerClient(HOST_NAME, PATH_NAME, HTTP_PORT);

// Defino el pin GPIO-09 como pin de RX para la comunicacion con el RDM6300.
#define RDM6300_RX_PIN 9
RDM6300 rdm6300;

// Especifico el address obtenido con el LCDScanner y el numero de columnas y lineas.
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Especifico las dimensiones del keypad.
// Aunque el keypad que vamos a utilizar es 4x3, por ahora lo mantengo como 4x4 hasta probarlo.
const byte ROWS = 4;
const byte COLS = 4;
KeypadWrapper keypad = KeypadWrapper(ROWS, COLS);

// Defino los pines a utilizar por el sensor de distancia.
#define ULTRASONIC_TRIGGER_PIN 4
#define ULTRASONIC_ECHO_PIN    5
unsigned int distance = 0;
// Inicializo el sensor de distancia.
Ultrasonic ultrasonic(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN);

void setup() {
  // Para conocer el address del lcd.
#ifdef LCDSCANNER_ENABLED
  LCDScanner::begin();
#else
  // Inicializo la comunicacion serial.
  Serial.begin(115200);
  
  // Configuro el lector de RFID (RDM6300).
  rdm6300.begin(RDM6300_RX_PIN);

  // Inicializo el lcd y activo el backlight.
  lcd.init();
  lcd.backlight();

  // Conecto el dispositivo a la red WiFi.
  Utils::connectWiFi(SSID, PASSWORD);

  // Conecto al dispositivo al servidor.
  client.connect();

  // Mensaje de inicio.
  Utils::printLCD(&lcd, "Apoye tarjeta o\nintgrese DNI", 0, 0, true);
#endif
}

void loop() {
  // Para conocer el address del lcd.
#ifdef LCDSCANNER_ENABLED
  LCDScanner::scan();
#else
  static int tipo = 4; // (Nr de Tarjeta = 0, DNI = 1, Proveedor = 3, Ninguno = 4)
  static uint32_t dato = 0;

  if (rdm6300.update()) {
    // Leer los sensores RFID.
    tipo = 0;
    dato = rdm6300.getTagId();
    Serial.println(dato, HEX);
    Utils::printLCD(&lcd, "Apoye tarjeta o\nintgrese DNI", 0, 0, true);

  } else {
    // Si no leyo ningun tag de rfid verifico si alguna tecla fue presionada.
    switch (keypad.getInput())
    {
    case 1:
      // En este caso se presiono una tecla valida y el largo no es el correcto.
      if (keypad.getCodeLength() == 0) {
        Utils::printLCD(&lcd, "Codigo:        ", 0, 0, true);
        Utils::printLCD(&lcd, String(keypad.getLastKey()).c_str(), keypad.getCodeLength(), 1, false);

      } else if (keypad.getLastKey() == keypad.enterKey) {
        // Si la tecla que se presiono era ENTER no escribo el caracter por pantalla y le informo que presione ENTER.
        Utils::printLCD(&lcd, "Presione ENTER  ", 0, 0, false);

      }
      break;

    case 2:
      // Si se presiono la tecla ENTER y el largo es correcto entonces obtengo el dato y el tipo segun el largo del codigo.
      dato = keypad.getConvertedNumber();
      if (keypad.getCodeLength() == 7 || keypad.getCodeLength() == 8) tipo = 1;
      if (keypad.getCodeLength() == 4) tipo = 2;
      keypad.cleanStream();
      Utils::printLCD(&lcd, "Apoye tarjeta o\nintgrese DNI", 0, 0, true);
      break;

    case 0:
      tipo = 4;
    }
  }

  // Verifcar que la conexion siga estable.
  if (WiFi.status() == WL_CONNECTED) {
    
    // En caso de no estar conectado tiene que guardar en un buffer las entradas y datos que registre
    // para enviarlos cuando recupere la conexion.
    // Por lo tanto aca va a realizar los request al servidor.
    if (client.getStatus() && tipo != 4) {

      client.sendRequest(tipo, dato, DIRECCION, HTTP_METHOD);
      Utils::printLCD(&lcd, "Espere...", 0, 0, true);
      Serial.println(client.readResponse());

    } else {
      // Si el dispositivo esta desconectado del servidor tengo que reconectarlo.
      Serial.println("Disconnected from the server");
      client.connect();
    }
  } else {
    // Si el dispositivo esta desconectado de la red WiFi tengo que reconectarlo.
    Serial.println("Disconnected from the Network");
    Utils::connectWiFi(SSID, PASSWORD);
  }
#endif
}