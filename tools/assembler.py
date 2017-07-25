import re
import string
import os
import sys


#==============================================================================

kInstructionPath = R"../instructions.h"

kInstructionPattern = R'Instruction\("([^"]+)"'
kInstructionRegex   = re.compile(kInstructionPattern)

kWhitespacePattern  = R'\s+'
kWhitespaceRegex    = re.compile(kWhitespacePattern)

kNumberPattern      = R'(0[xX][\dA-Fa-f]+|0[0-7]*|\d+)'
kNumberRegex        = re.compile(kNumberPattern)

kLabelValuePattern  = R'\w+'
kLabelValueRegex    = re.compile(kLabelValuePattern)

kLabelDeclPattern   = R'(\w+):'
kLabelDeclRegex     = re.compile(kLabelDeclPattern)

kArgumentPattern    = R'((?:@?\w+)|(?:[@#](?:0[xX][\dA-Fa-f]+|\d+)))'
kMnemonicPattern    = R'\s*([a-zA-Z]+)(?:\s+{0}(?:,\s*{0})?)?'.format(kArgumentPattern)
kMnemonicRegex      = re.compile(kMnemonicPattern)

#==============================================================================

s_mnemonics = []

#==============================================================================

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
    line = re.sub("(?<=@)\w+", "@", line)
    return line


def LoadMnemonics (filename):
    if len(s_mnemonics):
        return

    with open(filename) as file:
        for line in file:
            if len(line) == 0:
                continue
            match = kInstructionRegex.match(line)
            if not match:
                ErrorOut("unknown instruction '{}'".format(line))
            mnemonicUnsanitized = match.group(1)
            mnemonic = Sanitize(mnemonicUnsanitized)
            s_mnemonics.append(mnemonic)


def ErrorOut (message, filename=None, line=None):
    if filename and line:
        print("{}({}): error {}".format(filename, line, message))
    else:
        print("error: {}".format(message))
    exit(1)


def Assemble (filename):
    LoadMnemonics(kInstructionPath)

    for mnemonic in s_mnemonics:
        #print(mnemonic)
        pass

    output = []
    labels = {}
    label_fixups = {}

    with open(filename, "r") as f:
        for linenum, lineRaw in enumerate(f, 1):
            line = Clean(lineRaw)

            # Blank lines are legal, just skip it
            if len(line) == 0:
                continue

            # Look for a label to look up later
            labelDeclMatch = kLabelDeclRegex.match(line)
            if labelDeclMatch:
                identifier = labelDeclMatch.group(1)
                if identifier in labels:
                    ErrorOut("Label '{}' already defined".format("identifier"), filename, linenum)
                labels[identifier] = len(output)
                continue

            #otherwise, look for the mnemonic to be processed
            mnemonicMatch = kMnemonicRegex.match(line)
            assert(mnemonicMatch)

            mnemonic = Sanitize(line)

            try:
                opcode = s_mnemonics.index(mnemonic)
            except ValueError:
                ErrorOut("unknown instruction '{}'".format(line), filename, linenum)

            mnemonic = s_mnemonics[opcode]

            output.append((line, opcode))

            # Check for a parameter
            valueIndex = line.find("#")
            valueIndex = valueIndex if valueIndex >= 0 else line.find("@")

            if valueIndex >= 0:
                numberValueMatch = kNumberRegex.search(line, valueIndex + 1)
                labelValueMatch  = kLabelValueRegex.search(line, valueIndex + 1)
                if numberValueMatch:
                    value = int(numberValueMatch.group(0))
                    if value < 0x00 or value > 0xff:
                        ErrorOut("Value out of range", filename, linenum)
                    output.append((None,value))
                elif labelValueMatch:
                    value = labelValueMatch.group(0)
                    output.append((None,None)) # placeholder
                    label_fixups[len(output) - 1] = value

    # fixup jumps
    for source_address, label in label_fixups.iteritems():
        if label not in labels:
            ErrorOut("Label '{}' not found".format(label), filename, linenum)
        absolute_destination_address = labels[label]
        existing = output[source_address]
        output[source_address] = (existing[0], absolute_destination_address)

    # output to destination file
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