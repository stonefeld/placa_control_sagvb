#include "ServerClient.h"

#include <HTTPClient.h>

ServerClient::ServerClient(const char* hostname, const char* path, uint16_t port)
{
	m_Hostname = hostname;
	m_Pathname = path;
	m_Port = port;
}

String ServerClient::sendRequest(int tipo, uint32_t dato, bool teclado, int direccion, const char* method)
{
	String response = "";

	HTTPClient http;
	String fullPath = "http://" + String(m_Hostname) + ":" + String(m_Port) + String(m_Pathname) + "?tipo=" + String(tipo) + "&dato=" + String(dato) + "&direccion=" + String(direccion) + "&teclado=" + String(teclado);
	http.begin(fullPath.c_str());

	if (String(method).equals("GET")) {
		int responseCode = http.GET();
		if (responseCode > 0) {
			Serial.print("HTTP Response code: ");
			Serial.println(responseCode);
			response = http.getString();
		} else {
			Serial.print("Error code: ");
			Serial.println(responseCode);
		}
	} else {
		Serial.print(method);
		Serial.println(" is not a valid method");
	}

	return response;
}