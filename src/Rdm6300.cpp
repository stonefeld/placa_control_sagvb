#include "Rdm6300.h"
#include <Arduino.h>

void Rdm6300::begin(Stream* stream)
{
    m_Stream = stream;
    if (!m_Stream)
        return;

    // Seteo el timeout de lectura a 20ms.
    m_Stream->setTimeout(RDM6300_READ_TIMEOUT);
}

void Rdm6300::begin(int rx_pin, uint8_t uart_nr)
{
    // Inicializo la comunicacion serial para el RDM6300. Por default usara el uart1 y utilizara unicamente
    // pin de RX y seteo el timeout a 20ms.
    end();
#if defined(ARDUINO_ARCH_ESP32)
    m_Stream = m_HardwareSerial = new HardwareSerial(uart_nr);
    m_HardwareSerial->begin(RDM6300_BAUDRATE, SERIAL_8N1, rx_pin, -1);
#elif defined(ARDUINO_ARCH_ESP8266)
    if (rx_pin == 13) {
        m_Stream = m_HardwareSerial = &Serial;
        m_HardwareSerial->begin(RDM6300_BAUDRATE, SERIAL_8N1, SERIAL_RX_ONLY);
        if (uart_nr)
            m_HardwareSerial->swap();
    }
#endif
#ifdef RDM6300_SOFTWARE_SERIAL
    if (!m_Stream) {
        m_Stream = m_SoftwareSerial = new SoftwareSerial(rx_pin, -1);
        m_SoftwareSerial->begin(RDM6300_BAUDRATE);
    }
#endif
    begin(m_Stream);
}

void Rdm6300::end()
{
    // Cierro la comunicacion serial.
    m_Stream = NULL;
#ifdef RDM6300_HARDWARE_SERIAL
    if (m_HardwareSerial)
        m_HardwareSerial->end();
#endif
#ifdef RDM6300_SOFTWARE_SERIAL
    if (m_SoftwareSerial)
        m_SoftwareSerial->end();
#endif
}

bool Rdm6300::update()
{
    char buff[RDM6300_PACKET_SIZE];
    uint32_t tag_id;
    uint32_t checksum;

    // Si el stream esta vacio retorna.
    if (!m_Stream)
        return false;

    // Si no hay un stream disponible para la lectura retorna.
    if (!m_Stream->available())
        return false;

    // Si la primera posicion del stream recibido no corresponde con el valor asignado de inicio (02) retorna.
    if (m_Stream->peek() != RDM6300_PACKET_BEGIN && m_Stream->read())
        return false;

    // Si el stream recibido tiene un tamaño diferente al esperado retorna. En este if, guarda el
    // valor del stream leido en 'buff' para utilizar en caso de tener el tamaño correcto.
    if (RDM6300_PACKET_SIZE != m_Stream->readBytes(buff, RDM6300_PACKET_SIZE))
        return false;

    // Si la ultima posicion del stream recibido no corresponde con el valor asignado de finalizacion (03) retorna.
    if (buff[13] != RDM6300_PACKET_END)
        return false;

    // La funcion 'readBytes' llamada anteriormente guarda el stream recibido en el buffer pasado como parametro,
    // pero la ultima posicion del buffer no es NULL ('\0') y como ya verifique que el ultimo valor de posicion
    // sea el esperado puedo asignar manualmente el valor NULL para la funcion 'strtol'.
    buff[13] = '\0';
    // Le paso como parametro a strtol la posicion de memoria donde se encuentra el checksum y lo convierte en long.
    checksum = strtol(buff + 11, NULL, 16);

    // Nuevamente asigno a la seccion de finalizacion de los datos (que son 10 caracteres ASCII) un NULL para la funcion strtol.
    buff[11] = '\0';
    // Le paso a strtol la posicion donde comienzan los datos del tag y lo devuelve como un long.
    tag_id = strtol(buff + 3, NULL, 16);

    buff[3] = '\0';
    checksum ^= strtol(buff + 1, NULL, 16);

    // Hago un XOR de cada posicion del tag_id para verificar que el checksum sea correcto.
    //
    // Ejemplo:
    //               +----+--------------------------+----------+----+
    //   Estructura: | 02 | 10 ASCII Data Characters | Checksum | 03 |
    //               +----+--------------------------+----------+----+
    //   Tag ID: 62E3086CED
    //   Output data: 0x36, 0x32, 0x45, 0x33, 0x30, 0x38, 0x36, 0x43, 0x45, 0x44
    //   Checksum: (0x62) XOR (0xE3) XOR (0x08) XOR (0x6C) XOR (0xED) = 0x08
    //
    // El valor recibido por el stream y el calculado con la operacion por XOR deben coincidir para que sean validos.
    for (uint8_t i = 0; i < 32; i += 8)
        checksum ^= ((tag_id >> i) & 0xFF);

    // Si el checksum al finalizar el chequeo vale algun valor distinto de 0 no se verifico.
    if (checksum)
        return false;

    // Si el ultimo tag leido es distinto del actual le asigno el nuevo valor y reinicio el tiempo.
    if (m_LastTagId != tag_id) {
        m_LastTagId = tag_id;
        m_LastReadMs = 0;
    }

    // Si el tag viejo todavia esta asignado asigna el valor del tag actual a 0.
    if (isTagNear())
        tag_id = 0;

    m_LastReadMs = millis();
    m_TagId = tag_id;

    // Si todo lo anterior se cumplio pero por ejemplo el tag viejo es el mismo despues de un cierto tiempo
    // el tag_id sera cero y la funcion devolvera false. Si es un tag nuevo devolvera un valor distinto de 0,
    // es decir, devolvera true.
    return tag_id;
}

bool Rdm6300::isTagNear()
{
    return millis() - m_LastReadMs < RDM6300_NEXT_READ_MS;
}

uint32_t Rdm6300::getTagId()
{
    // Simplemente devuelve el valor que tiene guardado del tag leido y reinicia su valor para poder obtener uno nuevo.
    uint32_t tag_id = m_TagId;
    m_TagId = 0;
    return tag_id;
}