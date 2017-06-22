//
// Created by k.leyfer on 15.03.2017.
//

#ifndef TOUCHLOGGER_DIRTY_ELF_PARSER_H
#define TOUCHLOGGER_DIRTY_ELF_PARSER_H

#include <sys/types.h>
#include <linux/elf.h>

struct needed_info
{
  int size;
  unsigned long* addr;
};

struct dyn_info
{
  unsigned long DT_STRTAB_addr;
  unsigned long DT_SONAME_addr;
  struct needed_info DT_NEEDED_addrs;
};

struct strtab_entry
{
  unsigned long* type;
  char* value;
};

struct dependencies_info
{
  int size;
  struct strtab_entry* entries;
};

int get_elf_info(struct elf32_hdr* header, struct dyn_info* info);

int get_strtable_values(void* elf_mmaped_data, struct dependencies_info* dependencies,
                        struct strtab_entry* soname);

#endif //TOUCHLOGGER_DIRTY_ELF_PARSER_H
