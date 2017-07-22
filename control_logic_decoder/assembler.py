import re
import string
import os
import sys

s_mnemonics = []

kInstructionPattern = R'Instruction\("([^"]+)"'
kInstructionRegex   = re.compile(kInstructionPattern)
kWhitespacePattern  = R'\s+'
kWhitespaceRegex    = re.compile(kWhitespacePattern)
kNumberPattern      = R'(0[xX][\dA-Fa-f]+|0[0-7]*|\d+)'
kNumberRegex        = re.compile(kNumberPattern)
kLabelPattern       = R'\w+:'
kLabelRegex         = re.compile(kLabelPattern)

kArgumentPattern    = R'((?:@?[aAbB])|(?:[@#](?:0[xX][\dA-Fa-f]+|\d+)))'
kMnemonicPattern = R'\s*([a-zA-Z]+)(?:\s+{0}(?:,\s*{0})?)?'.format(kArgumentPattern)
kMnemonicRegex   = re.compile(kMnemonicPattern)


def Clean (line):
    """Fixes case, and removes redundent whitespace"""
    line = line.lower()
    line = kWhitespaceRegex.sub(" ", line)
    line = line.strip()
    return line


def Sanitize (line):
    """Removes all parameter values, removes redundent whitespace, and fixes case"""
    line = Clean(line)
    line = kNumberRegex.sub("", line)
    return line


def LoadMnemonics (filename):
    if len(s_mnemonics):
        return

    with open(filename) as file:
        for line in file:
            match = kInstructionRegex.match(line)
            mnemonicUnsanitized = match.group(1)
            mnemonic = Sanitize(mnemonicUnsanitized)
            s_mnemonics.append(mnemonic)


def ErrorOut (message, filename, line):
    print("{}({}): {}".format(filename, line, message))
    exit(1)


def Assemble (filename):
    LoadMnemonics("instructions.h")
    print(s_mnemonics)

    output = []

    with open(filename, "r") as f:
        for linenum, lineRaw in enumerate(f, 1):
            line = Clean(lineRaw)

            # Blank lines are legal, just skip it
            if len(line) == 0:
                continue

            # Look for a label to look up later
            labelMatch = kLabelRegex.match(line)
            if labelMatch:
                pass
                continue

            mnemonicMatch = kMnemonicRegex.match(line)
            assert(mnemonicMatch)

            mnemonic = Sanitize(line)

            try:
                opcode = s_mnemonics.index(mnemonic)
            except ValueError:
                ErrorOut("unknown instruction '{}'".format(line), filename, linenum)

            mnemonic = s_mnemonics[opcode]

            output.append((line, opcode));

            # Check for a paramter
            valueIndex = line.find("#")
            valueIndex = valueIndex if valueIndex >= 0 else line.find("@")

            if valueIndex >= 0:
                match = kNumberRegex.search(line, valueIndex + 1)
                value = int(match.group(0))
                if value < 0x00 or value > 0xff:
                    ErrorOut("Value out of range", filename, linenum)
                output.append((None,value))
    
    (inRoot, inExt) = os.path.splitext(filename)
    outPath = inRoot + ".jexe"

    maxLen = 0
    for code in output:
        if code[0]:
            maxLen = max(maxLen, len(code[0]))
    maxLen += 1

    with open(outPath, "w") as f:
        for address, code in enumerate(output):
            f.write("{:08b}: {:08b} {}\n".format(address, code[1], code[0] if code[0] else ""))
    print("Output file to '{}'".format(outPath))

if __name__ == "__main__":
    Assemble(sys.argv[1])