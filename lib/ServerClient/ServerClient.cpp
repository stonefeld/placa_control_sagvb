#include "ServerClient.h"

ServerClient::ServerClient(const char* hostname, const char* path, uint16_t port)
{
  m_Hostname = hostname;
  m_Pathname = path;
  m_Port = port;
}

bool ServerClient::connect()
{
  m_Client->stop();

  if (m_Client->connect(m_Hostname, m_Port)) {
    m_Connected = true;
  } else {
    m_Connected = false;
  }
}

void ServerClient::sendRequest(int tipo, uint32_t dato, int direccion, const char* method)
{
  m_Args = "?tipo=" + String(tipo) + "&dato=" + String(dato) + "&direccion=" + String(direccion);
  m_Client->println(String(method) + " " + String(m_Pathname) + m_Args + " HTTP/1.1");
  m_Client->println("Host: " + String(m_Hostname));
  m_Client->println("Connection: close");
  m_Client->println();
}

String ServerClient::readResponse()
{
  String response = "";

  while (m_Client->available()) {
    char c = m_Client->read();
    response += String(c);
  }

  return response;
}

bool ServerClient::getStatus()
{
  return m_Connected;
}