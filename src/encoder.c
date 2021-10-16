#include "encoder.h"
#include "instructions.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define ERROR(STR, ...) do { printf("Error on line %d file %s: \"" STR "\"\n", __LINE__, __FILE__, ##__VA_ARGS__); exit(1); } while(0)
#define NOTIMP() ERROR("Not implemented");


int get_disp_size(uint64_t disp_u) {
	long disp = disp_u;
	if (disp == 0)
		return 0;
	if (disp >= INT8_MIN && disp <= INT8_MAX)
		return 1;
	if (disp >= INT32_MIN && disp <= INT32_MAX)
		return 4;
	parse_send_error("Error! Invalid displacement size");
	ERROR("Invalid displacement size %ld", disp);
}

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
		parse_send_error("Not implemented");
		ERROR("Params: %d (%d, %d, %d)", disp_size, o->sib.base, o->sib.index, o->sib.scale);
	}

	// disp8(%base, %scale, index)
	// disp32(%base, %scale, index)
}

//struct encoding cmp2 = {0x83, .operand_encoding = {{OE_MODRM_RM}, {OE_IMM8}}};

void assemble_encoding(uint8_t *output, int *len, struct encoding *encoding, struct operand ops[4], char **reloc_name, int *reloc_offset, int *reloc_relative) {
	int has_imm8 = 0, has_imm16 = 0, has_imm32 = 0, has_imm64 = 0;
	uint64_t imm = 0;
	char *imm_name = NULL;
	*reloc_offset = 0;
	*reloc_relative = 0;

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

	int has_rel32 = 0;
	uint32_t rel = 0;

	for (int i = 0, j = 0; i < 4 && j < 4; i++, j++) {
		struct operand *o = ops + i;
		struct operand_encoding *oe = encoding->operand_encoding + j;

		if ((o->type == O_EMPTY) != (oe->type == OE_EMPTY))
			ERROR("Invalid number of arguments to instruction");

		if (o->type == O_REG && o->reg.rex == 1)
			has_rex = 1;

		switch (oe->type) {
		case OE_IMM8:
			has_imm8 = 1;
			imm = o->imm.value;
			imm_name = o->imm.str;
			break;

		case OE_IMM16:
			has_imm16 = 1;
			imm = o->imm.value;
			imm_name = o->imm.str;
			break;

		case OE_IMM32:
			has_imm32 = 1;
			imm = o->imm.value;
			imm_name = o->imm.str;
			break;

		case OE_IMM64:
			has_imm64 = 1;
			imm = o->imm.value;
			imm_name = o->imm.str;
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

		case OE_REL32:
			has_rel32 = 1;
			rel = o->imm.value;
			imm_name = o->imm.str;
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
		*reloc_offset = idx;
		idx++;
	} else if (has_imm16) {
		*(uint16_t *)(output + idx) = imm;
		*reloc_offset = idx;
		idx += 2;
	} else if (has_imm32) {
		*(uint32_t *)(output + idx) = imm;
		*reloc_offset = idx;
		idx += 4;
	} else if (has_imm64) {
		*(uint64_t *)(output + idx) = imm;
		*reloc_offset = idx;
		idx += 8;
	}

	if (has_rel32) {
		*(uint32_t *)(output + idx) = rel;
		*reloc_offset = idx;
		*reloc_relative = 1;
		idx += 4;
	}

	*len = idx;

	*reloc_name = imm_name;
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
		long s = o->imm.value;
		if (o->type != O_IMM)
			return 0;
		if (s < INT8_MIN || s > INT8_MAX)
			return 0;

		if (o->imm.str)
			return 0;
	} break;

	case ACC_IMM16_S: {
		long s = o->imm.value;
		if (o->type != O_IMM)
			return 0;
		if (s < INT16_MIN || s > INT16_MAX)
			return 0;

		if (o->imm.str)
			return 0;
	} break;

	case ACC_IMM32_S: {
		long s = o->imm.value;
		if (o->type != O_IMM)
			return 0;
		if (s < INT32_MIN || s > INT32_MAX)
			return 0;
	} break;

	case ACC_IMM32_U: {
		if (o->type != O_IMM)
			return 0;
		if (o->imm.value > UINT32_MAX)
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

	case ACC_REL32:
		if (o->type != O_IMM_ABSOLUTE)
			return 0;
		break;

	default:
		ERROR("Not imp: %d\n", oa->type);
	}

	return 1;
}

void assemble_instruction(uint8_t *output, int *len, const char *mnemonic, struct operand ops[4], char **reloc_name, int *reloc_offset, int *reloc_relative) {
	int best_len = 16;
	uint8_t best_output[15] = { 0 };
	char *best_name = NULL;
	int best_offset = 0;
	int best_relative = 0;
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
		char *current_reloc_name = NULL;
		int current_reloc_offset = 0;
		int current_reloc_relative = 0;
		assemble_encoding(current_output, &current_len, encoding, ops, &current_reloc_name, &current_reloc_offset, &current_reloc_relative);

		if (current_len < best_len) {
			memcpy(best_output, current_output, sizeof (best_output));
			best_len = current_len;
			best_name = current_reloc_name;
			best_offset = current_reloc_offset;
			best_relative = current_reloc_relative;
		}
	}

	if (best_len == 16) {
		*len = -1;
		return;
	}

	*len = best_len;
	memcpy(output, best_output, sizeof (best_output));
	*reloc_name = best_name;
	*reloc_offset = best_offset;
	*reloc_relative = best_relative;
}
