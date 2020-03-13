
#include <stdint.h>
#include <string.h>

#ifdef DEBUG
#include <stdio.h>
#include <stdlib.h>
#define DBPRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( 0 )
#define MEMCPY memcpy
#else
#include "../kstdlib.h"
#define DBPRINT(...) do{ } while ( 0 )
#define MEMCPY kmemcpy
#endif

#include "elfstruct.h"

static int validate_header(elf_header_t *header)
{
    const uint8_t expected_magic[] = { 0x7F, 0x45, 0x4c, 0x46 };
    int i;

    for (i = 0; i < 4; i++) {
        if (expected_magic[i] != header->ident_magic[i]) return ENOMAGIC;
    }

    if (header->ident_class != ELF_BIT32)
        return EFORMAT;

    if (header->ident_data != ELF_LITTLE)
        return EENDIAN;

    if (header->ident_version != 1)
        return EVERSION;
    
    if (header->ident_osabi != ABI_SYSV)
        return EOSABI;

    if (header->ident_abiver != 0)
        return EABIVER;
    
    if (header->type != ET_EXEC)
        return ETYPE;

    if (header->machine != ISA_X86)
        return EMACHINE;

    if (header->version != 1)
        return EVERSION;

    if (header->flags != 0)
        return EFLAGS;

    if (header->ident_header_size != sizeof(elf_header_t))
        return EHEADER;

    if (header->program_header_size != sizeof(elf_program_header_t))
        return EHEADER+1;

    if (header->section_header_size != sizeof(elf_section_header_t))
        return EHEADER+2;
    
    return EOK;
}


static int handle_program_header(const char *elf_start, const elf_program_header_t *pheader, void *(*virt_alloc)(uint32_t virt_start, uint32_t length))
{
    DBPRINT("PHEADER type:%u offset:%u vaddr:%x paddr:%x size:%u memsz:%u segflags:%u align:%u\n",
        pheader->p_type, pheader->p_offset, pheader->p_vaddr,
        pheader->p_paddr, pheader->p_filesz, pheader->p_memsz,
        pheader->p_segflags, pheader->p_align);

    if (pheader->p_type == PT_LOAD) {
        void *ptr = virt_alloc(pheader->p_vaddr, pheader->p_memsz);
        MEMCPY(ptr, elf_start + pheader->p_offset, pheader->p_filesz);
    }

    return 0;
}


#ifdef DEBUG
static char *fetch_string(const char *elf_start, uint32_t string_offset)
{
    elf_header_t *header = (elf_header_t *) elf_start;
    elf_section_header_t *sheaders = 
        (elf_section_header_t *) (elf_start + header->section_offset);

    elf_section_header_t *string_header = &sheaders[ header->section_header_names];
    uint32_t strings_in_elfimage = string_header->sh_offset;
    const char *string_ptr = (elf_start + strings_in_elfimage + string_offset);
    char *ptr = malloc(strlen(string_ptr)+1);
    strncpy(ptr, string_ptr, strlen(string_ptr));
    return ptr;
}
#endif


/** 
 * read_elf - reads elf data into memory
 * data - a pointer to the elf file's data image
 * alloc - a function to allocate memory, given a virtual address and size
 * */
int read_elf(const char *elf_start, void *(*virt_alloc)(uint32_t virt_start, uint32_t length))
{
    elf_header_t *header = (elf_header_t *) elf_start;

    int rc = validate_header(header);
    if (rc < 0) return rc;


    // load headers
    elf_program_header_t *pheaders = 
            (elf_program_header_t *) (elf_start + header->header_offset);
    
    int i;
    for (i = 0; i < header->program_header_num; i++) {
        handle_program_header(elf_start, &pheaders[i], virt_alloc);
    }

    return 0;
}