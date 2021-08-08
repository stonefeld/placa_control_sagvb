// Librerias.
#include "ServerClient.h"
#include "RDM6300.h"
#include <LiquidCrystal_I2C.h>
#include "KeypadWrapper.h"

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
const char*    HOST_NAME   = "server9julio.ddns.net";
const char*    PATH_NAME   = "/estacionamiento/";
ServerClient client = ServerClient(HOST_NAME, PATH_NAME, HTTP_PORT);

// Variables para evitar realizar varios requests en simultaneo.
unsigned long lastTime = 0;
unsigned long waitDelay = 5000;

// Defino el pin GPIO-09 como pin de RX para la comunicacion con el RDM6300.
#define RDM6300_RX_PIN 9
RDM6300 rdm6300;

// Especifico el address obtenido con el LCDScanner y el numero de columnas y lineas.
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Especifico las dimensiones del keypad.
const byte ROWS = 4;
const byte COLS = 4;
KeypadWrapper keypad = KeypadWrapper(ROWS, COLS);

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

  // Mensaje de inicio.
  Utils::printLCD(&lcd, "Apoye tarjeta o\nintgrese DNI", 0, 0, true);
#endif
}

void loop() {
  // Para conocer el address del lcd.
#ifdef LCDSCANNER_ENABLED
  LCDScanner::scan();
#else
  static int tipo = 4; // Nr de Tarjeta = 0, DNI = 1, Proveedor = 3, Ninguno = 4
  static uint32_t dato;

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
      // En este caso se presiono una tecla valida.
      if (keypad.getCodeLength() == 0) Utils::printLCD(&lcd, "Codigo:        ", 0, 0, true);
      Utils::printLCD(&lcd, String(keypad.getLastKey()).c_str(), keypad.getCodeLength(), 1, false);
      tipo = 4;
      break;

    case 2:
      // Si se presiono la tecla ENTER y el largo es correcto entonces obtengo el dato y el tipo segun el largo del codigo.
      dato = keypad.getConvertedNumber();
      uint32_t length = keypad.getCodeLength();
      if (length == 7 || length == 8) tipo = 1;
      if (length == 4) tipo = 2;
      keypad.cleanStream();
      Utils::printLCD(&lcd, "Apoye tarjeta o\nintgrese DNI", 0, 0, true);
      break;

    case 3:
      // En este caso se presiono una tecla distinta de ENTER y ya alcanzo el largo maximo.
      Utils::printLCD(&lcd, "Presione ENTER ", 0, 0, false);
      tipo = 4;
      break;

    case 0:
      tipo = 4;
      break;
    }
  }

  // Verifico que haya pasado cierto tiempo desde la ultima vez que se envio el dato.
  if ((millis() - lastTime) > waitDelay) {
    // Verifcar que la conexion siga estable.
    if (Utils::getWiFiStatus()) {
      // En caso de no estar conectado tiene que guardar en un buffer las entradas y datos que registre
      // para enviarlos cuando recupere la conexion.
      String response = client.sendRequest(tipo, dato, DIRECCION, HTTP_METHOD);
      Utils::printLCD(&lcd, "Espere...", 0, 0, true);
      Serial.println(response);
    } else {
      // Si el dispositivo esta desconectado de la red WiFi tengo que reconectarlo.
      Serial.println("Disconnected from the network");
      Utils::connectWiFi(SSID, PASSWORD);
    }

    lastTime = millis();
  }
#endif
}