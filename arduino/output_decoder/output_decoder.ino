#include "eeprom_common.h"


static const byte kDigits[] = {
  0x7e, 0x30, 0x6d, 0x79,
  0x33, 0x5b, 0x5f, 0x70,
  0x7f, 0x7b, 0x77, 0x1f,
  0x4e, 0x3d, 0x4f, 0x47,
};
const byte kDigitBlank = 0x00;
const byte kDigitMinus = 0x01;



//=========================================================================

void WriteOutputDecoder () {
  Serial.println("Writing Output Decoder");
  
  SetWriteMode(true);

  int address = 0;
  
  Serial.println("...Writing Unsigned Data");
  for (int base = 1; base < 1000; base *= 10) {
    for (int x = 0; x < 256; x++, address++) {
      const byte shifted = x / base;
      const byte digit   = shifted % 10;
      const bool blank   = digit == 0 && shifted == 0 && base != 1;
      const byte data    = blank ? kDigitBlank : kDigits[digit];
      WriteEEPROM(address, data);
    }
  }
  for (int x = 0; x < 256; x++, address++) {
    WriteEEPROM(address, kDigitBlank);
  }
    
  Serial.println("...Writing Signed Data");
  for (int base = 1; base < 1000; base *= 10, address += 256) {
    for (int x = -128; x < 128; x++) {
      const byte shifted = abs(x) / base;
      const byte digit   = shifted % 10;
      const bool blank   = digit == 0 && shifted == 0 && base != 1;
      const byte data    = blank ? kDigitBlank : kDigits[digit];
      const int  addr    = address + byte(x);
      WriteEEPROM(addr, data);
    }
  }
  for (int x = -128; x < 128; x++) {
    const byte data = (x < 0) ? kDigitMinus : kDigitBlank;
    const int  addr = address + byte(x);
    WriteEEPROM(addr, data);
  }
  
  Serial.println("...Done");
}

// ==========================================================================

void setup() {
  SetupForEEPROM();

  WriteOutputDecoder();
  Dump();
}

void loop() {
}
