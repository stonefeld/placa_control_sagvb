#include "KeypadWrapper.h"

KeypadWrapper::KeypadWrapper(byte rows, byte cols)
{
  m_Rows = rows;
  m_Cols = cols;

  char keypadMapping[m_Rows][m_Cols] = {
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' }
  };
  // En ambos casos asignamos segun la numeracion por GPIO.
  byte rowPins[m_Rows] = { 23, 22, 3, 21 };
  byte colPins[m_Cols] = { 19, 18, 5, 17 };

  m_Keypad = new Keypad(makeKeymap(keypadMapping), rowPins, colPins, m_Rows, m_Cols);
}

// 1 = Presiono tecla, 2 = Presiono Enter y todo OK, 0 = Sin input o error
int KeypadWrapper::getInput()
{
  char key = m_Keypad->getKey();

  if (key) {
    if (key == enterKey) {

        if (isCodeReady()) {
          return 2;
        }

    } else if (key == delKey) {

      // Por ahora no hacemos nada.
      return 0;

    } else {

      if (m_Index <= 8 && m_Codigo[m_Index] == '\0') {
        m_Codigo[m_Index] = key;
        m_LastKey = key;
        m_Index++;
        return 1;
      }

    }
  }
  return 0;
}

bool KeypadWrapper::isCodeReady()
{
  if (m_Index == 8 || m_Index == 7 || m_Index == 4) return true;
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
  
  for (int i = 0; i < 9; i++) {
    m_Codigo[i] = '\0';
  }
}