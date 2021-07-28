#include <WiFi.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

namespace Utils {

  void connectWiFi(const char* ssid, const char* password) {
    // Me aseguro que el dispositivo no este conectado.
    WiFi.disconnect();

    // Configuro el modulo WiFi.
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to ");
    Serial.println(ssid);

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
    Serial.println(WiFi.getHostname());
  }

  bool getWiFiStatus() {
    if (WiFi.status() == WL_CONNECTED) return true;
    return false;
  }

  void printLCD(LiquidCrystal_I2C* lcd, const char* message, int row, int col, bool clear) {
    if (clear) lcd->clear();
    lcd->setCursor(0, 0);
    lcd->printf(message);
  }
  
}