#ifndef ELF_H
#define ELF_H

#include <stdint.h>

void elf_init(void);
void elf_set_section(const char *section);
void elf_write(uint8_t *data, int len);
void elf_write_zero(int len);
void elf_finish(const char *path);

void elf_symbol_relocate_here(const char *name, int64_t offset);
void elf_symbol_set_here(const char *name, int64_t offset);
void elf_symbol_set_global(const char *name);

#endif
