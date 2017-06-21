//
// Created by k.leyfer on 15.03.2017.
//

#include <stdio.h>

#include "elf_parser.h"
#include "libdirty/dirty_copy.h"

signed int get_elf_info(struct elf32_hdr *elf_mmaped_area, struct dyn_info *info)
{
  struct elf32_phdr *programm_header_entry;
  int ph;
  unsigned long segment_addr;
  unsigned int i;

  if (elf_mmaped_area->e_ident[0] == ELFMAG0
      && elf_mmaped_area->e_ident[1] == ELFMAG1
      && elf_mmaped_area->e_ident[2] == ELFMAG2
      && elf_mmaped_area->e_ident[3] == ELFMAG3)
  {
    programm_header_entry = (struct elf32_phdr *) &elf_mmaped_area->e_ident[elf_mmaped_area->e_phoff];
    ph = 0;
    while (ph < elf_mmaped_area->e_phnum)
    {
      if (programm_header_entry->p_type == PT_DYNAMIC)
      {
        LOGV("dynamic linking info found");
        segment_addr = ((unsigned long) &elf_mmaped_area->e_ident[programm_header_entry->p_offset] -
                        (unsigned long) elf_mmaped_area);
        LOGV("segment addr: 0x%lx, base addr: 0x%lx", segment_addr,
             (unsigned long) elf_mmaped_area);
        i = 0;
        int found = 0;
        info->DT_NEEDED_addrs.size = programm_header_entry->p_filesz >> 3;
        info->DT_NEEDED_addrs.addr = (unsigned long *) malloc(
            sizeof(unsigned long) * programm_header_entry->p_filesz >> 3);
        LOGV("%d entries", programm_header_entry->p_filesz >> 3);
        while (i < programm_header_entry->p_filesz >> 3)
        {
          unsigned long value = (unsigned long) (elf_mmaped_area->e_ident[segment_addr]);
          LOGV("segment addr value: %lu", value);
          if (value == DT_STRTAB)
          {
            info->DT_STRTAB_addr = segment_addr;
            LOGV("DT_STRTAB_addr found");
          }
          else if (value == DT_NEEDED)
          {
            (info->DT_NEEDED_addrs.addr)[found] = segment_addr;
            found++;
            LOGV("DT_NEEDED_addr found");
          }
          else if (value == DT_SONAME)
          {
            info->DT_SONAME_addr = segment_addr;
            LOGV("DT_SONAME_addr found");
          }

          ++i;
          segment_addr += 8;
        }

        info->DT_NEEDED_addrs.size = found;
        return 0;
      }

      ++ph;
      ++programm_header_entry;                  // pointer += sizeof(elf32_phdr) (0x20)
    }
  }

  return -1;
}

int get_strtable_values(void* elf_mmaped_data, struct dependencies_info* dependencies, struct strtab_entry* soname)
{
  struct dyn_info info = {
      0, 0, 0
  };

  get_elf_info(elf_mmaped_data, &info);

  unsigned long strtable_addr_value = *((unsigned long*)(elf_mmaped_data + info.DT_STRTAB_addr + 4));
  dependencies->size = info.DT_NEEDED_addrs.size;
  dependencies->entries = (struct strtab_entry*) malloc(sizeof(struct strtab_entry) * dependencies->size);

  for (int i = 0; i < info.DT_NEEDED_addrs.size; ++i) {
    unsigned long DT_NEEDED_addr = info.DT_NEEDED_addrs.addr[i];
    dependencies->entries[i].type = (unsigned long*)(elf_mmaped_data + DT_NEEDED_addr);
    unsigned long needed_offset = *((unsigned long*)(elf_mmaped_data + DT_NEEDED_addr + 4));
    dependencies->entries[i].value = (char*)(elf_mmaped_data + strtable_addr_value + needed_offset);
    LOGV("needed: %s", dependencies->entries[i].value);
  }

  if (info.DT_SONAME_addr != 0)
  {
    soname->type = (unsigned long*)(elf_mmaped_data + info.DT_SONAME_addr);
    unsigned long soname_offset = *((unsigned long*)(elf_mmaped_data + info.DT_SONAME_addr + 4));
    soname->value = (char*)(elf_mmaped_data + strtable_addr_value + soname_offset);
    LOGV("soname: %s", soname->value);
  }

  free(info.DT_NEEDED_addrs.addr);
}