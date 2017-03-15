//
// Created by k.leyfer on 15.03.2017.
//

#ifndef TOUCHLOGGER_DIRTY_ELF_PARSER_H
#define TOUCHLOGGER_DIRTY_ELF_PARSER_H

#include <sys/types.h>

struct elf_header {
  char raw_data[16];
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint32_t e_entry;
  uint32_t e_phoff;
  uint32_t e_shoff;
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrnd;
};

struct dyn_info {
  unsigned long str_table_addr;
  unsigned long dependency_offset_addr;
  unsigned long llibname_offset_addr;
};

struct elf32_phdr {
  int p_type;
  int p_offset;
  int p_vaddr;
  int p_paddr;
  int p_filesz;
  int p_memsz;
  int p_flags;
  int p_align;
};

signed int get_elf_info(struct elf_header *elf_mmaped_area, struct dyn_info *a2);


#endif //TOUCHLOGGER_DIRTY_ELF_PARSER_H
