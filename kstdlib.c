#include <stdarg.h>
#include "console.h"
#include "kstdlib.h"
#include "chrdev.h"

chrdev_t *stdout;
chrdev_t *stdin;

void kmemset(void *ptr, int c, size_t n)
{
  size_t i;
  uint8_t *dst = (uint8_t *) ptr;

  for (i = 0; i < n; i++) {
    dst[i] = (uint8_t) (c & 0xff);
  }
}


void *kmemcpy(void *dest, const void *src, size_t n)
{
  size_t i;
  uint8_t *d = (uint8_t *) dest;
  uint8_t *s = (uint8_t *) src;

  for (i = 0; i < n; i++) {
    d[i] = s[i];
  }

  return dest;
}


void kstrncpy(char *dest, const char *src, size_t n)
{
  size_t i = 0;

  while ((i < n) && (src[i] != 0)) {
    dest[i] = src[i];
    i++;
  }

  dest[i] = 0x00;
}

int kstrcmp(const char *a, const char *b)
{
  int i = 0;

  while ((a[i] != 0) && (b[i] != 0) && (a[i] == b[i]))
    i++;

  return a[i] - b[i];
}

int kstrncmp(const char *a, const char *b, int n)
{
  int i = 0;

  while ((a[i] != 0) && (b[i] != 0) && (a[i] == b[i]) && (i < n))
    i++;

  return a[i] - b[i];
}


inline int isdigit(char ch)
{
  return ((ch >= '0') && (ch <= '9'));
}

inline int islower(char ch)
{
  return ((ch >= 'a') && (ch <= 'z'));
}

inline int isupper(char ch)
{
  return ((ch >= 'A') && (ch <= 'Z'));
}

inline int toupper(char ch)
{
  if (islower(ch)) return ch - 'a' + 'A';
  else return ch;
}

inline int tolower(char ch)
{
  if (isupper(ch)) return ch - 'A' + 'a';
  else return ch;
}


long int kstrtol(const char *nptr, char **endptr, int base)
{
  long int value = 0;
  int sign = 1;
  int i = 0;
  int v,ch;
  while(1) {
    ch = nptr[i];

    if ((i == 0) && (nptr[i] == '-')) {
      sign = -1;
      continue;
    }
    
    if (isdigit(nptr[i])) {
      v = nptr[i] - '0';
      if (v < base)
	value = (value * base) + v;
      else
	break;
    }
      
    else {
      if (islower(ch))
	ch = toupper(ch);
      v = ch - 'A';
      if (v < (base-10))
	value = (value * base) + (base + v);
      else
	break;
    }
  }

  if (endptr != NULL)
    *endptr = (char *) &nptr[i];

  return sign * value;
}
      
int katoi(const char *nptr)
{
  return kstrtol(nptr, NULL, 10);
}


static int kput_str(const char *str)
{
  int count = 0;
  while (str[count] != 0x00) {
    chrdev_putch(stdout,str[count]);
    count++;
  }

  return count;
}

static int kput_int(int value, int base)
{
  int32_t max_value = 1;
  int32_t next_value;
  int32_t started = 0;
  int count = 0;

  // short_circuit
  if ((value >= 0) && (value < base) && (value < 10)) {
    chrdev_putch(stdout, '0' + value);
    return 1;
  }
  if ((value >= 0) && (value < base) && (value >= 10)) {
    chrdev_putch(stdout, 'A' + (value-10));
    return 1;
  }
  
  // find the overflow!
  do {
    max_value = max_value * base;
    next_value = max_value * base;
  } while ((next_value / base) == max_value);


  // max_value now last value before overflow in this base

  // handle leading negatives
  if (value < 0) {
    chrdev_putch(stdout, '-');
    count++;
  }

  // divisor will start at greatest value
  int32_t divisor = max_value;

  // keep going while there is something to divide
  while (divisor > 0) {

    // get the digit
    uint8_t digit = (value / divisor);

    // skip leading zeros
    if ((started || digit > 0)) {

      // handle 0-9
      if (digit < 10) {
	chrdev_putch(stdout, '0' + digit);
      }
      // or base characters 
      else {
	chrdev_putch(stdout, 'A' + (digit-10));
      }
      started = 1;
      count++;
    }
    
    value = value % divisor;
    divisor = divisor / base;
  }

  return count;
}

static int kput_int64(int value, int base)
{
  int64_t max_value = 1;
  int64_t next_value;
  int64_t started = 0;
  int count = 0;

  // short_circuit
  if ((value >= 0) && (value < base) && (value < 10)) {
    chrdev_putch(stdout, '0' + value);
    return 1;
  }
  if ((value >= 0) && (value < base) && (value >= 10)) {
    chrdev_putch(stdout, 'A' + (value-10));
    return 1;
  }
  
  // find the overflow!
  do {
    max_value = max_value * base;
    next_value = max_value * base;
  } while ((next_value / base) == max_value);


  // max_value now last value before overflow in this base

  // handle leading negatives
  if (value < 0) {
    chrdev_putch(stdout, '-');
    count++;
  }

  // divisor will start at greatest value
  int64_t divisor = max_value;

  // keep going while there is something to divide
  while (divisor > 0) {

    // get the digit
    uint8_t digit = (value / divisor);

    // skip leading zeros
    if ((started || digit > 0)) {

      // handle 0-9
      if (digit < 10) {
	chrdev_putch(stdout, '0' + digit);
      }
      // or base characters 
      else {
	chrdev_putch(stdout, 'A' + (digit-10));
      }
      started = 1;
      count++;
    }
    
    value = value % divisor;
    divisor = divisor / base;
  }

  return count;
}

static int kput_uint(int value, int base)
{
  uint32_t max_value = 1;
  uint32_t next_value;
  uint32_t started = 0;
  int count = 0;

  // short_circuit
  if ((value < base) && (value < 10)) {
    chrdev_putch(stdout, '0' + value);
    return 1;
  }
  if ((value < base) && (value >= 10)) {
    chrdev_putch(stdout, 'A' + (value-10));
    return 1;
  }
  
  // find the overflow!
  do {
    max_value = max_value * base;
    next_value = max_value * base;
  } while ((next_value / base) == max_value);


  // divisor will start at greatest value
  uint32_t divisor = max_value;

  // keep going while there is something to divide
  while (divisor > 0) {

    // get the digit
    uint8_t digit = (value / divisor);

    // skip leading zeros
    if ((started || digit > 0)) {

      // handle 0-9
      if (digit < 10) {
	chrdev_putch(stdout, '0' + digit);
      }
      // or base characters 
      else {
	chrdev_putch(stdout, 'A' + (digit-10));
      }
      started = 1;
      count++;
    }
    
    value = value % divisor;
    divisor = divisor / base;
  }

  return count;
}


static int kput_uint64(uint64_t value, int base)
{
  uint64_t max_value = 1;
  uint64_t next_value;
  uint64_t started = 0;
  int count = 0;

  // short_circuit
  if ((value < base) && (value < 10)) {
    chrdev_putch(stdout, '0' + value);
    return 1;
  }
  if ((value < base) && (value >= 10)) {
    chrdev_putch(stdout, 'A' + (value-10));
    return 1;
  }
  
  // find the overflow!
  do {
    max_value = max_value * base;
    next_value = max_value * base;
  } while ((next_value / base) == max_value);


  // divisor will start at greatest value
  int32_t divisor = max_value;

  // keep going while there is something to divide
  while (divisor > 0) {

    // get the digit
    uint8_t digit = (value / divisor);

    // skip leading zeros
    if ((started || digit > 0)) {

      // handle 0-9
      if (digit < 10) {
	chrdev_putch(stdout, '0' + digit);
      }
      // or base characters 
      else {
	chrdev_putch(stdout, 'A' + (digit-10));
      }
      started = 1;
      count++;
    }
    
    value = value % divisor;
    divisor = divisor / base;
  }

  return count;
}



int kprintf(const char *format, ...) {

  va_list args;
  va_start(args, format);
  
  int state = 0;
  int count = 0;
  int longval = 0;

  while (*format != NULL) {

    switch(state) {
    case 0:
      if (*format == '%') {
        longval = *(format+1) == 'L' ? 1 : 0;
	      state = 1;
      }
      else if (*format == '\\')
	state = 2;
      else {
	chrdev_putch(stdout, *format);
	count++;
      }
      break;

    case 1:
      if (*format == 'd') {
        if (longval) {
          int64_t v = va_arg(args, int64_t);
	        count+=kput_int64(v, 10);
	        state = 0;
        }
        else {
          int v = va_arg(args, int);
	        count+=kput_int(v, 10);
	        state = 0;
          }
      }
      else if (*format == 'x') {
        if (longval) {
          int64_t v = va_arg(args, int64_t);
	        count+=kput_uint64(v, 16);
	        state = 0;
        }
        else {
          int v = va_arg(args, int);
	        count+=kput_uint(v, 16);
	        state = 0;
          }
      }
      else if (*format == 'p') {
          int v = va_arg(args, void *);
	        count+=kput_uint(v, 16);
	        state = 0;
      }
      else if (*format == 's') {
	const char *ptr = va_arg(args, const char *);
	count+=kput_str(ptr);
	state = 0;
      }
      else if (*format == '%') {
	chrdev_putch(stdout, '%');
	state = 0;
	count++;
      }
      break;

    case 2:
      if (*format == 'n') chrdev_putch(stdout, '\n');
      else if (*format == 'r') chrdev_putch(stdout,'\r');
      else if (*format == 'a') chrdev_putch(stdout, '\a');
      else if (*format == 'b') chrdev_putch(stdout,'\b');
      else if (*format == 'v') chrdev_putch(stdout,'\v');
      else if (*format == 'f') chrdev_putch(stdout,'\f');
      else break;
      
      state = 0;
      count++;
      break;
    }

    ++format;
  }

  return count;
}


char getchar( )
{
  return chrdev_getch(stdin);
}

char *gets(char *s)
{
  int i = 0;
  while(1) {
    char ch = chrdev_getch(stdin);

    if (ch == '\n') break;
    else if (ch == '\r') break;
    else if (ch == '\b') i--;
    else  s[i++] = ch;
    s[i] = 0x00;
  }
    return s;
}

void putc(char ch)
{
  chrdev_putch(stdout, ch);
}

inline void kdisable_interrupts( )
{
  cli( );

}

inline void kenable_interrupts( )
{
  sti( );
}


