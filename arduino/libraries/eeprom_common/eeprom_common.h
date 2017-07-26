const byte kShiftData   = 2;
const byte kShiftClock  = 3;
const byte kShiftLatch  = 4;

const byte kEepromD0    = 5;
const byte kEepromD7    = 12;
const byte kWriteEnable = 13;


//=========================================================================

void SetAddress (int address, bool outputEnable) {
  int first = (address >> 8) | (outputEnable ? 0x00 : 0x80);
  int second = address;
  shiftOut(kShiftData, kShiftClock, MSBFIRST, first);
  shiftOut(kShiftData, kShiftClock, MSBFIRST, second);
  digitalWrite(kShiftLatch, LOW);
  digitalWrite(kShiftLatch, HIGH);
  digitalWrite(kShiftLatch, LOW);
}

void SetWriteMode (bool shouldWrite) {
  for (int pin = kEepromD0; pin <= kEepromD7; pin++) {
    pinMode(pin, shouldWrite ? OUTPUT : INPUT);
  }
}

byte ReadEEPROM (int address) {
  SetWriteMode(false);
  SetAddress(address, true);
  
  int data = 0;
  for (int pin = kEepromD7; pin >= kEepromD0; pin--) {
    data = (data << 1) | digitalRead(pin);
  }
  return data;
}

void WriteEEPROM (int address, byte data) {
  SetWriteMode(true);
  SetAddress(address, false);
  
  for (int pin = kEepromD0; pin <= kEepromD7; pin++, data >>= 1) {
    digitalWrite(pin, data & 0x01);
  }

  digitalWrite(kWriteEnable, LOW);
  delayMicroseconds(1);
  digitalWrite(kWriteEnable, HIGH);  
  delay(4);
}

void Erase (byte value = 0xff) {
  Serial.println("Erasing");
  
  for (int address = 0; address < 2048; address++) {
    WriteEEPROM(address, value);
  }
  
  Serial.println("...Done");
}

void Dump () {
  SetWriteMode(false);
  
  for (int base = 0; base < 2048; base += 16) {
    byte data[16];
    for (int i = 0; i < 16; i++) {
      data[i] = ReadEEPROM(base + i);
    }

    char buffer[80];
    sprintf(
      buffer,
      "%03x: "
      " %02x %02x %02x %02x"
      " %02x %02x %02x %02x"
      " %02x %02x %02x %02x"
      " %02x %02x %02x %02x",
      base,
      data[0],  data[1],  data[2],  data[3],
      data[4],  data[5],  data[6],  data[7],
      data[8],  data[9],  data[10], data[11],
      data[12], data[13], data[14], data[15]
    );

    Serial.println(buffer);
  }
}

// ==========================================================================

void SetupForEEPROM () {
  pinMode(kShiftData,  OUTPUT);
  pinMode(kShiftClock, OUTPUT);
  pinMode(kShiftLatch, OUTPUT);

  digitalWrite(kWriteEnable, HIGH);  
  pinMode(kWriteEnable, OUTPUT);
  
  Serial.begin(57600);
}

// ==========================================================================

#define __ASSERT_USE_STDERR
#include <assert.h>
/*
__assert (const char *func, const char *file, int line, const char *failedexpr) {
  if (func == NULL) {
    (void)fprintf(
      stderr,
      "Assertion failed: (%s), file %s, line %d.\n",
      failedexpr,
      file,
      line
    );
  }
  else {
    (void)fprintf(
      stderr,
      "Assertion failed: (%s), function %s, file %s, line %d.\n",
      failedexpr,
      func,
      file,
      line
    );
  }
  abort();
}*/