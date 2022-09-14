/* coding:utf-8
 * Copyright (C) dirlt
 */

#include <assert.h>
#include <elf.h>
#include <memory.h>
#include <stdio.h>
#include <utils.h>

#ifdef CONFIG_FTRACE
static FuncEntry func_entries[256];
static int func_entry_number = 0;
static char string_table[102400];

void read_func_entries(const char* fname) {
    FILE* fp = fopen(fname, "r");
    Elf64_Ehdr header;
    size_t ret = 0;
    ret = fread(&header, sizeof(header), 1, fp);
    printf("section number = %d\n", header.e_shnum);

    int string_position = 0;
    fseek(fp, header.e_shoff, SEEK_SET);
    for (size_t i = 0; i < header.e_shnum; i++) {
        Elf64_Shdr sh;
        ret = fread(&sh, sizeof(sh), 1, fp);
        size_t now = ftell(fp);
        if (sh.sh_type == SHT_STRTAB) {
            // printf("str: pos = 0x%lx, size = %ld, number = %ld\n", sh.sh_offset, sh.sh_size, sh.sh_entsize);
            fseek(fp, sh.sh_offset, SEEK_SET);
            ret = fread(string_table + string_position, 1, sh.sh_size, fp);
            string_position += sh.sh_size;
            fseek(fp, now, SEEK_SET);
        }
    }

    fseek(fp, header.e_shoff, SEEK_SET);
    for (size_t i = 0; i < header.e_shnum; i++) {
        Elf64_Shdr sh;
        ret = fread(&sh, sizeof(sh), 1, fp);
        size_t now = ftell(fp);

        if (sh.sh_type == SHT_SYMTAB) {
            // printf("sym: pos = 0x%lx, size = %ld, number = %ld\n", sh.sh_offset, sh.sh_size, sh.sh_entsize);
            Elf64_Sym sym;
            assert(sh.sh_entsize == sizeof(sym));
            size_t number = sh.sh_size / sizeof(sym);
            // assert(sh.sh_entsize * sizeof(sym) == sh.sh_size);
            fseek(fp, sh.sh_offset, SEEK_SET);
            for (size_t i = 0; i < number; i++) {
                ret = fread(&sym, sizeof(sym), 1, fp);
                if (ELF64_ST_TYPE(sym.st_info) == STT_FUNC) {
                    const char* name = string_table + sym.st_name;
                    // printf("func: type = %d, name = %s(%d), addr = 0x%lx, size = 0x%lx\n", sym.st_info, name,
                    //        sym.st_name, sym.st_value, sym.st_size);
                    FuncEntry* entry = &func_entries[func_entry_number++];
                    entry->addr = sym.st_value;
                    entry->size = sym.st_size;
                    entry->name = name;
                }
            }
            fseek(fp, now, SEEK_SET);
        }
    }
    (void)ret;
    fclose(fp);
}

FuncEntry* list_func_entries(int* size) {
    *size = func_entry_number;
    return func_entries;
}
#endif