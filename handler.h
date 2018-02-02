#ifndef _HANDLER
#define _HANDLER

#define IRQ_OFFSET (32)

typedef enum {
  TASK32=5, INTR16=6, TRAP16=7, INTR32=14, TRAP32=32
} idt_gate_t;

typedef struct {
  uint16_t offset_lo;
  uint16_t selector;
  uint8_t zero;
  uint8_t type;
  uint16_t offset_hi;
} __attribute__((packed)) idt_entry_t;

typedef struct {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) idt_desc_t;

void register_handler(uint16_t intnum,  void (*handler)(int));
void init_handler( );

void _excp_0( );
void _excp_1( );
void _excp_2( );
void _excp_3( );
void _excp_4( );
void _excp_5( );
void _excp_6( );
void _excp_7( );
void _err_8( );
void _err_10( );
void _err_11( );
void _err_12( );
void _err_13( );
void _err_14( );
void _excp_16( );
void _err_17( );
void _excp_18( );
void _excp_19( );
void _intr_32( );
void _intr_33( );
void _intr_34( );
void _intr_35( );
void _intr_36( );
void _intr_37( );
void _intr_38( );
void _intr_39( );
void _intr_40( );
void _intr_41( );
void _intr_42( );
void _intr_43( );
void _intr_44( );
void _intr_45( );
void _intr_46( );
void _intr_47( );
void _intr_48( );

void excp_handler(int excp_num);
void err_handler(int err_num);
void int_handler(int int_num);

#endif
