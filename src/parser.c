#include "parser.h"

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define ERROR(STR, ...) do { printf("Error on line %d file %s: \"" STR "\"\n", __LINE__, __FILE__, ##__VA_ARGS__); exit(1); } while(0)
#define NOTIMP() ERROR("Not implemented");

// Input.
static char input[2];
static FILE *fp = NULL;

static int line = 1;

static void input_next(void) {
	input[0] = input[1];

	if (input[0] == '\n')
		line++;

	if (feof(fp)) {
		input[1] = '\0';
	} else {
		int in_char = fgetc(fp);
		if (in_char == EOF)
			input[1] = '\0';
		else
			input[1] = in_char;
	}
}

static void input_expect(char c) {
	if (input[0] != c)
		ERROR("Expected character %c, but got %c on line %d\n", c, input[0], line);

	input_next();
}

int is_alpha(char c) {
	return (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z');
}

int is_identifier(char c) {
	return (c == '_' || is_alpha(c) || c == '.');
}

int is_digit(char c) {
	return (c >= '0' && c <= '9');
}

int is_start_of_number(char c) {
	return is_digit(c) || c == '+' || c == '-';
}

// Todo: make this crash when exceeding buffer size.
int input_get_identifier(char *buffer) {
	if (!is_identifier(input[0]))
		return 0;

	while (is_identifier(input[0]) || is_digit(input[0])) {
		*(buffer++) = input[0];
		input_next();
	}

	*buffer = '\0';

	return 1;
}

// This implements a trie for finding registers.
// It is not optimized for readability.
int input_get_register_trie(enum reg *reg, int *size, int *rex) {
	switch(input[0]) {
	case 'a': input_next();
		switch(input[0]) {
		case 'h': input_next();
			*reg = REG_RSP; *size = 1; *rex = -1; return 1;
		case 'l': input_next();
			*reg = REG_RAX; *size = 1; *rex = 0; return 1;
		case 'x': input_next();
			*reg = REG_RAX; *size = 2; *rex = 0; return 1;
		default: return 0;
		}
	case 'b': input_next();
		switch(input[0]) {
		case 'h': input_next();
			*reg = REG_RDI; *size = 1; *rex = -1; return 1;
		case 'l': input_next();
			*reg = REG_RBX; *size = 1; *rex = 0; return 1;
		case 'p': input_next();
			switch(input[0]) {
			case 'l': input_next();
				*reg = REG_RBP; *size = 1; *rex = 1; return 1;
			default:
				*reg = REG_RBP; *size = 2; *rex = 0; return 1;
			}
		case 'x': input_next();
			*reg = REG_RBX; *size = 2; *rex = 0; return 1;
		default: return 0;
		}
	case 'c': input_next();
		switch(input[0]) {
		case 'h': input_next();
			*reg = REG_RBP; *size = 1; *rex = -1; return 1;
		case 'l': input_next();
			*reg = REG_RCX; *size = 1; *rex = 0; return 1;
		case 'x': input_next();
			*reg = REG_RCX; *size = 2; *rex = 0; return 1;
		default: return 0;
		}
	case 'd': input_next();
		switch(input[0]) {
		case 'h': input_next();
			*reg = REG_RSI; *size = 1; *rex = -1; return 1;
		case 'i': input_next();
			switch(input[0]) {
			case 'l': input_next();
				*reg = REG_RDI; *size = 1; *rex = 1; return 1;
			default:
				*reg = REG_RDI; *size = 2; *rex = 0; return 1;
			}
		case 'l': input_next();
			*reg = REG_RDX; *size = 1; *rex = 0; return 1;
		case 'x': input_next();
			*reg = REG_RDX; *size = 2; *rex = 0; return 1;
		default: return 0;
		}
	case 'e': input_next();
		switch(input[0]) {
		case 'a': input_next();
			input_expect('x');
			*reg = REG_RAX; *size = 4; *rex = 0; return 1;
		case 'b': input_next();
			switch(input[0]) {
			case 'p': input_next();
				*reg = REG_RBP; *size = 4; *rex = 0; return 1;
			case 'x': input_next();
				*reg = REG_RBX; *size = 4; *rex = 0; return 1;
			default: return 0;
			}
		case 'c': input_next();
			input_expect('x');
			*reg = REG_RCX; *size = 4; *rex = 0; return 1;
		case 'd': input_next();
			switch(input[0]) {
			case 'i': input_next();
				*reg = REG_RDI; *size = 4; *rex = 0; return 1;
			case 'x': input_next();
				*reg = REG_RDX; *size = 4; *rex = 0; return 1;
			default: return 0;
			}
		case 's': input_next();
			switch(input[0]) {
			case 'i': input_next();
				*reg = REG_RSI; *size = 4; *rex = 0; return 1;
			case 'p': input_next();
				*reg = REG_RSP; *size = 4; *rex = 0; return 1;
			default: return 0;
			}
		default: return 0;
		}
	case 'r': input_next();
		switch(input[0]) {
		case '1': input_next();
			switch(input[0]) {
			case '0': input_next();
				switch(input[0]) {
				case 'd': input_next();
					*reg = REG_R10; *size = 4; *rex = 0; return 1;
				case 'l': input_next();
					*reg = REG_R10; *size = 1; *rex = 0; return 1;
				case 'w': input_next();
					*reg = REG_R10; *size = 2; *rex = 0; return 1;
				default:
					*reg = REG_R10; *size = 8; *rex = 0; return 1;
				}
			case '1': input_next();
				switch(input[0]) {
				case 'd': input_next();
					*reg = REG_R11; *size = 4; *rex = 0; return 1;
				case 'l': input_next();
					*reg = REG_R11; *size = 1; *rex = 0; return 1;
				case 'w': input_next();
					*reg = REG_R11; *size = 2; *rex = 0; return 1;
				default:
					*reg = REG_R11; *size = 8; *rex = 0; return 1;
				}
			case '2': input_next();
				switch(input[0]) {
				case 'd': input_next();
					*reg = REG_R12; *size = 4; *rex = 0; return 1;
				case 'l': input_next();
					*reg = REG_R12; *size = 1; *rex = 0; return 1;
				case 'w': input_next();
					*reg = REG_R12; *size = 2; *rex = 0; return 1;
				default:
					*reg = REG_R12; *size = 8; *rex = 0; return 1;
				}
			case '3': input_next();
				switch(input[0]) {
				case 'd': input_next();
					*reg = REG_R13; *size = 4; *rex = 0; return 1;
				case 'l': input_next();
					*reg = REG_R13; *size = 1; *rex = 0; return 1;
				case 'w': input_next();
					*reg = REG_R13; *size = 2; *rex = 0; return 1;
				default:
					*reg = REG_R13; *size = 8; *rex = 0; return 1;
				}
			case '4': input_next();
				switch(input[0]) {
				case 'd': input_next();
					*reg = REG_R14; *size = 4; *rex = 0; return 1;
				case 'l': input_next();
					*reg = REG_R14; *size = 1; *rex = 0; return 1;
				case 'w': input_next();
					*reg = REG_R14; *size = 2; *rex = 0; return 1;
				default:
					*reg = REG_R14; *size = 8; *rex = 0; return 1;
				}
			case '5': input_next();
				switch(input[0]) {
				case 'd': input_next();
					*reg = REG_R15; *size = 4; *rex = 0; return 1;
				case 'l': input_next();
					*reg = REG_R15; *size = 1; *rex = 0; return 1;
				case 'w': input_next();
					*reg = REG_R15; *size = 2; *rex = 0; return 1;
				default:
					*reg = REG_R15; *size = 8; *rex = 0; return 1;
				}
			default: return 0;
			}
		case '8': input_next();
			switch(input[0]) {
			case 'd': input_next();
				*reg = REG_R8; *size = 4; *rex = 0; return 1;
			case 'l': input_next();
				*reg = REG_R8; *size = 1; *rex = 0; return 1;
			case 'w': input_next();
				*reg = REG_R8; *size = 2; *rex = 0; return 1;
			default:
				*reg = REG_R8; *size = 8; *rex = 0; return 1;
			}
		case '9': input_next();
			switch(input[0]) {
			case 'd': input_next();
				*reg = REG_R9; *size = 4; *rex = 0; return 1;
			case 'l': input_next();
				*reg = REG_R9; *size = 1; *rex = 0; return 1;
			case 'w': input_next();
				*reg = REG_R9; *size = 2; *rex = 0; return 1;
			default:
				*reg = REG_R9; *size = 8; *rex = 0; return 1;
			}
		case 'a': input_next();
			input_expect('x');
			*reg = REG_RAX; *size = 8; *rex = 0; return 1;
		case 'b': input_next();
			switch(input[0]) {
			case 'p': input_next();
				*reg = REG_RBP; *size = 8; *rex = 0; return 1;
			case 'x': input_next();
				*reg = REG_RBX; *size = 8; *rex = 0; return 1;
			default: return 0;
			}
		case 'c': input_next();
			input_expect('x');
			*reg = REG_RCX; *size = 8; *rex = 0; return 1;
		case 'd': input_next();
			switch(input[0]) {
			case 'i': input_next();
				*reg = REG_RDI; *size = 8; *rex = 0; return 1;
			case 'x': input_next();
				*reg = REG_RDX; *size = 8; *rex = 0; return 1;
			default: return 0;
			}
		case 's': input_next();
			switch(input[0]) {
			case 'i': input_next();
				*reg = REG_RSI; *size = 8; *rex = 0; return 1;
			case 'p': input_next();
				*reg = REG_RSP; *size = 8; *rex = 0; return 1;
			default: return 0;
			}
		default: return 0;
		}
	case 's': input_next();
		switch(input[0]) {
		case 'i': input_next();
			switch(input[0]) {
			case 'l': input_next();
				*reg = REG_RSI; *size = 1; *rex = 1; return 1;
			default:
				*reg = REG_RSI; *size = 2; *rex = 0; return 1;
			}
		case 'p': input_next();
			switch(input[0]) {
			case 'l': input_next();
				*reg = REG_RSP; *size = 1; *rex = 1; return 1;
			default:
				*reg = REG_RSP; *size = 2; *rex = 0; return 1;
			}
		default: return 0;
		}
	default: return 0;
	}
}

int input_get_register(enum reg *reg, int *size, int *rex) {
	if (input[0] != '%')
		return 0;

	input_next();

	if (!input_get_register_trie(reg, size, rex)) {
		ERROR("Invalid register");
	}

	return 1;
}

int input_get_number(uint64_t *immediate) {
	if (!is_start_of_number(input[0]))
		return 0;

	char buffer[256];
	int idx = 0;
	while (is_digit(input[0]) || input[0] == '-') {
		buffer[idx++] = input[0];
		input_next();
	}
	buffer[idx] = '\0';

	if (input[0] == '-')
		*immediate = strtol(buffer, NULL, 10);
	else
		*immediate = strtoul(buffer, NULL, 10);

	return 1;
}

int input_get_immediate(uint64_t *immediate) {
	if (input[0] != '$' || !is_start_of_number(input[1]))
		return 0;

	input_next();

	if (!input_get_number(immediate))
		ERROR("Expected number after $ on line %d, got char %c", line, input[0]);

	return 1;
}

int input_get_immediate_identifier(char *buffer) {
	if (input[0] != '$')
		return 0;

	input_next();

	if (!input_get_identifier(buffer))
		ERROR("Expected number after $ on line %d, got char %c", line, input[0]);

	return 1;
}

int input_get_string(char *buffer) {
	if (input[0] != '"')
		return 0;

	input_next();
	while (input[0] != '"') {
		*(buffer++) = input[0];
		input_next();
	}

	*buffer = '\0';
	input_next();

	return 1;
}

// Tokenizer.
struct token {
	enum token_type {
		T_EOF,
		T_NEWLINE,
		T_IDENTIFIER,
		T_REGISTER,
		T_STAR_REGISTER,
		T_NUMBER,
		T_IMMEDIATE,
		T_IMMEDIATE_IDENTIFIER,
		T_COMMA,
		T_COLON,
		T_LEFT_PARENTHESIS,
		T_RIGHT_PARENTHESIS,
		T_STRING
	} type;

	union {
		char *identifier;
		struct {
			enum reg reg;
			int size, rex;
		} register_;
		uint64_t immediate;
	};

	int line;
};

static struct token tokens[2];

int token_flush_whitespace(void) {
	if (input[0] == '\n' || !isspace(input[0]))
		return 0;

	while (input[0] != '\n' && isspace(input[0]))
		input_next();

	return 1;
}

int token_flush_comment(void) {
	if (input[0] != '#')
		return 0;

	while (input[0] != '\n' && input[0])
		input_next();

	return 1;
}

void token_next(void) {
	tokens[0] = tokens[1];

	tokens[1] = (struct token) { 0 };

	while (token_flush_whitespace() ||
		   token_flush_comment());

	int token_start_line = line;
	static char tok_buffer[256] = { 0 };

	if (input[0] == '\0') {
		tokens[1].type = T_EOF;
	} else if (input[0] == '(') {
		tokens[1].type = T_LEFT_PARENTHESIS;
		input_next();
	} else if (input[0] == ')') {
		tokens[1].type = T_RIGHT_PARENTHESIS;
		input_next();
	} else if (input[0] == ',') {
		tokens[1].type = T_COMMA;
		input_next();
	} else if (input[0] == ':') {
		tokens[1].type = T_COLON;
		input_next();
	} else if (input[0] == '\n') {
		tokens[1].type = T_NEWLINE;
		input_next();
	} else if (input_get_identifier(tok_buffer)) {
		tokens[1] = (struct token) {
			.type = T_IDENTIFIER,
			.identifier = strdup(tok_buffer)
		};
	} else if (input_get_string(tok_buffer)) {
		tokens[1] = (struct token) {
			.type = T_STRING,
			.identifier = strdup(tok_buffer)
		};
	} else if (input_get_register(&tokens[1].register_.reg,
								  &tokens[1].register_.size,
								  &tokens[1].register_.rex)) {
		tokens[1].type = T_REGISTER;
	} else if (input[0] == '*') {
		input_next();
		if (!input_get_register(&tokens[1].register_.reg,
								&tokens[1].register_.size,
								&tokens[1].register_.rex))
			ERROR("Expected register after *");
		tokens[1].type = T_STAR_REGISTER;
	} else if (input_get_immediate(&tokens[1].immediate)) {
		tokens[1].type = T_IMMEDIATE;
	} else if (input_get_immediate_identifier(tok_buffer)) {
		tokens[1] = (struct token) {
			.type = T_IMMEDIATE_IDENTIFIER,
			.identifier = strdup(tok_buffer)
		};
	} else if (input_get_number(&tokens[1].immediate)) {
		tokens[1].type = T_NUMBER;
	} else {
		ERROR("Invalid char %c, %d", input[0], input[0]);
	}

	tokens[1].line = token_start_line;
}

int token_accept(enum token_type type) {
	if (tokens[0].type != type)
		return 0;
	token_next();
	return 1;
}

void token_expect(enum token_type type) {
	if (!token_accept(type)) {
		if (tokens[0].type == T_IDENTIFIER) {
			printf("%s\n", tokens[0].identifier);
		}
		ERROR("Expected %d, but got %d on line %d", type, tokens[0].type, tokens[0].line);
	}
}

// Parser construction/destruction.
void parse_init(const char *path) {
	fp = fopen(path, "r");
	input_next();
	input_next();

	token_next();
	token_next();
}

void parse_close(void) {
	fclose(fp);
}

// Parse functions.
static int parse_sib(struct operand *operand) {
	if (tokens[0].type == T_NUMBER && tokens[1].type == T_LEFT_PARENTHESIS) {
		operand->sib.offset = tokens[0].immediate;
		token_next();
	} else if (tokens[0].type == T_LEFT_PARENTHESIS) {
		operand->sib.offset = 0;
	} else {
		return 0;
	}

	token_expect(T_LEFT_PARENTHESIS);

	if (tokens[0].type != T_REGISTER)
		ERROR("Expected register as first argument of SIB");

	operand->type = O_SIB;

	operand->sib.base = REG_NONE;
	operand->sib.scale = 1;
	operand->sib.index = REG_NONE;

	operand->sib.base = tokens[0].register_.reg;
	token_next();

	if (token_accept(T_COMMA)) {
		if (tokens[0].type != T_REGISTER)
			ERROR("Expected register as second argument of SIB");

		operand->sib.index = tokens[0].register_.reg;
		token_next();

		if (token_accept(T_COMMA)) {
			if (tokens[0].type != T_NUMBER)
				ERROR("Expected number as third argument of SIB");

			operand->sib.scale = tokens[0].immediate;
			token_next();
		}
	}

	token_expect(T_RIGHT_PARENTHESIS);

	return 1;
}

static int parse_operand(struct operand *operand) {
	switch (tokens[0].type) {
	case T_REGISTER:
		operand->type = O_REG;
		operand->reg.reg = tokens[0].register_.reg;
		operand->reg.size = tokens[0].register_.size;
		operand->reg.rex = tokens[0].register_.rex;
		token_next();
		return 1;

	case T_STAR_REGISTER:
		operand->type = O_REG_STAR;
		operand->reg.reg = tokens[0].register_.reg;
		operand->reg.size = tokens[0].register_.size;
		operand->reg.rex = tokens[0].register_.rex;
		token_next();
		return 1;

	case T_IMMEDIATE:
		operand->type = O_IMM;
		operand->imm.value = tokens[0].immediate;
		operand->imm.str = NULL;
		token_next();
		return 1;

	case T_IMMEDIATE_IDENTIFIER:
		operand->type = O_IMM;
		operand->imm.value = 0;
		operand->imm.str = tokens[0].identifier;
		token_next();
		return 1;

	default:
		return parse_sib(operand);
	}
}

int parse_instruction(struct instruction *instruction) {
	if (tokens[0].type != T_IDENTIFIER)
		return 0;

	instruction->mnemonic = tokens[0].identifier;
	token_next();

	for (int i = 0; i < 4; i++)
		instruction->operands[i].type = O_EMPTY;

	int op_idx = 0;
	do {
		parse_operand(instruction->operands + op_idx++);
	} while (token_accept(T_COMMA));

	token_expect(T_NEWLINE);

	return 1;
}

int parse_label(struct label *label) {
	if (tokens[0].type != T_IDENTIFIER ||
		tokens[1].type != T_COLON)
		return 0;

	label->name = tokens[0].identifier;
	token_next();
	token_next();

	return 1;
}

int parse_directive(struct directive *directive) {
	if (tokens[0].type != T_IDENTIFIER)
		return 0;

	char *name = tokens[0].identifier;
	if (strcmp(name, ".section") == 0) {
		token_next();
		if (tokens[0].type != T_IDENTIFIER)
			ERROR("Expected identifer on line %d", tokens[0].line);
		directive->type = DIR_SECTION;
		directive->name = tokens[0].identifier;
		token_next();
		token_expect(T_NEWLINE);
		return 1;
	} else if (strcmp(name, ".global") == 0) {
		token_next();
		if (tokens[0].type != T_IDENTIFIER)
			ERROR("Expected identifer on line %d", tokens[0].line);
		directive->type = DIR_GLOBAL;
		directive->name = tokens[0].identifier;
		token_next();
		token_expect(T_NEWLINE);
		return 1;
	} else if (strcmp(name, ".string") == 0) {
		token_next();
		if (tokens[0].type != T_STRING)
			ERROR("Expected string on line %d", tokens[0].line);
		directive->type = DIR_STRING;
		directive->name = tokens[0].identifier;
		token_next();
		token_expect(T_NEWLINE);
		return 1;
	} else if (strcmp(name, ".zero") == 0) {
		token_next();
		if (tokens[0].type != T_NUMBER)
			ERROR("Expected number on line %d", tokens[0].line);
		directive->type = DIR_ZERO;
		directive->immediate = tokens[0].immediate;
		token_next();
		token_expect(T_NEWLINE);
		return 1;
	} else if (strcmp(name, ".quad") == 0) {
		token_next();
		if (tokens[0].type != T_NUMBER)
			ERROR("Expected number on line %d", tokens[0].line);
		directive->type = DIR_QUAD;
		directive->immediate = tokens[0].immediate;
		token_next();
		token_expect(T_NEWLINE);
		return 1;
	}

	return 0;
}

int parse_is_eof(void) {
	return token_accept(T_EOF);
}

void parse_expect_empty_line(void) {
	token_expect(T_NEWLINE);
}

void parse_send_error(const char *message) {
	printf("Instruction error: \"%s\" on line: %d\n", message, tokens[0].line);
}
