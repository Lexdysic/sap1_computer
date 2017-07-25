#include "../cpu_common.h"

const int kChipIndex = 0;

//==============================================================

const uint32_t HLT = 0b000000000000000000000001;
const uint32_t MI  = 0b000000000000000000000010;
const uint32_t RI  = 0b000000000000000000000100;
const uint32_t RO  = 0b000000000000000000001000;
const uint32_t II  = 0b000000000000000000010000;
const uint32_t OI  = 0b000000000000000000100000;
const uint32_t AI  = 0b000000000000000001000000;
const uint32_t AO  = 0b000000000000000010000000;

const uint32_t BI  = 0b000000000000000100000000;
const uint32_t BO  = 0b000000000000001000000000;
const uint32_t CE  = 0b000000000000010000000000;
const uint32_t CO  = 0b000000000000100000000000;
const uint32_t CI  = 0b000000000001000000000000;
const uint32_t ALO = 0b000000000010000000000000;
const uint32_t AL0 = 0b000000000100000000000000;
const uint32_t AL1 = 0b000000001000000000000000;

const uint32_t AL2 = 0b000000010000000000000000;
const uint32_t AL3 = 0b000000100000000000000000;
const uint32_t AL4 = 0b000001000000000000000000;
const uint32_t XX0 = 0b000010000000000000000000;
const uint32_t XX1 = 0b000100000000000000000000;
const uint32_t XX2 = 0b001000000000000000000000;
const uint32_t XX3 = 0b010000000000000000000000;
const uint32_t SKP = 0b100000000000000000000000;


//==============================================================

#define GET_ARG_COUNT(...) INTERNAL_GET_ARG_COUNT_PRIVATE(0, ## __VA_ARGS__,  8, 7, 6, 5, 4, 3, 2, 1, 0)
#define INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, count, ...) count

#define PASTE_(left, right) left ## right
#define PASTE(left, right) PASTE_(left, right)

//==============================================================

#define FETCH MI|CO, RO|II|CE

#define INSTRUCTION_0(...) FETCH, ##__VA_ARGS__, SKP, 0, 0, 0, 0, 0,
#define INSTRUCTION_1(...) FETCH, ##__VA_ARGS__, SKP, 0, 0, 0, 0,
#define INSTRUCTION_2(...) FETCH, ##__VA_ARGS__, SKP, 0, 0, 0,
#define INSTRUCTION_3(...) FETCH, ##__VA_ARGS__, SKP, 0, 0,
#define INSTRUCTION_4(...) FETCH, ##__VA_ARGS__, SKP, 0,
#define INSTRUCTION_5(...) FETCH, ##__VA_ARGS__, SKP,
#define INSTRUCTION_6(...) FETCH, ##__VA_ARGS__,

#define Instruction(name, ...) PASTE(INSTRUCTION_, GET_ARG_COUNT(__VA_ARGS__))(__VA_ARGS__)

//==============================================================

const uint32_t kLogic[] = {
  #include "instructions.h"
};
const int32_t kLogicCount = sizeof(kLogic) / sizeof(uint32_t);

const int32_t kTotalMemory          = 2048;
const int32_t kMaxMicroInstructions = 8;
const int32_t kMaxInstructions      = kTotalMemory / kMaxMicroInstructions;

const uint32_t kFetch[] = {
  Instruction() // NOP
};

//==============================================================

bool IsPow2 (int32_t n) {
  return (n & (n - 1)) == 0;
}

void ValidateMicroInstruction (int32_t * instruction) {
  const int32_t kOutputs = RO | AO | BO | CO | ALO;
  const int32_t kInputs  = MI | RI | II | OI | AI | BI | CI;

  // Only a single output is allowed onto the bus at a time.
  assert(IsPow2(instruction[0] & kOutputs));

  // If there is an output to the bus, there must be at least one thing reading it.
  assert(!(instruction[0] & kOutputs) || (instruction[0] & kInputs));
  
  // If Counter-Out is used, the next microcode must modify the counter
  assert(!(instruction[0] & CO) || (instruction[1] & (CE|CI)));

}

void EraseToFetch (int chipIndex) {
  Serial.println("Erasing");

  const uint32_t shift = chipIndex * 8;
  for (int address = 0; address < kTotalMemory; address++) {
    const uint32_t value = kFetch[address % 8];
    WriteEEPROM(address, uint8_t(value >> shift));
  }
  
  Serial.println("...Done");
}

void WriteLogic (int chipIndex) {
  //EraseToFetch(chipIndex);
  
  Serial.print("Writing Logic ");
  Serial.println(chipIndex);

  const uint32_t shift = chipIndex * 8;
  for (int address = 0; address < kLogicCount; ++address) {
    ValidateMicroInstruction(kLogic + address);
    const uint32_t logic = kLogic[address];
    const uint8_t  value = uint8_t(logic >> shift);
    WriteEEPROM(address, value);
  }

  Serial.println("...Logic Done");
  Serial.println("...Erasing Remaining");
  
  for (int address = kLogicCount; address < kTotalMemory; ++address) {
    const uint32_t logic = kFetch[address];
    const uint8_t  value = uint8_t(logic >> shift);
  }
  
  Serial.println("...Done");
}

void DumpLogic () {
  SetWriteMode(false);
  
  for (int base = 0; base < 2048; base += 8) {
    byte data[8];
    for (int i = 0; i < 8; i++) {
      data[i] = ReadEEPROM(base + i);
    }

    char buffer[80];
    sprintf(
      buffer,
      "%03x: "
      " %02x %02x %02x %02x"
      " %02x %02x %02x %02x",
      base / 8,
      data[0],  data[1],  data[2],  data[3],
      data[4],  data[5],  data[6],  data[7]
    );

    Serial.println(buffer);
  }
}

// ==========================================================================

void setup() {
  SetupForEEPROM();

  WriteLogic(kChipIndex);
  DumpLogic();
}

void loop() {
}
