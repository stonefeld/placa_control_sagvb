#ifndef KEYPADWRAPPER_H
#define KEYPADWRAPPER_H

#include <Keypad.h>

class KeypadWrapper
{
public:
	KeypadWrapper(byte rows, byte cols);

	int getInput(void);
	bool isCodeReady(void);
	uint32_t getConvertedNumber(void);
	byte getCodeLength(void);
	char getLastKey(void);
	void cleanStream(void);

public:
	char enterKey = '#';
	char delKey = '*';

private:
	Keypad* m_Keypad;
	byte m_Rows;
	byte m_Cols;

	byte m_Index = 0;
	char m_Codigo[9] = { '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
	char m_LastKey;
};

#endif