#ifndef KEYPADWRAPPER_H
#define KEYPADWRAPPER_H

#include "Keypad.h"

class KeypadWrapper : Keypad
{
public:
	KeypadWrapper(char* map, byte* pRows, byte* pCols, byte rows, byte cols);

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
	byte m_Index = 0;
	char m_Codigo[9] = { '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };
	char m_LastKey;
};

#endif