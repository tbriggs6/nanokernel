#ifndef ELFSTRUCT_H
#define ELFSTRUCT_H
#include <stdint.h>


enum elf_class {
    ELF_BIT32 = 1, ELF_BIT64 = 2
};

enum elf_endianess {
    ELF_LITTLE = 1, ELF_BIG = 2
};

enum elf_abi {
    ABI_SYSV = 0x0, ABI_HPUX = 0x1, ABI_NETBSD = 0x2,
    ABI_LINUX = 0x3, ABI_HURD = 0x4, ABI_SOLARIS = 0x6, 
    ABI_AIX = 0x7, ABI_IRIX = 0x8, ABI_FBSD = 0x9, 
    ABI_TRU64 = 0xa, ABI_MODESTO = 0xb, ABI_OBSD = 0xc, 
    ABI_NSTOP = 0xe, ABI_AROS = 0x1f, ABI_FENIX = 0x10,
    ABI_CLOUD = 0x11
};

enum elf_type { 
    ET_NONE = 0x00, ET_REL = 0x01, 
    ET_EXEC = 0x02, ET_DYN = 0x03, 
    ET_CORE = 0x04, ET_LOOS = 0xfe00,
    ET_HIOS = 0xfeff, ET_LOPROC = 0xff00, 
    ET_HIPROC = 0xffff
};

enum elf_isa {
    ISA_NONE = 0x00, ISA_SPARC = 0x02, ISA_X86 = 0x03, 
    ISA_MIPS = 0x08, ISA_POWERPC = 0x14, ISA_S390 = 0x16, 
    ISA_ARM = 0x28, ISA_SUPERH = 0x2A, ISA_IA64 = 0x32, 
    ISA_AMD64 = 0x3E, ISA_AARCH64 = 0xB7, ISA_RISCV = 0xF3
};

enum elf_ptype {
    PT_NULL = 0, PT_LOAD = 0x01, PT_DYANMIC = 0x02, 
    PT_INTERP = 0x03, PT_NOTE = 0x04, PT_SHLIB = 0x05, 
    PT_PHDR = 0x06, PT_TLS = 0x07, 
    PT_LOOS = 0x60000000, PT_HIOS = 0x6FFFFFFF, 
    PT_LOPROC = 0x70000000, PT_HIPROC = 0x7FFFFFFF
};

enum elf_align {
    ALIGN_NONE0 = 0, ALIGN_BYTE=1, 
    ALIGN_SHORT = 2, ALIGN_ODD = 3,
    ALIGN_WORD = 4, ALIGN_QWORD =5
};

enum elf_section_type {
    SHT_NULL = 0x00, SHT_PROGBITS = 0x01, 
    SHT_SYMTAB = 0x02, SHT_STRTAB = 0x03, 
    SHT_RELA = 0x04, SHT_HASH = 0x05, 
    SHT_DYNAMIC = 0x06, SHT_NOTE = 0x07,
    SHT_NOBITS = 0x08, SHT_REL = 0x09,
    SHT_SHLIB = 0x0a, SHT_DYNSYM = 0x0b,
    SHT_INIT_ARRAY = 0x0e, SHT_FINI_ARRAY = 0x0f, 
    SHT_PREINIT_ARRAY = 0x10, SHT_GROUP = 0x11,
    SHT_SYMTAB_SHNDX = 0x12, SHT_NUM = 0x13,
    SHT_LOOOS = 0x60000000
};

enum elf_section_flags {
    SHF_WRITE = 0x01,           // writable
    SHF_ALLOC = 0x02,           // occupies memory during execution
    SHF_EXECINSTR = 0x04,       // executable 
    SHF_MERGE = 0x10,           // might be merged
    SHF_STRING = 0x20,          // contains null-terminated strings
    SHF_INFO_LINK = 0x40,       // 'sh_info' contains SHT index
    SHF_LINK_ORDER = 0x80,      // preserve order after combining
    SHF_OS_NONCONFORMING = 0x100,   // non-standard os handling required
    SHF_GROUP = 0x200,          // section is a member of a group
    SHF_TLS = 0x400,            // section holds thread-local data
    SHF_MASKOS = 0x0ff00000,    // os-specific
    SHF_MASKPROC = 0xf0000000,  // proc-specific
    SHF_ORDERED = 0x4000000,    // special ordering requirement (solaris)
    SHF_EXCLUDE = 0x8000000     // section is excluded unless referenced (solaris)
};


typedef struct {
    uint8_t ident_magic[4];     // ELF magic 0x7F 0x45 0x4C 0x46 (0x7F ELF)
    uint8_t ident_class;        // ELF32 or ELF64 class 
    uint8_t ident_data;         // Data endianness
    uint8_t ident_version;      // ELF version (1 = original and current version of ELF)
    uint8_t ident_osabi;        // OS ABI
    uint8_t ident_abiver;       // ABI version (depends on OS ABI)
    uint8_t pad[7];             // padding
    uint16_t type;              // Object file type
    uint16_t machine;           // machine instruction set architecture
    uint32_t version;           // version again??
    uint32_t entry_address;     // address of first instruction
    uint32_t header_offset;     // offset of the program header table
    uint32_t section_offset;    // offset of section header table
    uint32_t flags;             // flags in target architecture
    uint16_t ident_header_size;     // size of the ident header (should be 64 bytes)
    uint16_t program_header_size;   // size of a program header table entry
    uint16_t program_header_num;    // number of entries in program_header
    uint16_t section_header_size;   // size of a section header entry
    uint16_t section_header_num;    // number of entries in section header
    uint16_t section_header_names;  // index of the section table with the section names
} elf_header_t;


typedef struct {
    uint32_t p_type;        // type of segment 
    uint32_t p_offset;      // offset of segment in file image
    uint32_t p_vaddr;       // virtual address in memory
    uint32_t p_paddr;       // segment's physical address
    uint32_t p_filesz;       // size in bytes of segment in file (may be 0)
    uint32_t p_memsz;       // size in bytes of segment in memory (may be 0)
    uint32_t p_segflags;    // segment-dependent flags
    uint32_t p_align;       // alignment
} elf_program_header_t;

typedef struct {
    uint32_t sh_name;       // offset to a string in the .shstrtab for the name of this section
    uint32_t sh_type;       // type of header
    uint32_t sh_flags;      // identifies attributes of section
    uint32_t sh_addr;       // virtual address of the section in memory
    uint32_t sh_offset;     // offset of the section in the file image
    uint32_t sh_size;       // size in bytes of section in the file image (may be 0)
    uint32_t sh_link;       // section index of an associated section
    uint32_t sh_info;       // extra info about the section
    uint32_t sh_addralign;  // contains required alignment of section 
    uint32_t sh_entsize;    // contains the size, in bytes, of each entry for sections that 
                            //    contain fixed sized ewntries
} elf_section_header_t;

#define EOK         (0)
#define ENOMAGIC    (-100)
#define EFORMAT     (-101)
#define EENDIAN     (-102)
#define EVERSION    (-103)
#define EOSABI      (-104)
#define EABIVER     (-105)
#define ETYPE       (-106)
#define EMACHINE    (-107)
#define EFLAGS      (-108)
#define EHEADER     (-109)
#define ESTRUCT     (-110)
#endif