#include <WiFi.h>
// Tienen que agregar WiFiConfig.h en la carpeta include/ con las dos variables correspondientes a su red WiFi:
// const char* SSID = "SSID";
// const char* PASSWORD = "PASSWORD";
#include "WiFiConfig.h"
#include "Rdm6300.h"
#include <LiquidCrystal_I2C.h>

// Descomentar para utilizar el sketch de analisis del address del lcd.
// #define LCD_ADDRESS_SCANNER
#ifdef LCDSCANNER_ENABLED
  #include "LCDScanner.h"
#endif

// Defino el pin GPIO-09 como pin de RX para la comunicacion con el RDM6300.
#define RDM6300_RX_PIN 9
Rdm6300 rdm6300;

// Especifico el address obtenido con el LCDScanner, el numero de columnas y el numero de lineas.
LiquidCrystal_I2C lcd(0x27, 16, 2);

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
#endif
}

void loop() {
  // Para conocer el address del lcd.
#ifdef LCDSCANNER_ENABLED
  LCDScanner::scan();
#else
  // Posiciono el cursor del display en la esquina superior izquierda.
  lcd.setCursor(0, 0);

  // Verifcar que la conexion siga estable.
  if (WiFi.status() == WL_CONNECTED) {
    // En caso de no estar conectado tiene que guardar en un buffer las entradas y datos que registre
    // para enviarlos cuando recupere la conexion.
    // Por lo tanto aca va a realizar los request al servidor.
  }

  // Leer los sensores RFID.
  // Si lee algo procesar la informacion y proceder con la logica correspondiente.
  // Por el momento solo imprimo por pantalla el valor.
  if (rdm6300.update()) {
    // Serial.println(rdm6300.getTagId(), HEX);
    lcd.print(rdm6300.getTagId(), HEX);
  }

  // En el futuro capaz no haga falta el delay porque la misma logica hara mas largo el loop.
  delay(10);

  // Al final la ejecucion de cada ciclo del loop, el lcd limpiara su contenido.
#endif
}