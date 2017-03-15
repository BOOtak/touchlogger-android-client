//
// Created by k.leyfer on 15.03.2017.
//

#include <stdio.h>

#include "elf_parser.h"

signed int get_elf_info(struct elf_header *elf_mmaped_area, struct dyn_info *a2)
{
  struct elf32_phdr *programm_header_entry; // r3@5
  int ph; // r2@5
  unsigned long segment_addr; // r0@10
  unsigned int v6; // r2@10

  if ( elf_mmaped_area->raw_data[0] == 0x7F
       && elf_mmaped_area->raw_data[1] == 0x45
       && elf_mmaped_area->raw_data[2] == 0x4C
       && elf_mmaped_area->raw_data[3] == 0x46 )   // ELF header
  {
    programm_header_entry = (struct elf32_phdr *)&elf_mmaped_area->raw_data[elf_mmaped_area->e_phoff];
    ph = 0;
    while ( ph < elf_mmaped_area->e_phnum )
    {
      if ( programm_header_entry->p_type == 2 ) // dynamic linking information
      {
        printf("dynamic linking info found\n");
        segment_addr = ((unsigned long)&elf_mmaped_area->raw_data[programm_header_entry->p_offset] - (unsigned long)elf_mmaped_area);
        printf("segment addr: 0x%lx, base addr: 0x%lx\n", (unsigned long)segment_addr, (unsigned long)elf_mmaped_area);
        v6 = 0;
        printf("%d entries\n", programm_header_entry->p_filesz >> 3);
        while ( v6 < programm_header_entry->p_filesz >> 3 )
        {
          unsigned long value = (unsigned long)(elf_mmaped_area->raw_data[segment_addr]);
          printf("segment addr value: %lu\n", value);
          if ( value == 5 )   // address of the string table
          {
            a2->str_table_addr = (unsigned long)segment_addr;
            printf("str_table_addr found\n");
          }
          else if ( value == 1 )                   // string table offset of a needed library
          {
            a2->dependency_offset_addr = (unsigned long)segment_addr;
            printf("dependency_offset_addr found\n");
          }
          else if ( value == 0xE )                 // the name of the shared object
          {
            a2->llibname_offset_addr = (unsigned long)segment_addr;
            printf("llibname_offset_addr found\n");
          }
          ++v6;
          segment_addr += 8;
        }
        return 0;
      }
      ++ph;
      ++programm_header_entry;                  // pointer += sizeof(elf32_phdr) (0x20)
    }
  }
  return -1;
}
