#include "parser.h"
#include "encoder.h"
#include "elf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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

int get_escape(char nc) {
	switch (nc) {
	case 'n':
		return '\n';
	case '\'':
		return '\'';
	case '\"':
		return '\"';
	case 't':
		return '\t';
	case '0':
		return '\0';
	case '\\':
		return '\\';

	default:
		ERROR("Invalid escape sequence \\%c", nc);
	}
}

void write_escaped_string(const char *str) {
	uint8_t buffer[256] = { 0 };
	if (strlen(str) > 255)
		NOTIMP();

	int i = 0;
	for (; *str; str++, i++) {
		if (*str == '\\') {
			str++;
			buffer[i] = get_escape(*str);
		} else {
			buffer[i] = *str;
		}
	}
	buffer[i] = '\0';

	elf_write(buffer, i);
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
				write_escaped_string(directive.name);
				// TODO: Escape characters
				break;
			case DIR_SECTION:
				elf_set_section(directive.name);
				break;
			case DIR_ZERO:
				assert(directive.immediate.str == NULL);
				elf_write_zero(directive.immediate.value);
				break;
			case DIR_QUAD:
				if (directive.immediate.str)
					elf_symbol_relocate_here(directive.immediate.str, 0, R_X86_64_64);
				elf_write((uint8_t *)&directive.immediate.value, 8);
				break;
			case DIR_BYTE:
				if (directive.immediate.str)
					NOTIMP();
				elf_write((uint8_t *)&directive.immediate.value, 1);
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
				int type = reloc_relative ? R_X86_64_PC32 : R_X86_64_32S;
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
