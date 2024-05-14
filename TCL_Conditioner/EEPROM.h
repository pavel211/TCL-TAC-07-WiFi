// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// A set of functions for easy reading and writing to the EEPROM for the
// ESP8266 microcontroller in the Arduino environment is likely to work 
// on other platforms.
// Page of project: https://github.com/pavel211/EEPROM_Comfort/
// License: GNU GPL-3.0
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <EEPROM.h>

boolean isFloat(String tString) {
  String tBuf;
  boolean decPt = false;
  
  if(tString.charAt(0) == '+' || tString.charAt(0) == '-') tBuf = &tString[1];
  else tBuf = tString;  

  for(int x=0;x<tBuf.length();x++)
  {
    if(tBuf.charAt(x) == '.') {
      if(decPt) return false;
      else decPt = true;  
    }    
    else if(tBuf.charAt(x) < '0' || tBuf.charAt(x) > '9') return false;
  }
  return true;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

float EepromReadFloat (int StartCell, float MinValue, float MaxValue, float StandartValue)
{
   float OutValue;
   EEPROM.begin(512);
   EEPROM.get(StartCell, OutValue);
   EEPROM.commit();
   if ((OutValue >= MinValue) and (OutValue <= MaxValue)) { return OutValue; } else { return StandartValue; } 
}

int EepromReadInt (int StartCell, int MinValue, int MaxValue, int StandartValue)
{
   int OutValue;
   EEPROM.begin(512);
   EEPROM.get(StartCell, OutValue);
   EEPROM.commit();
   if ((OutValue >= MinValue) and (OutValue <= MaxValue)) { return OutValue; } else { return StandartValue; } 
}

long EepromReadLong (int StartCell, long MinValue, long MaxValue, long StandartValue)
{
   long OutValue;
   EEPROM.begin(512);
   EEPROM.get(StartCell, OutValue);
   EEPROM.commit();
   if ((OutValue >= MinValue) and (OutValue <= MaxValue)) { return OutValue; } else { return StandartValue; } 
}

String EepromReadString (int StartCell, String StandartValue, int MaxLength)
{
   String OutValue;
   bool NormalString = false;
   char read_char = 0; 
   EEPROM.begin(512);
   for (int i = 0; i < MaxLength; ++i) { read_char = char(EEPROM.read(StartCell + i)); if (read_char == '\0') {  NormalString = true; break; } else {OutValue += read_char; }  }
   EEPROM.commit();
   
   if ((OutValue.length() < 1) or (!NormalString)) { return StandartValue; } else { return OutValue; }
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++

bool EepromSaveFloat (int StartCell, String SaveValue)
{
  if (SaveValue.length() > 0)
   {
     EEPROM.begin(512);
     if (isFloat(SaveValue)) EEPROM.put(StartCell, SaveValue.toFloat());
     EEPROM.commit();

     float TestValue;
     EEPROM.begin(512);
     EEPROM.get(StartCell, TestValue);
     EEPROM.commit();
   
     if (TestValue == SaveValue.toFloat()) { return true;  }
     else                                  { return false; }
   } 
  else                                     { return false; }
}

bool EepromSaveInt (int StartCell, String SaveValue)
{
  if (SaveValue.length() > 0)
   {
     int SaveV = SaveValue.toInt();
     EEPROM.begin(512);
     EEPROM.put(StartCell, SaveV);
     EEPROM.commit();

     int TestValue;
     EEPROM.begin(512);
     EEPROM.get(StartCell, TestValue);
     EEPROM.commit();
   
     if (TestValue == SaveV)  { return true;  }
     else                     { return false; }
   } 
  else                        { return false; }     
}

bool EepromSaveLong (int StartCell, String SaveValue)
{
  if (SaveValue.length() > 0)
   {
     long SaveV = SaveValue.toInt();
     EEPROM.begin(512);
     EEPROM.put(StartCell, SaveV);
     EEPROM.commit();

     long TestValue;
     EEPROM.begin(512);
     EEPROM.get(StartCell, TestValue);
     EEPROM.commit();
   
     if (TestValue == SaveV)  { return true;  }
     else                     { return false; }
   } 
  else                        { return false; }     
}

bool EepromSaveString (int StartCell, String SaveValue, int MaxLength)
{
  if (SaveValue.length() > 0)
   {
     EEPROM.begin(512);
     for (int i = 0; i < SaveValue.length(); ++i) { EEPROM.write(StartCell + i, SaveValue[i]); if (i > MaxLength) break; }
     EEPROM.write(StartCell + SaveValue.length(), '\0');
     EEPROM.commit();

     String TestValue;
     char read_char = 0; 
     EEPROM.begin(512);
     for (int i = 0; i < 32; ++i) { read_char = char(EEPROM.read(StartCell + i)); TestValue += read_char; if (read_char == '\0') break;  }
     EEPROM.commit();

     if (TestValue == SaveValue) { return true;  }
     else                        { return false; }
   } 
  else                           { return false; }
}
