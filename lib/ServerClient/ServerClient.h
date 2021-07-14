#ifndef SERVERCLIENT_H
#define SERVERCLIENT_H

#include <WiFi.h>

class ServerClient
{
public:
  ServerClient(const char* hostname, const char* path, const uint16_t port);

  void connect(void);
  void sendRequest(int tipo, uint32_t dato, int direccion, const char* method);
  String readResponse(void);
  bool getStatus(void);

private:
  WiFiClient* m_Client;
  const char* m_Hostname;
  uint16_t m_Port;
  const char* m_Pathname;
  String m_Args;
  bool m_Connected;
};

#endif