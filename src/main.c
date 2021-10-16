#include "parser.h"
#include "encoder.h"
#include "elf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR(STR, ...) do { printf("Error on line %d file %s: \"" STR "\"\n", __LINE__, __FILE__, ##__VA_ARGS__); exit(1); } while(0)
#define NOTIMP() ERROR("Not implemented");

// Flip order of operands, and ignore empty on the end.
void flip_order(struct operand ops[4]) {
	struct operand out[4];
	struct operand tmp = ops[0];
	int out_i = 0;
	for (int i = 3; i >= 0; i--) {
		if (ops[i].type == O_EMPTY)
			continue;
		out[out_i++] = ops[i];
	}

	for (; out_i < 4; out_i++)
		out[out_i].type = O_EMPTY;

	memcpy(ops, out, sizeof out);
}

int main(int argc, char **argv) {
	const char *input = NULL, *output = NULL;
	if (argc != 3)
		ERROR("Invalid number of arguments");
	input = argv[1];
	output = argv[2];

	parse_init(input);

	elf_init();
	for (;;) {
		struct instruction instruction;
		struct directive directive;
		struct label label;
		if (parse_label(&label)) {
			elf_symbol_set_here(label.name, 0);
		} else if (parse_directive(&directive)) {
			switch (directive.type) {
			case DIR_GLOBAL:
				elf_symbol_set_global(directive.name);
				break;
			case DIR_STRING:
				// TODO: Escape characters
				elf_write((uint8_t *)directive.name, strlen(directive.name) + 1);
				break;
			case DIR_SECTION:
				elf_set_section(directive.name);
				break;
			case DIR_ZERO:
				elf_write_zero(directive.immediate);
				break;
			case DIR_QUAD:
				elf_write((uint8_t *)&directive.immediate, 8);
				break;
			default:
				NOTIMP();
			}
		} else if (parse_instruction(&instruction)) {
			flip_order(instruction.operands);

			uint8_t output[15] = { 0 };
			int len;
			char *reloc_name = NULL;
			int reloc_offset = 0;
			int reloc_relative = 0;
			assemble_instruction(output, &len, instruction.mnemonic,
								 instruction.operands,
								 &reloc_name, &reloc_offset, &reloc_relative);

			if (len <= 0) {
				parse_send_error("no match for instruction");
				ERROR("Couldn't encode instruction.");
			}

			if (reloc_name) {
				elf_symbol_relocate_here(reloc_name, reloc_offset, reloc_relative);
			}

			elf_write(output, len);
		} else if (parse_is_eof()) {
			break;
		} else {
			parse_expect_empty_line();
		}
	}

	parse_close();

	elf_finish(output);
}
