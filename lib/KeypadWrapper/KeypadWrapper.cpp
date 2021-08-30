#include "KeypadWrapper.h"

KeypadWrapper::KeypadWrapper(char* keypadMapping, byte* rowPins, byte* colPins, byte rowCount, byte colCount)
	: Keypad(keypadMapping, rowPins, colPins, rowCount, colCount)
{}

// 1 = Presiono tecla, 2 = Presiono ENTER y todo bien, 3 = Presiono tecla pero supero el largo maximo, 0 = No se presiono tecla o hubo un ERROR.
int KeypadWrapper::getInput()
{
	char key = getKey();

	if (key)
	{
		if (key == enterKey)
		{
			if (isCodeReady())
				return 2;
		}
		else if (key == delKey)
		{
			if (m_Index > 0)
			{
				m_Index--;
				m_Codigo[m_Index] = '\0';
				m_LastKey = m_Codigo[m_Index - 1];
				return 4;
			}
		}
		else
		{
			if (m_Index < 8)
			{
				if (m_Codigo[m_Index] == '\0')
				{
					m_Codigo[m_Index] = key;
					m_LastKey = key;
					m_Index++;
					return 1;
				}
			}
			else
			{
				return 3;
			}
		}
	}
	return 0;
}

bool KeypadWrapper::isCodeReady()
{
	if (m_Index == 8 || m_Index == 7 || m_Index == 4)
		return true;
	return false;
}

uint32_t KeypadWrapper::getConvertedNumber()
{
	return strtol(m_Codigo, NULL, 10);
}

byte KeypadWrapper::getCodeLength()
{
	return m_Index;
}

char KeypadWrapper::getLastKey()
{
	return m_LastKey;
}

void KeypadWrapper::cleanStream()
{
	m_Index = 0;
	m_LastKey = '\0';
	
	for (int i = 0; i < 9; i++)
		m_Codigo[i] = '\0';
}