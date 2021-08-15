#ifndef SERVERCLIENT_H
#define SERVERCLIENT_H

#include <WiFi.h>

class ServerClient
{
public:
  ServerClient(const char* hostname, const char* path, uint16_t port);

  String sendRequest(int tipo, uint32_t dato, bool teclado, int direccion, const char* method);

private:
  const char* m_Hostname;
  uint16_t m_Port;
  const char* m_Pathname;
};

#endif