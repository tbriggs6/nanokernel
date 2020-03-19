#ifndef _KSTDLIB_H
#define _KSTDLIB_H

#include <stdint.h>
#include "i386.h"
#include "chrdev.h"

#ifndef NULL
#define NULL (0)
#endif

typedef uint32_t size_t;


extern chrdev_t *stdout;
extern chrdev_t *stdin;

void kmemset(void *ptr, int c, size_t);
void *kmemcpy(void *dest, const void *src, size_t n);
void kstrncpy(char *dest, const char *src, size_t n);
int kstrcmp(const char *a, const char *b);
int kstrncmp(const char *a, const char *b, int n);

void *kmalloc(size_t size);
void kfree(void *ptr);

int isdigit(char ch);
int islower(char ch);
int isupper(char ch);
int toupper(char ch);
int tolower(char ch);
long int kstrtol(const char *nptr, char **endptr, int base);
int katoi(const char *nptr);

int kprintf(const char *format, ...) __attribute__((format(printf, 1,2)));
char getchar( );
char *gets(char *s);
void putc(char ch);

void kdisable_interrupts( );
void kenable_interrupts( );

void panic( );
#endif
