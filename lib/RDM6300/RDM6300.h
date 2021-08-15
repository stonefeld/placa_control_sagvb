#ifndef RDM6300_H
#define RDM6300_H

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
#define RDM6300_HARDWARE_SERIAL
#endif

#if !defined(ARDUINO_ARCH_ESP32)
#define RDM6300_SOFTWARE_SERIAL
#endif

#ifdef RDM6300_HARDWARE_SERIAL
#include <HardwareSerial.h>
#endif

#ifdef RDM6300_SOFTWARE_SERIAL
#include <SoftwareSerial.h>
#endif

#include <Stream.h>

#define RDM6300_BAUDRATE        9600
#define RDM6300_PACKET_SIZE     14
#define RDM6300_PACKET_BEGIN    0x02
#define RDM6300_PACKET_END      0x03
#define RDM6300_NEXT_READ_MS    220
#define RDM6300_READ_TIMEOUT    20

class RDM6300
{
public:
	void begin(Stream* stream);
	void begin(int rx_pin, uint8_t uart_nr=1);
	void end(void);
	bool update(void);
	uint32_t getTagId(void);
	bool isTagNear(void);

private:
#ifdef RDM6300_HARDWARE_SERIAL
	HardwareSerial* m_HardwareSerial = NULL;
#endif
#ifdef RDM6300_SOFTWARE_SERIAL
	SoftwareSerial *m_SoftwareSerial = NULL;
#endif
	Stream* m_Stream = NULL;
	uint32_t m_TagId = 0;
	uint32_t m_LastTagId = 0;
	uint32_t m_LastReadMs = 0;
};

#endif