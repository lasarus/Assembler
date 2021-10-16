#ifndef ENCODER_H
#define ENCODER_H

// The encoder encodes instructions into machine code

#include "parser.h"

#include <stdint.h>

void assemble_instruction(uint8_t *output, int *len, const char *mnemonic, struct operand ops[4], char **reloc_name, int *reloc_offset, int *reloc_relative);

#endif
