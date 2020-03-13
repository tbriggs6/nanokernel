#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <stdint.h>

#include "elf.h"

size_t getsize(int fid)
{
    struct stat statbuf;

    int rc = fstat(fid, &statbuf);
    assert(rc >= 0);

    return statbuf.st_size;    
}

void *mapfile(int fid)
{
    size_t size = getsize(fid);
    void *ptr = mmap(NULL, size, PROT_READ, 
        MAP_PRIVATE, fid, 0);

    assert(ptr != NULL);
    return ptr;
}      

void *fake_alloc(uint32_t virtual, uint32_t size)
{
    void *ptr = malloc(size);
    printf("ALLOC: %u bytes %x = %p\n", 
        (unsigned int) size, (unsigned int) virtual, ptr);
    return ptr;
}

// lets pick an elf file
int main(int argc, char **argv)
{

    int fid = open("simple", O_RDONLY);
    assert(fid > 0);

    void *ptr = mapfile(fid);
    char *cptr = (char *) ptr;
    printf("ELF file mapped into memory: %p\n", ptr);

    int rc = read_elf(cptr, fake_alloc);
    printf("read_elf reported %d\n", rc);
}
