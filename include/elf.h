// Format of an ELF executable file

#ifndef INCLUDE_ELF_H_
#define INCLUDE_ELF_H_

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian


// File header
struct elf_t {
    u32         e_magic;  // must equal ELF_MAGIC
    u8      	e_ident[12];
    u16         e_type;
    u16         e_machine;
    u32         e_version;
    u32         e_entry;
    u32         e_phoff;
    u32         e_shoff;
    u32         e_flags;
    u16         e_ehsize;
    u16         e_phentsize;
    u16         e_phnum;
    u16         e_shentsize;
    u16         e_shnum;
    u16         e_shstrndx;
};

//Section header
struct sec_t {
    u32     sh_name;
    u32     sh_type;
    u32     sh_flag;
    u32     sh_addr;
    u32     sh_offset;
    u32     sh_size;
    u32     sh_link;
    u32     sh_info;
    u32     sh_addralign;
    u32     sh_entsize;
};

//Symbol table
struct symtable_t {
    u32     st_name;
    u32     st_value;
    u32     st_size;
    u8      st_info;
    u8      st_other;
    u16     st_shndx;
};

// Program section header
struct prog_t {
    u32         ph_type;
    u32         ph_off;
    u32         ph_vaddr;
    u32         ph_paddr;
    u32         ph_filesz;
    u32         ph_memsz;
    u32         ph_flags;
    u32         ph_align;
};


#endif
