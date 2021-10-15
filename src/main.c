#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define ERROR(STR, ...) do { printf("Error on line %d file %s: \"" STR "\"\n", __LINE__, __FILE__, ##__VA_ARGS__); exit(1); } while(0)
#define NOTIMP() ERROR("Not implemented");

struct operand_encoding {
	enum {
		OE_EMPTY,
		OE_NONE,
		OE_MODRM_REG,
		OE_MODRM_RM,
		OE_IMM8,
		OE_IMM16,
		OE_IMM32,
		OE_IMM64,
		OE_OPEXT
	} type;

	int duplicate;
};

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

void str_to_reg(const char *str, enum reg *reg, int *size, int *requires_rex) {
	if (requires_rex)
		*requires_rex = 0;
	if (strcmp(str, "rax") == 0) { *reg = REG_RAX; *size = 8; return; }
	if (strcmp(str, "rcx") == 0) { *reg = REG_RCX; *size = 8; return; }
	if (strcmp(str, "rdx") == 0) { *reg = REG_RDX; *size = 8; return; }
	if (strcmp(str, "rbx") == 0) { *reg = REG_RBX; *size = 8; return; }
	if (strcmp(str, "rsp") == 0) { *reg = REG_RSP; *size = 8; return; }
	if (strcmp(str, "rbp") == 0) { *reg = REG_RBP; *size = 8; return; }
	if (strcmp(str, "rsi") == 0) { *reg = REG_RSI; *size = 8; return; }
	if (strcmp(str, "rdi") == 0) { *reg = REG_RDI; *size = 8; return; }
	if (strcmp(str, "r8") == 0) { *reg = REG_R8; *size = 8; return; }
	if (strcmp(str, "r9") == 0) { *reg = REG_R9; *size = 8; return; }
	if (strcmp(str, "r10") == 0) { *reg = REG_R10; *size = 8; return; }
	if (strcmp(str, "r11") == 0) { *reg = REG_R11; *size = 8; return; }
	if (strcmp(str, "r12") == 0) { *reg = REG_R12; *size = 8; return; }
	if (strcmp(str, "r13") == 0) { *reg = REG_R13; *size = 8; return; }
	if (strcmp(str, "r14") == 0) { *reg = REG_R14; *size = 8; return; }
	if (strcmp(str, "r15") == 0) { *reg = REG_R15; *size = 8; return; }

	if (!requires_rex)
		ERROR("Invalid register %s", str);

	if (strcmp(str, "eax") == 0) { *reg = REG_RAX; *size = 4; return; }
	if (strcmp(str, "ecx") == 0) { *reg = REG_RCX; *size = 4; return; }
	if (strcmp(str, "edx") == 0) { *reg = REG_RDX; *size = 4; return; }
	if (strcmp(str, "ebx") == 0) { *reg = REG_RBX; *size = 4; return; }
	if (strcmp(str, "esp") == 0) { *reg = REG_RSP; *size = 4; return; }
	if (strcmp(str, "ebp") == 0) { *reg = REG_RBP; *size = 4; return; }
	if (strcmp(str, "esi") == 0) { *reg = REG_RSI; *size = 4; return; }
	if (strcmp(str, "edi") == 0) { *reg = REG_RDI; *size = 4; return; }
	if (strcmp(str, "r8d") == 0) { *reg = REG_R8; *size = 4; return; }
	if (strcmp(str, "r9d") == 0) { *reg = REG_R9; *size = 4; return; }
	if (strcmp(str, "r10d") == 0) { *reg = REG_R10; *size = 4; return; }
	if (strcmp(str, "r11d") == 0) { *reg = REG_R11; *size = 4; return; }
	if (strcmp(str, "r12d") == 0) { *reg = REG_R12; *size = 4; return; }
	if (strcmp(str, "r13d") == 0) { *reg = REG_R13; *size = 4; return; }
	if (strcmp(str, "r14d") == 0) { *reg = REG_R14; *size = 4; return; }
	if (strcmp(str, "r15d") == 0) { *reg = REG_R15; *size = 4; return; }

	if (strcmp(str, "ax") == 0) { *reg = REG_RAX; *size = 2; return; }
	if (strcmp(str, "cx") == 0) { *reg = REG_RCX; *size = 2; return; }
	if (strcmp(str, "dx") == 0) { *reg = REG_RDX; *size = 2; return; }
	if (strcmp(str, "bx") == 0) { *reg = REG_RBX; *size = 2; return; }
	if (strcmp(str, "sp") == 0) { *reg = REG_RSP; *size = 2; return; }
	if (strcmp(str, "bp") == 0) { *reg = REG_RBP; *size = 2; return; }
	if (strcmp(str, "si") == 0) { *reg = REG_RSI; *size = 2; return; }
	if (strcmp(str, "di") == 0) { *reg = REG_RDI; *size = 2; return; }
	if (strcmp(str, "r8w") == 0) { *reg = REG_R8; *size = 2; return; }
	if (strcmp(str, "r9w") == 0) { *reg = REG_R9; *size = 2; return; }
	if (strcmp(str, "r10w") == 0) { *reg = REG_R10; *size = 2; return; }
	if (strcmp(str, "r11w") == 0) { *reg = REG_R11; *size = 2; return; }
	if (strcmp(str, "r12w") == 0) { *reg = REG_R12; *size = 2; return; }
	if (strcmp(str, "r13w") == 0) { *reg = REG_R13; *size = 2; return; }
	if (strcmp(str, "r14w") == 0) { *reg = REG_R14; *size = 2; return; }
	if (strcmp(str, "r15w") == 0) { *reg = REG_R15; *size = 2; return; }

	if (strcmp(str, "al") == 0) { *reg = REG_RAX; *size = 1; return; }
	if (strcmp(str, "cl") == 0) { *reg = REG_RCX; *size = 1; return; }
	if (strcmp(str, "dl") == 0) { *reg = REG_RDX; *size = 1; return; }
	if (strcmp(str, "bl") == 0) { *reg = REG_RBX; *size = 1; return; }
	/* if (strcmp(str, "ah") == 0) { *reg = REG_RSP; *size = 1; return; } */
	/* if (strcmp(str, "ch") == 0) { *reg = REG_RBP; *size = 1; return; } */
	/* if (strcmp(str, "dh") == 0) { *reg = REG_RSI; *size = 1; return; } */
	/* if (strcmp(str, "bh") == 0) { *reg = REG_RDI; *size = 1; return; } */
	if (strcmp(str, "spl") == 0) { *reg = REG_RSP; *size = 1; *requires_rex = 1; return; }
	if (strcmp(str, "bpl") == 0) { *reg = REG_RBP; *size = 1; *requires_rex = 1; return; }
	if (strcmp(str, "sil") == 0) { *reg = REG_RSI; *size = 1; *requires_rex = 1; return; }
	if (strcmp(str, "dil") == 0) { *reg = REG_RDI; *size = 1; *requires_rex = 1; return; }

	if (strcmp(str, "r8l") == 0) { *reg = REG_R8; *size = 1; return; }
	if (strcmp(str, "r9l") == 0) { *reg = REG_R9; *size = 1; return; }
	if (strcmp(str, "r10l") == 0) { *reg = REG_R10; *size = 1; return; }
	if (strcmp(str, "r11l") == 0) { *reg = REG_R11; *size = 1; return; }
	if (strcmp(str, "r12l") == 0) { *reg = REG_R12; *size = 1; return; }
	if (strcmp(str, "r13l") == 0) { *reg = REG_R13; *size = 1; return; }
	if (strcmp(str, "r14l") == 0) { *reg = REG_R14; *size = 1; return; }
	if (strcmp(str, "r15l") == 0) { *reg = REG_R15; *size = 1; return; }
	ERROR("Invalid reg string %s", str);
}

struct operand_accepts {
	enum {
		ACC_EMPTY,
		ACC_RAX,
		ACC_RCX,
		ACC_REG,
		ACC_REG_STAR,
		ACC_IMM8_S,
		ACC_IMM16_S,
		ACC_IMM32_S,
		ACC_IMM8_U,
		ACC_IMM16_U,
		ACC_IMM32_U,
		ACC_IMM64,
		ACC_MODRM
	} type;

	union {
		struct {
			int size;
		} reg;
	};
};

#define A_IMM32_S {.type = ACC_IMM32_S }
#define A_IMM8_S {.type = ACC_IMM8_S }
#define A_IMM8 {.type = ACC_IMM8 }
#define A_IMM32 {.type = ACC_IMM32_U }
#define A_IMM64 {.type = ACC_IMM64 }
#define A_REG(SIZE) {.type = ACC_REG, .reg.size = SIZE }
#define A_MODRM(SIZE) {.type = ACC_MODRM, .reg.size = SIZE }
#define A_REG_STAR(SIZE) {.type = ACC_REG_STAR, .reg.size = SIZE }
#define A_RAX(SIZE) {.type = ACC_RAX, .reg.size = SIZE }
#define A_RCX(SIZE) {.type = ACC_RCX, .reg.size = SIZE }

struct encoding {
	const char *mnemonic;
	uint8_t opcode;
	uint8_t op2, op3;
	int rex, rexw;
	int modrm_extension;
	int slash_r;
	int op_size_prefix;
	struct operand_encoding operand_encoding[4];
	struct operand_accepts operand_accepts[4];
};

struct operand {
	enum {
		O_EMPTY,
		O_REG,
		O_REG_STAR,
		O_IMM,
		O_SIB
	} type;

	union {
		struct {
			int size;
			int requires_rex;
			enum reg reg;
		} reg;

		struct  {
			enum reg index, base;
			int scale;
			uint64_t offset;
		} sib;

		uint64_t imm;
	};
};

int get_disp_size(uint64_t disp_u) {
	long disp = disp_u;
	if (disp == 0)
		return 0;
	if (disp >= INT8_MIN && disp <= INT8_MAX)
		return 1;
	if (disp >= INT32_MIN && disp <= INT32_MAX)
		return 4;
	ERROR("Invalid displacement size");
}

#define MR {{OE_MODRM_RM}, {OE_MODRM_REG}}

struct encoding encodings[] = {
	// ADDQ
	{"addq", 0x05, .rex = 1, .rexw = 1, .operand_encoding = {{OE_NONE}, {OE_IMM32}}, .operand_accepts = {A_RAX(8), A_IMM32_S}},
	{"addq", 0x04, .operand_encoding = {{OE_NONE}, {OE_IMM8}}, .operand_accepts = {A_REG(4), A_IMM8_S}},
	{"addq", 0x83, .rex = 1, .rexw = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_IMM8}}, .operand_accepts = {A_REG(8), A_IMM8_S}},
	{"addq", 0x01, .rex = 1, .rexw = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_REG(8), A_REG(8)}},

	{"subq", 0x83, .rex = 1, .rexw = 1, .modrm_extension = 5, .operand_encoding = {{OE_MODRM_RM}, {OE_IMM8}}, .operand_accepts = {A_MODRM(8), A_IMM8_S}},
	{"subq", 0x81, .rex = 1, .rexw = 1, .modrm_extension = 5, .operand_encoding = {{OE_MODRM_RM}, {OE_IMM32}}, .operand_accepts = {A_MODRM(8), A_IMM32_S}},
	{"subq", 0x29, .rex = 1, .rexw = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_MODRM(8), A_REG(8)}},
	{"subl", 0x29, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_MODRM(4), A_REG(4)}},

	{"andl", 0x21, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_REG(4), A_REG(4)}},
	{"andq", 0x21, .rex = 1, .rexw = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_REG(8), A_REG(8)}},
	{"andq", 0x83, .rex = 1, .rexw = 1, .modrm_extension = 4, .operand_encoding = {{OE_MODRM_RM}, {OE_IMM8}}, .operand_accepts = {A_REG(8), A_IMM8_S}},

	{"orl", 0x09, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_REG(4), A_REG(4)}},
	{"orq", 0x09, .rexw = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_REG(8), A_REG(8)}},

	{"xor", 0x31, .rex = 1, .rexw = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_REG(8), A_REG(8)}},
	{"xorq", 0x31, .rex = 1, .rexw = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_REG(8), A_REG(8)}},
	{"xorl", 0x31, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_REG(4), A_REG(4)}},

	{"divl", 0xf7, .modrm_extension = 6, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(4)}},
	{"divq", 0xf7, .rexw = 1, .modrm_extension = 6, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(8)}},

	{"idivl", 0xf7, .modrm_extension = 7, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(4)}},
	{"idivq", 0xf7, .rexw = 1, .modrm_extension = 7, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(8)}},

	{"imulq", 0x69, .rex = 1, .rexw = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM, 1}, {OE_MODRM_REG}, {OE_IMM32}}, .operand_accepts = {A_REG(8), A_IMM32_S}},
	{"imulq", 0x6b, .rex = 1, .rexw = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM, 1}, {OE_MODRM_REG}, {OE_IMM8}}, .operand_accepts = {A_REG(8), A_IMM8_S}},
	{"imulq", 0x0f, .op2 = 0xaf, .rex = 1, .rexw = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(8), A_MODRM(8)}},

	{"imull", 0x0f, .op2 = 0xaf, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(4), A_MODRM(4)}},

	{"callq", 0xff, .modrm_extension = 2, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_REG_STAR(8)}},
	{ "cltd", .opcode = 0x99 },
	{ "cqto", .rexw = 1, .opcode = 0x99 },
	{ "leave", .opcode = 0xc9 },
	{ "ret", .opcode = 0xc3 },
	{ "ud2", .opcode = 0x0f, .op2 = 0x0b },

	{"cmpl", 0x39, .slash_r = 1, .operand_encoding = MR, .operand_accepts = {A_REG(4), A_REG(4)}},
	{"cmpq", 0x39, .rex = 1, .rexw = 1, .slash_r = 1, .operand_encoding = MR, .operand_accepts = {A_MODRM(8), A_REG(8)}},

	{"cmpl", 0x83, .modrm_extension = 7, .operand_encoding = {{OE_MODRM_RM}, {OE_IMM8}}, .operand_accepts = {A_REG(4), A_IMM8_S}},

	{"movl", 0xb8, .modrm_extension = 0, .operand_encoding = {{OE_OPEXT}, {OE_IMM32}}, .operand_accepts = {A_REG(4), A_IMM32}},
	{"movl", 0xc7, .modrm_extension = 0, .operand_encoding = {{OE_MODRM_RM}, {OE_IMM32}}, .operand_accepts = {A_MODRM(4), A_IMM32}},
	{"movl", 0xc7, .modrm_extension = 0, .operand_encoding = {{OE_MODRM_RM}, {OE_IMM32}}, .operand_accepts = {A_MODRM(4), A_IMM32_S}},
	{"movl", 0x89, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_MODRM(4), A_REG(4)}},
	{"movl", 0x89, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_MODRM(4), A_REG(4)}},
	{"movw", 0x89, .op_size_prefix = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_MODRM(2), A_REG(2)}},
	{"movw", 0x8b, .op_size_prefix = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(2), A_MODRM(2)}},
	{"movq", 0x89, .rex = 1, .rexw = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_MODRM(8), A_REG(8)}},
	{"movq", 0x8b, .rex = 1, .rexw = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(8), A_MODRM(8)}},
	{"movb", 0x8a, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(1), A_MODRM(1)}},
	{"movb", 0x88, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_MODRM(1), A_REG(1)}},
	{"movb", 0xc6, .operand_encoding = {{OE_MODRM_RM}, {OE_IMM8}}, .operand_accepts = {A_MODRM(1), A_IMM8_S}},
	{"movl", 0x8b, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(4), A_MODRM(4)}},
	{"movq", 0xc7, .rex = 1, .rexw = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_IMM32}}, .operand_accepts = {A_MODRM(8), A_IMM32_S}},

	{"movabsq", 0xb8, .rex = 1, .rexw = 1, .operand_encoding = {{OE_OPEXT}, {OE_IMM64}}, .operand_accepts = {A_REG(8), A_IMM64}},

	{"leaq", 0x8d, .rex = 1, .rexw = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(8), A_MODRM(8)}},
	{"leal", 0x8d, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(4), A_MODRM(4)}},

	{"movswl", 0x0f, .op2 = 0xbf, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(4), A_MODRM(2)}},
	{"movswq", 0x0f, .rexw = 1, .op2 = 0xbf, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(8), A_MODRM(2)}},
	{"movslq", 0x63, .rex = 1, .rexw = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(8), A_MODRM(4)}},
	{"movsbl", 0x0f, .op2 = 0xbe, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(4), A_MODRM(1)}},
	{"movsbw", 0x0f, .op_size_prefix = 1, .op2 = 0xbe, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(2), A_MODRM(1)}},
	{"movsbq", 0x0f, .rexw = 1, .op2 = 0xbe, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(8), A_MODRM(1)}},

	{"movzwl", 0x0f, .op2 = 0xb7, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(4), A_MODRM(2)}},
	{"movzbl", 0x0f, .op2 = 0xb6, .slash_r = 1, .operand_encoding = {{OE_MODRM_REG}, {OE_MODRM_RM}}, .operand_accepts = {A_REG(4), A_MODRM(1)}},

	{"pushq", 0x50, .operand_encoding = {{OE_OPEXT}}, .operand_accepts = {A_REG(8)}},

	{"notl", 0xf7, .modrm_extension = 2, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(4)}},
	{"notq", 0xf7, .rex = 1, .rexw = 1, .modrm_extension = 2, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(8)}},

	{"negl", 0xf7, .modrm_extension = 3, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(4)}},
	{"negq", 0xf7, .rexw = 1, .modrm_extension = 3, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(8)}},

	{"seta", 0x0f, .op2 = 0x97, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(1)}},
	{"setb", 0x0f, .op2 = 0x92, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(1)}},
	{"setbe", 0x0f, .op2 = 0x96, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(1)}},
	{"sete", 0x0f, .op2 = 0x94, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(1)}},
	{"setg", 0x0f, .op2 = 0x9f, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(1)}},
	{"setge", 0x0f, .op2 = 0x9d, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(1)}},
	{"setgl", 0x0f, .op2 = 0x9c, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(1)}},
	{"setl", 0x0f, .op2 = 0x9c, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(1)}},
	{"setle", 0x0f, .op2 = 0x9e, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(1)}},
	{"setnb", 0x0f, .op2 = 0x93, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(1)}},
	{"setne", 0x0f, .op2 = 0x95, .operand_encoding = {{OE_MODRM_RM}}, .operand_accepts = {A_MODRM(1)}},
	
	{"salq", 0xd3, .rex = 1, .rexw = 1, .modrm_extension = 4, .operand_encoding = {{OE_MODRM_RM}, {OE_NONE}}, .operand_accepts = {A_MODRM(8), A_RCX(1)}},
	{"sall", 0xd3, .modrm_extension = 4, .operand_encoding = {{OE_MODRM_RM}, {OE_NONE}}, .operand_accepts = {A_MODRM(4), A_RCX(1)}},

	{"sarl", 0xd3, .modrm_extension = 7, .operand_encoding = {{OE_MODRM_RM}, {OE_NONE}}, .operand_accepts = {A_MODRM(4), A_RCX(1)}},
	{"sarq", 0xd3, .rexw = 1, .modrm_extension = 7, .operand_encoding = {{OE_MODRM_RM}, {OE_NONE}}, .operand_accepts = {A_MODRM(8), A_RCX(1)}},

	{"shrl", 0xd3, .modrm_extension = 5, .operand_encoding = {{OE_MODRM_RM}, {OE_NONE}}, .operand_accepts = {A_MODRM(4), A_RCX(1)}},
	{"shrq", 0xd3, .rexw = 1, .modrm_extension = 5, .operand_encoding = {{OE_MODRM_RM}, {OE_NONE}}, .operand_accepts = {A_MODRM(8), A_RCX(1)}},

	{"testb", 0x84, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_MODRM(1), A_REG(1)}},
	{"testl", 0x85, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_MODRM(4), A_REG(4)}},
	{"testq", 0x85, .rexw = 1, .slash_r = 1, .operand_encoding = {{OE_MODRM_RM}, {OE_MODRM_REG}}, .operand_accepts = {A_MODRM(8), A_REG(8)}},
};

// has_sib[%base][%index] = (%scale
/* int has_sib_only_base[REG_MAX] = { */
/* 	[REG_RSP] = 1, [REG_RBP] = 1, [REG_R12] = 1, [REG_R13] = 1 */
/* }; */

void encode_sib(struct operand *o, int *rex_b, int *rex_x, int *modrm_mod, int *modrm_rm,
				uint64_t *disp, int *has_disp8, int *has_disp32,
				int *has_sib, int *sib_scale, int *sib_index, int *sib_base) {
	int disp_size = get_disp_size(o->sib.offset);

	// (%reg)
	if (o->sib.index == REG_NONE && o->sib.base != REG_NONE &&
		disp_size == 0) {
		if (o->sib.base == REG_RSP ||
			o->sib.base == REG_R12) {
			*modrm_rm = 4;
			*modrm_mod = 0;

			*has_sib = 1;
			*sib_index = 4;
			*sib_base = 4;
			if (o->sib.base == REG_R12)
				*rex_b = 1;
		} else if (o->sib.base == REG_RBP ||
				   o->sib.base == REG_R13) {
			*sib_index = 4;
			*modrm_mod = 1;

			*has_disp8 = 1;
			*disp = 0;
			*sib_base = 5;
			if (o->sib.base == REG_R13)
				*rex_b = 1;
		} else {
			*modrm_mod = 0;

			*modrm_rm = o->sib.base & 7;
			*rex_b = (o->sib.base & 0x8) >> 3;
		}
	// disp8(%reg)
	} else if (o->sib.index == REG_NONE && o->sib.base != REG_NONE &&
			   disp_size <= 1) {
		if (o->sib.base == REG_RSP ||
			o->sib.base == REG_R12) {
			*modrm_rm = 4;
			*modrm_mod = 1;

			*has_sib = 1;
			*sib_index = 4;
			*sib_base = 4;
			if (o->sib.base == REG_R12)
				*rex_b = 1;
		} else {
			*modrm_mod = 1;
			*has_disp8 = 1;

			*modrm_rm = o->sib.base & 7;
			*rex_b = (o->sib.base & 0x8) >> 3;
		}

		*has_disp8 = 1;
		*disp = o->sib.offset;
	// disp32(%reg)
	} else if (o->sib.index == REG_NONE && o->sib.base != REG_NONE &&
			   disp_size <= 4) {
		if (o->sib.base == REG_RSP ||
			o->sib.base == REG_R12) {
			*modrm_mod = 2;

			*has_sib = 1;
			*sib_index = 4;

			*modrm_rm = o->sib.base & 7;
			*rex_b = (o->sib.base & 0x8) >> 3;
		} else {
			*modrm_mod = 2;

			*disp = o->sib.offset;

			*modrm_rm = o->sib.base & 7;
			*rex_b = (o->sib.base & 0x8) >> 3;
		}

		*has_disp32 = 1;
		*disp = o->sib.offset;
	// (%base, %scale, index)
	} else if (o->sib.index != REG_NONE && o->sib.base != REG_NONE &&
			   disp_size == 0) {
		*has_sib = 1;
		if (o->sib.index == REG_RSP) {
			NOTIMP();
		} else if (o->sib.base == REG_RBP ||
				   o->sib.base == REG_R13) {
			NOTIMP();
		} else {
			*modrm_rm = 4;
				
			*sib_base = o->sib.base & 7;
			*rex_b = (o->sib.base & 0x8) >> 3;

			*sib_index = o->sib.index & 7;
			*rex_x = (o->sib.index & 0x8) >> 3;
		}
	} else {
		NOTIMP();
	}

	// disp8(%base, %scale, index)
	// disp32(%base, %scale, index)
}

//struct encoding cmp2 = {0x83, .operand_encoding = {{OE_MODRM_RM}, {OE_IMM8}}};

void assemble_encoding(uint8_t *output, int *len, struct encoding *encoding, struct operand ops[4]) {
	int has_imm8 = 0, has_imm16 = 0, has_imm32 = 0, has_imm64 = 0;
	uint64_t imm = 0;
	//uint8_t imm8 = 0;

	int has_rex = encoding->rexw || encoding->rex;
	int rex_b = 0;
	int rex_r = 0;
	int rex_x = 0;

	int has_modrm = 0;
	int modrm_reg = 0;
	int modrm_mod = 0;
	int modrm_rm = 0;

	int has_disp8 = 0, has_disp32 = 0;
	uint64_t disp = 0;

	int has_sib = 0;
	int sib_index = 0;
	int sib_scale = 0;
	int sib_base = 0;

	uint8_t op_ext = 0;

	for (int i = 0, j = 0; i < 4 && j < 4; i++, j++) {
		struct operand *o = ops + i;
		struct operand_encoding *oe = encoding->operand_encoding + j;

		if ((o->type == O_EMPTY) != (oe->type == OE_EMPTY))
			ERROR("Invalid number of arguments to instruction");

		if (o->type == O_REG && o->reg.requires_rex)
			has_rex = 1;

		switch (oe->type) {
		case OE_IMM8:
			has_imm8 = 1;
			imm = o->imm;
			break;

		case OE_IMM16:
			has_imm16 = 1;
			imm = o->imm;
			break;

		case OE_IMM32:
			has_imm32 = 1;
			imm = o->imm;
			break;

		case OE_IMM64:
			has_imm64 = 1;
			imm = o->imm;
			break;

		case OE_MODRM_RM:
			has_modrm = 1;
			switch (o->type) {
			case O_REG:
			case O_REG_STAR:
				modrm_mod = 3;

				modrm_rm = o->reg.reg;
				if (has_rex)
					rex_b = (modrm_rm & 0x8) >> 3;
				//modrm_rm = 
				//NOTIMP();
				break;

			case O_SIB: {
				encode_sib(o, &rex_b, &rex_x, &modrm_mod, &modrm_rm,
						   &disp, &has_disp8, &has_disp32,
						   &has_sib, &sib_scale, &sib_index, &sib_base);
			} break;

			default:
				ERROR("Not imp: %d\n", o->type);
				NOTIMP();
			}
			break;

		case OE_MODRM_REG:
			switch (o->type) {
			case O_REG:
				assert(encoding->slash_r);
				modrm_reg = o->reg.reg;
				rex_r = (modrm_reg & 0x8) >> 3;
				break;
			default:
				ERROR("Not imp: %d\n", o->type);
			}
			break;

		case OE_OPEXT:
			op_ext = o->reg.reg;
			break;

		case OE_NONE:
			break;

		case OE_EMPTY:
			break;

		default:
			ERROR("Not imp: %d, %d", oe->type, i);
		}

		if (oe->duplicate)
			i--;
	}

	if (rex_b || rex_r) {
		has_rex = 1;
	}

	int idx = 0;

	if (encoding->op_size_prefix)
		output[idx++] = 0x66;

	if (has_rex) {
		uint8_t rex_byte = 0x40;

		if (encoding->rexw)
			rex_byte |= 0x8;

		rex_byte |= rex_b;
		rex_byte |= rex_r << 2;

		output[idx++] = rex_byte;
	}

	output[idx++] = encoding->opcode | op_ext;
	if (encoding->opcode == 0x0f) {
		output[idx++] = encoding->op2;
		if (encoding->op2 == 0x38 ||
			encoding->op2 == 0x3a)
			output[idx++] = encoding->op3;
	}

	if (has_modrm) {
		uint8_t modrm_byte = 0;
		modrm_byte |= (encoding->modrm_extension & 0x7) << 3;
		modrm_byte |= (modrm_reg & 0x7) << 3;
		modrm_byte |= (modrm_mod & 0x3) << 6;
		modrm_byte |= (modrm_rm & 0x7);

		output[idx++] = modrm_byte;
	}

	if (has_sib) {
		uint8_t sib_byte = 0;

		sib_byte |= (sib_scale & 3) << 6;
		sib_byte |= (sib_index & 7) << 3;
		sib_byte |= (sib_base & 7);

		output[idx++] = sib_byte;
	}

	if (has_disp8) {
		output[idx] = (uint8_t)disp;
		idx++;
	} else if (has_disp32) {
		*(uint32_t *)(output + idx) = disp;
		idx += 4;
	}

	if (has_imm8) {
		output[idx] = (uint8_t)imm;
		idx++;
	} else if (has_imm16) {
		*(uint16_t *)(output + idx) = imm;
		idx += 2;
	} else if (has_imm32) {
		*(uint32_t *)(output + idx) = imm;
		idx += 4;
	} else if (has_imm64) {
		*(uint64_t *)(output + idx) = imm;
		idx += 8;
	}

	*len = idx;
}

int does_match(struct operand *o, struct operand_accepts *oa) {
	switch (oa->type) {
	case ACC_RAX:
		if (o->type != O_REG)
			return 0;
		if (o->reg.size != oa->reg.size)
			return 0;
		if (o->reg.reg != REG_RAX)
			return 0;
		break;

	case ACC_RCX:
		if (o->type != O_REG)
			return 0;
		if (o->reg.size != oa->reg.size)
			return 0;
		if (o->reg.reg != REG_RCX)
			return 0;
		break;

	case ACC_REG:
		if (o->type != O_REG)
			return 0;
		if (o->reg.size != oa->reg.size)
			return 0;
		break;

	case ACC_REG_STAR:
		if (o->type != O_REG_STAR)
			return 0;
		if (o->reg.size != oa->reg.size)
			return 0;
		break;

	case ACC_EMPTY:
		if (o->type != O_EMPTY)
			return 1;
		break;

	case ACC_IMM8_S: {
		long s = o->imm;
		if (o->type != O_IMM)
			return 0;
		if (s < INT8_MIN || s > INT8_MAX)
			return 0;
	} break;

	case ACC_IMM16_S: {
		long s = o->imm;
		if (o->type != O_IMM)
			return 0;
		if (s < INT16_MIN || s > INT16_MAX)
			return 0;
	} break;

	case ACC_IMM32_S: {
		long s = o->imm;
		if (o->type != O_IMM)
			return 0;
		if (s < INT32_MIN || s > INT32_MAX)
			return 0;
	} break;

	case ACC_IMM32_U: {
		if (o->type != O_IMM)
			return 0;
		if (o->imm > UINT32_MAX)
			return 0;
	} break;

	case ACC_MODRM:
		// Either accept register of correct size or SIB.
		if (o->type == O_REG) {
			if (o->reg.size != oa->reg.size)
				return 0;
		} else if (o->type == O_SIB) {
		} else {
			return 0;
		}
		break;

	case ACC_IMM64:
		if (o->type != O_IMM)
			return 0;
		break;

	default:
		ERROR("Not imp: %d\n", oa->type);
	}

	return 1;
}

void assemble_instruction(uint8_t *output, int *len, const char *mnemonic, struct operand ops[4]) {
	int best_len = 16;
	uint8_t best_output[15] = { 0 };
	// TODO: Just order the instructions in such a way that we can just take the first one that appears.
	for (unsigned i = 0; i < sizeof encodings / sizeof *encodings; i++) {
		struct encoding *encoding = encodings + i;

		if (strcmp(encoding->mnemonic, mnemonic) != 0)
			continue;

		(void)encoding;
		int matches = 1;
		for (int j = 0; j < 4; j++) {
			struct operand *o = ops + j;
			struct operand_accepts *oa = encoding->operand_accepts + j;
			matches = does_match(o, oa);

			if (!matches)
				break;
		}

		if (!matches)
			continue;

		//printf("Matched %d\n", i);
		uint8_t current_output[15];
		int current_len;
		assemble_encoding(current_output, &current_len, encoding, ops);

		if (current_len < best_len) {
			memcpy(best_output, current_output, sizeof (best_output));
			best_len = current_len;
		}
	}

	if (best_len == 16) {
		*len = -1;
		return;
		//ERROR("No match for instruction");
		//NOTIMP();
	}

	*len = best_len;
	memcpy(output, best_output, sizeof (best_output));
}

void parse_operand(const char *str, struct operand *op) {
	if (*str == '$') {
		uint64_t imm = 0;
		if (str[1] == '-') {
			imm = strtol(str + 1, NULL, 10);
		} else {
			imm = strtoul(str + 1, NULL, 10);
		}
		op->type = O_IMM;
		op->imm = imm;
	} else if (*str == '%') {
		op->type = O_REG;
		str_to_reg(str + 1, &op->reg.reg, &op->reg.size, &op->reg.requires_rex);
	} else if (str[0] == '*' && str[1] == '%') {
		op->type = O_REG_STAR;
		str_to_reg(str + 2, &op->reg.reg, &op->reg.size, &op->reg.requires_rex);
	} else if (str[0] == '-' || str[0] == '+' || isdigit(str[0]) || str[0] == '(') {
		op->type = O_SIB;
		op->sib.index = REG_NONE;
		op->sib.scale = 1;
		int idx = 0;
		if (str[0] != '(') {
			char offset_str[100];
			for (; *str; str++) {
				if (!isdigit(str[0]) && str[0] != '+' && str[0] != '-')
					break;
				offset_str[idx++] = *str;
			}
			offset_str[idx] = '\0';

			uint64_t offset = strtol(offset_str, NULL, 10);
			op->sib.offset = offset;
		} else {
			op->sib.offset = 0;
		}
		if (*str != '(')
			NOTIMP();
		str++;

		char reg[10];
		idx = 0;
		for (; *str; str++) {
			if (*str == ',' || *str == ')')
				break;
			reg[idx++] = *str;
		}
		reg[idx] = '\0';
		int size;
		str_to_reg(reg + 1, &op->sib.base, &size, NULL);

		if (*str == ',') {
			idx = 0;
			str++;
			for (; *str; str++) {
				if (*str == ',' || *str == ')')
					break;
				reg[idx++] = *str;
			}
			reg[idx] = '\0';

			str_to_reg(reg + 1, &op->sib.index, &size, NULL);

			if (*str == ',')
				NOTIMP();
		} else if (*str != ')') {
			ERROR("hmm: %s", str);
		}
	} else {
		NOTIMP();
	}
}

void flush_whitespace(const char **str) {
	while (isspace(**str)) {
		(*str)++;
	}
}

void parse_instruction(const char *str, const char **mnemonic, struct operand ops[4]) {
	flush_whitespace(&str);
	char mnemonic_buffer[32];
	int i = 0;
	for (; *str; i++, str++) {
		if (isspace(*str))
			break;
		mnemonic_buffer[i] = *str;
	}
	mnemonic_buffer[i] = '\0';

	*mnemonic = strdup(mnemonic_buffer);

	int curr_op = 0;
	for (; *str; str++) {
		flush_whitespace(&str);
		int idx = 0;
		char arg_buffer[100];

		int in_paren = 0;
		for (; *str; str++) {
			if (!in_paren && *str == ',')
				break;
			if (*str == '(')
				in_paren = 1;
			else if (*str == ')')
				in_paren = 0;
			arg_buffer[idx++] = *str;
		}
		arg_buffer[idx] = '\0';

		parse_operand(arg_buffer, ops + curr_op++);
		if (!*str)
			break;
	}

	for (; curr_op < 4; curr_op++) {
		ops[curr_op].type = O_EMPTY;
	}
}

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

struct test_data {
	const char *instruction;
	int output_len;
	uint8_t output[15];
};

#include "../test_data.h"

int main(int argc, char **argv) {
	(void)argc, (void)argv;
	uint8_t output[15] = { 0 };
	int len;

	struct operand ops[4];
	const char *mnemonic;

	int all_match = 1;
	for (unsigned i = 0; i < sizeof test_data / sizeof *test_data; i++) {
		//printf("Currently testing %s\n", test_data[i].instruction);
		parse_instruction(test_data[i].instruction, &mnemonic, ops);
		flip_order(ops);

		assemble_instruction(output, &len, mnemonic, ops);

		int matches = 1;
		if (len != test_data[i].output_len) {
			matches = 0;
		} else {
			for (int j = 0; j < len; j++) {
				if (output[j] != test_data[i].output[j]) {
					matches = 0;
					break;
				}
			}
		}

		if (!matches) {
			printf("Didn't match!!!!\n");

			printf("%s\n", test_data[i].instruction);
			printf("Got: \n");
			for (int j = 0; j < len; j++) {
				printf("%02x ", output[j]);
			}
			printf("\n");
			printf("Expected: \n");
			for (int j = 0; j < test_data[i].output_len; j++) {
				printf("%02x ", test_data[i].output[j]);
			}
			printf("\n");

			all_match = 0;

			return 1;
		}
	}

	if (all_match)
		printf("All matched! Good job!\n");
}
