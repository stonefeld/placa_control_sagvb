/* ---------- DEFINES ---------- */
// Descomentar para utilzar el analisis de address del lcd.
//#define LCDSCANNER_ENABLED

// GPIO-09 como RX para RDM6300.
#define RDM6300_RX_PIN 9
/* ---------- END DEFINES ---------- */


/* ---------- LIBRERIAS ---------- */
// INSTALLED LIBRARIES
#include <Arduino.h>
#include <ESP32QRCodeReader.h>
#include <LiquidCrystal_I2C.h>

// OWN LIBRARIES
#include "KeypadWrapper.h"
#include "RDM6300.h"
#include "ServerClient.h"

// HEADERS
#include "Utils.h"
// Agregar WiFiConfig.h en carpeta include/ con las variables del WiFi. (Ver README)
#include "WiFiConfig.h"

// LCDSCANNER
#ifdef LCDSCANNER_ENABLED
#include "LCDScanner.h"
#endif
/* ---------- END LIBRERIAS ---------- */


/* ---------- GLOBAL VARIABLES ---------- */
const int DIRECCION = 0; // ENTRADA = 0, SALIDA = 1

// Caracteristicas del keypad.
byte rowPins[] = { 19, 18, 17, 16 };
byte colPins[] = { 32, 33, 25 };
char keypadMapping[sizeof(rowPins) / sizeof(byte)][sizeof(colPins) / sizeof(byte)] = {
	{ '1', '2', '3' },
	{ '4', '5', '6' },
	{ '7', '8', '9' },
	{ '*', '0', '#' }
};

// Declaro las variables para los requests al servidor.
const uint16_t HTTP_PORT   = 80;
const char*    HTTP_METHOD = "GET";
const char*    HOST_NAME   = "server9julio.ddns.net";
const char*    PATH_NAME   = "/estacionamiento/";

// Variables para evitar realizar varios requests en simultaneo.
unsigned long lastTime = 0;
const unsigned long WAIT_DELAY = 5000;

bool teclado = false;
int tipo = 4;  // Nr de Tarjeta = 0, DNI = 1, Proveedor = 3, Ninguno = 4
uint32_t dato = 0; // Dato a enviar al servidor.
/* ---------- END VARIABLES ---------- */

/* ---------- OBJECTS ---------- */
// // QR READER
ESP32QRCodeReader qrReader(CAMERA_MODEL_AI_THINKER);

// // LCD
// // Especifico address obtenido con LCDScanner y numero de columnas y lineas.
LiquidCrystal_I2C lcd(0x27, 16, 2);

// // KEYPAD
KeypadWrapper keypad = KeypadWrapper((char*)keypadMapping, rowPins, colPins, sizeof(rowPins) / sizeof(byte), sizeof(colPins) / sizeof(byte));

// // RFID READER
RDM6300 rfidReader;

// // HTTP CLIENT
ServerClient client = ServerClient(HOST_NAME, PATH_NAME, HTTP_PORT);
/* ---------- END OBJECTS ---------- */


/* ---------- FUNCTION DECLARATION ---------- */
void onQrCodeTask(void *pvParameters);
/* ---------- END FUNCTION DECLARATION ---------- */


/* ---------- FUNCTION DEFINITION ---------- */
void setup()
{
	// Para conocer el address del lcd.
#ifdef LCDSCANNER_ENABLED
	LCDScanner::begin();
#else
	// Inicializo la comunicacion serial.
	Serial.begin(115200);

	// Conecto el dispositivo a la red WiFi.
	String ipAddr = Utils::connectWiFi(SSID, PASSWORD);
	
	// Configuro el lector de RFID (RDM6300).
	rfidReader.begin(RDM6300_RX_PIN);

	// Inicializo el lcd y activo el backlight.
	lcd.init();
	lcd.backlight();

	// Mensaje de estado de conexion a WiFi.
	if (Utils::getWiFiStatus())
	{
		Utils::printLCD(&lcd, SSID, 0, 0, true);
		Utils::printLCD(&lcd, ipAddr.c_str(), 1, 0, false);
	}
	else
	{
		Utils::printLCD(&lcd, "Desconectado", 0, 0, true);
	}
	delay(2000);

	// Mensaje de inicio.
	Utils::printLCD(&lcd, "Apoye tarjeta o", 0, 0, true);
	Utils::printLCD(&lcd, "ingrese DNI", 1, 0, false);
#endif
}

void loop()
{
	// Para conocer el address del lcd.
#ifdef LCDSCANNER_ENABLED
	LCDScanner::scan();
#else
	if (rfidReader.update())
	{
		// Leer los sensores RFID.
		tipo = 0;
		dato = rfidReader.getTagId();
		Serial.println(dato, HEX);
		Utils::printLCD(&lcd, "Apoye tarjeta o", 0, 0, true);
		Utils::printLCD(&lcd, "ingrese DNI", 1, 0, false);
	}
	else
	{
		switch (keypad.getInput())
		{
		case 1:
			// Tecla valida.
			if (keypad.getCodeLength() == 1)
				Utils::printLCD(&lcd, "Codigo:        ", 0, 0, true);
			Utils::printLCD(&lcd, String(keypad.getLastKey()).c_str(), 1, keypad.getCodeLength() - 1, false);
			tipo = 4;
			break;

		case 2:
		{
			// Tecla ENTER y largo correcto.
			teclado = true;
			dato = keypad.getConvertedNumber();
			uint32_t length = keypad.getCodeLength();
			if (length == 7 || length == 8) tipo = 1;
			if (length == 4) tipo = 2;
			keypad.cleanStream();
			Utils::printLCD(&lcd, "Apoye tarjeta o", 0, 0, true);
			Utils::printLCD(&lcd, "ingrese DNI", 1, 0, false);
		} break;

		case 3:
			// Tecla distinta de ENTER y largo maximo.
			Utils::printLCD(&lcd, "Presione ENTER ", 0, 0, false);
			tipo = 4;
			break;

		case 4:
			// Tecla Backspace.
			if (keypad.getCodeLength() == 0)
			{
				Utils::printLCD(&lcd, "Apoye tarjeta o", 0, 0, true);
				Utils::printLCD(&lcd, "ingrese DNI", 1, 0, false);
			}
			else
			{
				Utils::printLCD(&lcd, "Codigo:        ", 0, 0, false);
				Utils::printLCD(&lcd, " ", 1, keypad.getCodeLength(), false);
			}
			tipo = 4;
			break;

		case 0:
			tipo = 4;
			break;
		}
	}

	if ((millis() - lastTime) > WAIT_DELAY && tipo != 4)
	{
		// Verifcar conexion estable.
		if (Utils::getWiFiStatus())
		{
			Utils::printLCD(&lcd, "Espere...", 0, 0, true);
			String response = client.sendRequest(tipo, dato, teclado, DIRECCION, HTTP_METHOD);
			Utils::printLCD(&lcd, response.c_str(), 1, 0, false);
			delay(5000);
			Utils::printLCD(&lcd, "Apoye tarjeta o", 0, 0, true);
			Utils::printLCD(&lcd, "ingrese DNI", 1, 0, false);
			tipo = 4;
			dato = 0;
			teclado = false;
		}
		else
		{
			// Reconectar al WiFi.
			Utils::printLCD(&lcd, "Desconectado", 0, 0, true);
			Utils::connectWiFi(SSID, PASSWORD);
		}
		lastTime = millis();
	}
#endif
}

void onQrCodeTask(void* pvParameters)
{
	struct QRCodeData qrCodeData;

	while (true)
	{
		if (qrReader.receiveQrCode(&qrCodeData, 100))
		{
			if (qrCodeData.valid)
				dato = (uint32_t)qrCodeData.payload;
		}
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}
/* ---------- END FUNCTION DEFINITION ---------- */