#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

enum reg {
	REG_RAX = 0,
	REG_RCX,
	REG_RDX,
	REG_RBX,
	REG_RSP,
	REG_RBP,
	REG_RSI,
	REG_RDI,
	REG_R8,
	REG_R9,
	REG_R10,
	REG_R11,
	REG_R12,
	REG_R13,
	REG_R14,
	REG_R15,

	REG_NONE,

	REG_MAX
};

struct operand {
	enum {
		O_EMPTY,
		O_REG,
		O_REG_STAR,
		O_IMM,
		O_IMM_ABSOLUTE,
		O_SIB
	} type;

	union {
		struct {
			int size;
			int rex;
			enum reg reg;
		} reg;

		struct  {
			enum reg index, base;
			int scale;
			uint64_t offset;
		} sib;

		struct {
			char *str;
			uint64_t value;
		} imm;
	};
};

struct instruction {
	const char *mnemonic;
	struct operand operands[4];
};

struct label {
	const char *name;
};

struct directive {
	enum {
		DIR_SECTION,
		DIR_GLOBAL,
		DIR_STRING,
		DIR_ZERO,
		DIR_QUAD,
		DIR_BYTE
	} type;

	union {
		const char *name;

		struct {
			uint64_t value;
			char *str;
		} immediate;
	};
};

void parse_init(const char *path);
void parse_close(void);
int parse_instruction(struct instruction *instruction);
int parse_label(struct label *label);
int parse_directive(struct directive *directive);
int parse_is_eof(void);
void parse_expect_empty_line(void);

void parse_send_error(const char *message);

#endif
