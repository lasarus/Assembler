#include "elf.h"
#include "darray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR(STR, ...) do { printf("Error on line %d file %s: \"" STR "\"\n", __LINE__, __FILE__, ##__VA_ARGS__); exit(1); } while(0)
#define NOTIMP() ERROR("Not implemented");

static size_t shstring_size, shstring_cap;
static char *shstrings = NULL;

int register_shstring(const char *str) {
	char *space = ADD_ELEMENTS(shstring_size, shstring_cap, shstrings, strlen(str) + 1);

	strcpy(space, str);

	return space - shstrings;
}

static size_t string_size, string_cap;
static char *strings = NULL;

int register_string(const char *str) {
	char *space = ADD_ELEMENTS(string_size, string_cap, strings, strlen(str) + 1);

	strcpy(space, str);

	return space - strings;
}

struct section {
	const char *name;
	int idx;
	int sh_idx;

	size_t size, cap;
	uint8_t *data;

};

size_t section_size, section_cap;
struct section *sections = NULL;

struct section *current_section = NULL;

void elf_init(void) {
	elf_set_section(".text");
	register_string("");
}

void elf_set_section(const char *section) {
	for (unsigned i = 0; i < section_size; i++) {
		if (strcmp(sections[i].name, section) == 0) {
			current_section = sections + i;
			return;
		}
	}

	current_section = &ADD_ELEMENT(section_size, section_cap, sections);
	*current_section = (struct section) {
		.name = strdup(section),
		.idx = section_size - 1,
	};
}

void elf_write(uint8_t *data, int len) {
	memcpy(ADD_ELEMENTS(current_section->size, current_section->cap, current_section->data, len),
		   data, len);
}

void elf_write_zero(int len) {
	memset(ADD_ELEMENTS(current_section->size, current_section->cap, current_section->data, len),
		   0, len);
}

struct symbol {
	int string_idx; // gotten from register_string()
	char *name;
	uint64_t value;
	uint64_t size;
	int section;
	int global;
};

size_t symbol_size, symbol_cap;
struct symbol *symbols;

static int find_symbol(const char *name) {
	for (unsigned i = 0; i < symbol_size; i++)
		if (strcmp(symbols[i].name, name) == 0)
			return i;
	return -1;
}

int elf_create_symbol(const char *name, int64_t offset) {
	ADD_ELEMENT(symbol_size, symbol_cap, symbols) = (struct symbol) {
		.name = strdup(name),
		.string_idx = register_string(name)
	};

	return symbol_size - 1;
}

int elf_new_symbol(const char *name) {
	ADD_ELEMENT(symbol_size, symbol_cap, symbols) = (struct symbol) {
		.name = strdup(name),
		.string_idx = register_string(name)
	};

	return symbol_size - 1;
}

void elf_symbol_relocate_here(const char *name, int64_t offset) {
	NOTIMP();
}

void elf_symbol_set_here(const char *name, int64_t offset) {
	int idx = find_symbol(name);
	if (idx == -1)
		idx = elf_new_symbol(name);

	symbols[idx].section = current_section->idx;
	symbols[idx].value = current_section->size + offset;
}

void elf_symbol_set_global(const char *name) {
	int idx = find_symbol(name);
	if (idx == -1)
		idx = elf_new_symbol(name);

	symbols[idx].global = 1;
}

static FILE *output = NULL;
size_t current_pos = 0;

void write(const void *ptr, size_t size) {
	current_pos += size;
	if (fwrite(ptr, size, 1, output) != 1)
		ERROR("Could not write to file");
}

void write_null(size_t size) {
	if (size == 0)
		return;

	if (size > 512)
		NOTIMP();

	uint8_t zero[512] = { 0 };
	write(zero, size);
}

void write_skip(size_t target) {
	write_null(target - current_pos);
}

void write_byte(uint8_t byte) {
	write(&byte, 1);
}

// TODO: Endianness??
void write_word(uint16_t word) {
	write(&word, 2);
}

void write_long(uint32_t long_) {
	write(&long_, 4);
}

void write_quad(uint64_t quad) {
	write(&quad, 8);
}

#define SH_OFF 128

enum {
	SHT_NULL = 0,
	SHT_PROGBITS = 1,
	SHT_SYMTAB = 2,
	SHT_STRTAB = 3,
	SHT_RELA = 4,
	SHT_NOBITS = 8,
};

enum {
	STB_LOCAL = 0,
	STB_GLOBAL = 1,
};

struct elf_section_header {
	uint32_t sh_name, sh_type;
	uint64_t sh_flags, sh_addr, sh_offset /*, sh_size*/;
	uint32_t sh_link, sh_info;
	uint64_t sh_addralign, sh_entsize;
};

struct elf_section {
	struct elf_section_header header;
	size_t size;
	uint8_t *data;
};

static size_t elf_section_size, elf_section_cap;
static struct elf_section *elf_sections = NULL;

uint64_t sh_address = 0;

static void write_header(int shstrndx) {
	static uint8_t magic[4] = {0x7f, 0x45, 0x4c, 0x46};
	write(magic, sizeof magic);

	write_byte(2); // EI_CLASS = 64 bit
	write_byte(1); // EI_DATA = little endian
	write_byte(1); // EI_VARSION = 1
	write_byte(0); // EI_OSABI = System V
	write_byte(0); // EI_ABIVERSION = 0

	write_null(7); // Padding

	write_word(1); // e_type = ET_REL
	write_word(0x3e); // e_machine = AMD x86-64

	write_long(1); // e_version = 1

	write_quad(0); // e_entry = ??
	write_quad(0); // e_phoff = ??
	write_quad(SH_OFF); // e_shoff = ??

	write_long(0); // e_flags = 0
	write_word(64); // e_ehsize = 64

	write_word(0); // e_phentsize = 0
	write_word(0); // e_phnum = 0
	write_word(64); // e_shentsize = 0
	write_word(elf_section_size); // e_shnum = 0
	write_word(shstrndx); // e_shstrndx = 0
}

struct elf_section *add_elf_section(void) {
	return &ADD_ELEMENT(elf_section_size, elf_section_cap, elf_sections);
}

int elf_add_section(uint32_t name, uint32_t type) {
	struct elf_section *section = add_elf_section();
	section->header = (struct elf_section_header) { .sh_name = name, .sh_type = type };
	section->size = 0;
	section->data = NULL;

	return section - elf_sections;
}

static void write_section_header(struct elf_section_header *header, size_t size) {
	write_long(header->sh_name);
	write_long(header->sh_type);

	write_quad(header->sh_flags);
	write_quad(header->sh_addr);
	write_quad(header->sh_offset);
	write_quad(size);

	write_long(header->sh_link);
	write_long(header->sh_info);

	write_quad(header->sh_addralign);
	write_quad(header->sh_entsize);
}

static void write_section_headers(void) {
	write_skip(SH_OFF);

	for (unsigned i = 0; i < elf_section_size; i++) {
		struct elf_section *section = elf_sections + i;

		write_section_header(&section->header, section->size);
	}

	for (unsigned i = 0; i < elf_section_size; i++) {
		struct elf_section *section = elf_sections + i;

		if (section->size == 0)
			continue;

		write_skip(section->header.sh_offset);
		write(section->data, section->size);
		printf("Wrote %lu to %lu (%d)\n", section->size, section->header.sh_offset, i);
	}
}

static void allocate_sections(void) {
	uint64_t address = 128;
	sh_address = address;
	address += 64 * elf_section_size;
	for (unsigned i = 0; i < elf_section_size; i++) {
		struct elf_section *section = elf_sections + i;

		if (i == 0)
			continue;

		section->header.sh_offset = address;
		address += section->size;
	}
}

uint8_t *symbol_table_write(int *n_local) {
	uint8_t *buffer = calloc((symbol_size + 1), 24);

	*n_local = 1;
	for (unsigned i = 0; i < symbol_size; i++) {
		if (!symbols[i].global)
			(*n_local)++;
	}

	int curr_entry = 0;

	for (unsigned i = 0; i < symbol_size; i++) {
		if (symbols[i].global)
			continue;
		curr_entry++;
		uint8_t *ent_addr = buffer + (curr_entry) * 24;
		*(uint32_t *)(ent_addr + 0) = symbols[i].string_idx; // st_name
		*(uint8_t *)(ent_addr + 4) = 0; // st_info
		*(uint8_t *)(ent_addr + 5) = 0; // st_other
		*(uint16_t *)(ent_addr + 6) = sections[symbols[i].section].sh_idx; // st_shndx
		*(uint64_t *)(ent_addr + 8) = symbols[i].value; // st_value
		*(uint64_t *)(ent_addr + 16) = 0; // st_size
	}

	for (unsigned i = 0; i < symbol_size; i++) {
		if (!symbols[i].global)
			continue;
		curr_entry++;
		uint8_t *ent_addr = buffer + (curr_entry) * 24;
		*(uint32_t *)(ent_addr + 0) = symbols[i].string_idx; // st_name
		*(uint8_t *)(ent_addr + 4) = STB_GLOBAL << 4; // st_info
		*(uint8_t *)(ent_addr + 5) = 0; // st_other
		*(uint16_t *)(ent_addr + 6) = sections[symbols[i].section].sh_idx; // st_shndx
		*(uint64_t *)(ent_addr + 8) = symbols[i].value; // st_value
		*(uint64_t *)(ent_addr + 16) = 0; // st_size
	}

	return buffer;
}

void elf_finish(const char *path) {
	output = fopen(path, "wb");
	int null_section = elf_add_section(register_shstring(""), SHT_NULL);

	for (unsigned i = 0; i < section_size; i++) {
		struct section *section = sections + i;
		int id = elf_add_section(register_shstring(section->name), SHT_PROGBITS);

		elf_sections[id].size = section->size;
		elf_sections[id].data = section->data;

		section->sh_idx = id;
	}

	int sym = elf_add_section(register_shstring(".symtab"), SHT_SYMTAB);

	int strtab_section = elf_add_section(register_shstring(".strtab"), SHT_STRTAB);
	int shstrtab_section = elf_add_section(register_shstring(".shstrtab"), SHT_STRTAB);

	elf_sections[sym].header.sh_link = strtab_section;
	elf_sections[sym].header.sh_entsize = 24;
	elf_sections[sym].size = (symbol_size + 1) * 24;
	int n_local_symb = 0;
	elf_sections[sym].data = symbol_table_write(&n_local_symb);
	elf_sections[sym].header.sh_info = n_local_symb;

	elf_sections[shstrtab_section].size = shstring_size;
	elf_sections[shstrtab_section].data = (uint8_t *)shstrings;

	elf_sections[strtab_section].size = string_size;
	elf_sections[strtab_section].data = (uint8_t *)strings;

	allocate_sections();
	write_header(shstrtab_section);
	write_section_headers();
	fclose(output);
}

