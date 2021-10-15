#include "parser.h"
#include "encoder.h"

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

	FILE *output_file = fopen(output, "wb");
	for (;;) {
		struct instruction instruction;
		if (parse_instruction(&instruction)) {
			flip_order(instruction.operands);

			uint8_t output[15] = { 0 };
			int len;
			assemble_instruction(output, &len, instruction.mnemonic,
								 instruction.operands);

			if (len <= 0) {
				parse_send_error("no match for instruction");
				ERROR("Shoud not be here");
			}

			if (fwrite(output, len, 1, output_file) != 1)
				ERROR("Can't write");
		} else if (parse_is_eof()) {
			break;
		} else {
			parse_expect_empty_line();
		}
	}

	fclose(output_file);

	parse_close();
}
