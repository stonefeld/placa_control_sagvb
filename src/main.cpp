// Librerias a utilizar.
#include <WiFi.h>
#include "RDM6300.h"
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Ultrasonic.h>

// Archivos de tipo Headers con ciertas utilidades.
// Tienen que agregar WiFiConfig.h en la carpeta include/ con las dos variables correspondientes a su red WiFi:
// const char* SSID = "SSID";
// const char* PASSWORD = "PASSWORD";
#include "WiFiConfig.h"

// Descomentar para utilizar el sketch de analisis del address del lcd.
// #define LCD_ADDRESS_SCANNER
#ifdef LCDSCANNER_ENABLED
#include "LCDScanner.h"
#endif

// Cambiar segun la ubicacion del dispositivo.
#define DIRECCION 0 // (ENTRADA = 0, SALIDA = 1)

// Declaro las variables para los requests al servidor.
const int    HTTP_PORT     = 80;
const String HTTP_METHOD   = "GET";
const char   HOST_NAME[22] = "server9julio.ddns.net"; // Esto es solo conceptual, mas adelante va a haber que cambiarlo por la IP de la Raspberry.
const String PATH_NAME     = "/estacionamiento";
String       HTTP_ARGS     = "";
// Declaro el cliente que se va a conectar al servidor.
WiFiClient client;

// Defino el pin GPIO-09 como pin de RX para la comunicacion con el RDM6300.
#define RDM6300_RX_PIN 9
RDM6300 rdm6300;

// Especifico el address obtenido con el LCDScanner, el numero de columnas y el numero de lineas.
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Especifico el tipo de Keypad a utilizar, sus dimensiones, y los valores de cada tecla.
// Aunque el keypad que vamos a utilizar es 4x3, por ahora lo mantengo como 4x4 hasta probarlo.
const byte ROWS = 4;
const byte COLS = 4;
char keyPadMapping[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
// En ambos casos asignamos segun la numeracion por GPIO.
byte rowPins[ROWS] = { 23, 22, 3, 21 };
byte colPins[COLS] = { 19, 18, 5, 17 };
Keypad keyPad = Keypad(makeKeymap(keyPadMapping), rowPins, colPins, ROWS, COLS);

// Defino los pines a utilizar por el sensor de distancia.
#define ULTRASONIC_TRIGGER_PIN    4
#define ULTRASONIC_ECHO_PIN       5
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

  if (client.connect(HOST_NAME, HTTP_PORT)) {
    Serial.println("Connected to server");
  } else {
    Serial.println("Connection failed");
  }

  // Posiciono el cursor del display en la esquina superior izquierda.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("Apoye tarjeta o\ningrese DNI");
#endif
}

void loop() {
  // Para conocer el address del lcd.
#ifdef LCDSCANNER_ENABLED
  LCDScanner::scan();
#else
  static int tipo = 4; // (Nr de Tarjeta = 0, DNI = 1, Proveedor = 3, Ninguno = 4)
  static char codigo[9] = { '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
  static uint32_t dato = 0;

  // Leer los sensores RFID.
  if (rdm6300.update()) {
    // Si lee algo procesar y verificar que sea de un socio.
    tipo = 0;
    dato = rdm6300.getTagId();
    Serial.println(dato, HEX);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("Apoye tarjeta o\ningrese DNI");
  } else {
    // Si no lee nada debe ingresar el DNI.
    char key = keyPad.getKey();
    if (key) {
      // El caracter '#' va a funcionar como ENTER.
      if (key == '#') {
        for (int i = 0; i < 9; i++) {

          // Ejemplo:
          // Codigo = '1', '2', '3', '4', '5', '6', '7', '8', '\0'
          // El array es de 9 espacios para que el ultimo sea siempre NULL, de esta forma
          // podemos saber el largo del codigo ingresado (4 = Proveedor, 8 = DNI)
          if (codigo[i] == '\0') {

            if (i != 3 && i != 7) {
              // Si se presiono ENTER pero el largo del codigo no es 4 ni 8 el codigo es invalido.
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.printf("Error. Codigo\nno valido");

              delay(500);

              // Vuelvo a setear todos los valores al default.
              for (int i = 0; i < 9; i++) {
                codigo[i] = '\0';
              }

              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.printf("Apoye tarjeta o\ningrese DNI");

            } else {
              // Si el codigo tiene el largo correcto, asigno el valor del tipo para el request.
              if (i == 7) tipo = 1;
              if (i == 3) tipo = 2;

              // Transformo el valor leido en long.
              dato = strtol(codigo, NULL, 10);

              // Vuelvo a setear todos los valores al default.
              for (int i = 0; i < 9; i++) {
                codigo[i] = '\0';
              }

              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.printf("Apoye tarjeta o\ningrese DNI");
            }
            break;
          }
        }
      } else {
        // Si no se presiono la tecla ENTER almacenar el valor en el proximo espacio disponible.
        int i = 0;
        for (i = 0; i < 8; i++) {
          if (codigo[i] == '\0') {
            codigo[i] = key;
            break;
          }
        }

        if (i == 0) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Codigo:        ");
        }

        if (i >= 8) {
          // Si en el loop supero el valor 8, significa que ya estaba listo para presionar ENTER.
          lcd.setCursor(0, 0);
          lcd.print("Presione ENTER ");
        } else {
          lcd.setCursor(1, --i);
          lcd.print(key);
        }
      }
    } else {
      // Asigno el valor tipo = 4 para evitar realizar el request con un valor invalido.
      tipo = 4;
    }
  }

  // Verifcar que la conexion siga estable tanto a la red como al servidor y que el tipo sea distinto de 4.
  if (WiFi.status() == WL_CONNECTED && client.connected() && tipo != 4) {
    
    // En caso de no estar conectado tiene que guardar en un buffer las entradas y datos que registre
    // para enviarlos cuando recupere la conexion.
    // Por lo tanto aca va a realizar los request al servidor.
    if (client.connected()) {
      HTTP_ARGS = "?tipo=" + String(tipo) + "&dato=" + String(dato) + "&direccion=" + String(DIRECCION);
      client.println(HTTP_METHOD + " " + PATH_NAME + HTTP_ARGS + " HTTP/1.1");
      client.println("Host: " + String(HOST_NAME));
      client.println("Connection: close");
      client.println();

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Espere...");

      while (client.available()) {
        char c = client.read();
        Serial.print(c);
      }
    } else {
      // Si el dispositivo esta desconectado del servidor tengo que reconectarlo.
      Serial.println("Disconnected from the server");
      client.stop();
      delay(500);

      Serial.println("Trying to reconnect");
      uint8_t i = 0;
      while (!client.connected()) {
        if (client.connect(HOST_NAME, HTTP_PORT)) {
          Serial.println("Connected to server");
        } else {
          Serial.println("Connection failed");
        }

        Serial.print('.');
        delay(500);

        if ((++i % 16) == 0) {
          Serial.print(" still trying to connect ");
        }
      }
    }
  } else {
    // Si el dispositivo esta desconectado de la red WiFi tengo que reconectarlo.
    Serial.println("Disconnected from the Network");
    WiFi.disconnect();

    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to ");
    Serial.print(SSID);
    
    uint8_t i = 0;
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(500);

      if ((++i % 16) == 0) {
        Serial.print(" still trying to connect ");
      }
    }

    Serial.print("\nConnected. My IP address is: ");
    Serial.print(WiFi.localIP());
    Serial.print(". My hostname is: ");
    Serial.print(WiFi.getHostname());
  }

  // Obtengo la distancia que mide el sensor ultrasonico.
  distance = ultrasonic.read(CM);
#endif
}