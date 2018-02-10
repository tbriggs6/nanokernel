#include "kstdlib.h"
#include "fifo.h"
#include "handler.h"
#include "i386.h"
#include "chrdev.h"


#define KEYB_DATA (0x60)
#define KEYB_STATUS (0x64)
#define KEYB_CMD (0x64)

#define KEYB_CMD_RDRAM (0x20)
#define KEYB_CMD_WRRAM (0x60)
#define KEYB_CMD_PORTTEST2 (0xA8)
#define KEYB_CMD_SELFTEST (0xAA)
#define KEYB_CMD_PORTTEST1 (0xAB)
#define KEYB_CMD_DIS_PORT1 (0xAD)
#define KEYB_CMD_ENA_PORT1 (0xAE)
#define KEYB_CMD_DIS_PORT2 (0xA7)
#define KEYB_CMD_ENA_PORT2 (0xA8)



#define KEYB_SELFTEST_OK (0x55)

typedef struct {
  int num_ports;
  int port1_present;
  int port2_present;
} kbd_driver_config_t;

static kbd_driver_config_t kbd_config = { 0,0,0 };

typedef struct  __attribute__((packed)) {
  union {
    uint8_t status_byte;
    struct {
      // output buffer (0==empty, 1== full)
      unsigned int outbuff_stat:1;
      
      // input buffer (0 == empty (clear), 1 == full)
      unsigned int inpbuff_stat:1;
      
      // system passed POST
      unsigned int sysflag:1;
      
      // 0 = data written to DATA port is data, 1 = command
      unsigned int cmd_data:1;
      
      unsigned int :2;
      
      // timeout error detected (0 == no error, 1 == error)
      unsigned int timeout_err:1;
      
      // parity error detected (0 == no error, 1 == error)
      unsigned int parity_err:1;
    };
  };
} keyb_status_t;

typedef struct  __attribute__((packed)) {
  union  {
    uint8_t config_byte;
    struct {
      unsigned int port1_intr_en:1;   // bit 0
      unsigned int port2_intr_en:1;   //     1
      unsigned int sysflag:1;         //     2
      unsigned int :1;                //     3
      unsigned int port1_clk_en:1;    //     4
      unsigned int port2_clk_en:1;    //     5
      unsigned int port1_xlat_en:1;   //     6
      unsigned int :1;                //     7
    };
  };
} keyb_config_t; 


typedef struct  __attribute__((packed)) {
  union {
    uint8_t output_byte;
    struct {
      unsigned int system_reset:1;
      unsigned int gate_a20:1;
      unsigned int port2_clk:1;
      unsigned int port2_data:1;
      unsigned int port1_full:1;
      unsigned int port2_full:1;
      unsigned int port1_clk:1;
      unsigned int port1_data:1;
    };
  };
} keyb_ctl_output_t;

      
      
      


typedef enum {
  PORT1, PORT2
} keyb_port_t;


static fifo_t *keyboard_fifo = NULL;

/* Is the controller ready to send data TO PC */
static int keyb_outp_ready( )
{
  keyb_status_t status;

  status.status_byte = inb(KEYB_STATUS);
  return (status.outbuff_stat == 1);
}

/* Is the controller ready to read data FROM PC */
static int keyb_inp_ready( )
{
   keyb_status_t status;
  status.status_byte = inb(KEYB_STATUS);

  return (status.inpbuff_stat == 0);
}

/* Send command to the PC */
static void keyb_send_cmd(uint8_t cmd_byte)
{
  while (! keyb_inp_ready()) ;
  
  outb(KEYB_CMD, cmd_byte);
}

/* Read data from the PC */
static uint8_t keyb_read_data( )
{
  while (! keyb_outp_ready());
  
  return inb(KEYB_DATA);
}

/* Write data to the controller */
static void keyb_send_data(uint8_t data)
{
  while (! keyb_inp_ready());

  outb(KEYB_DATA, data);
}


/* Read the configuration byte */
static keyb_config_t keyb_read_cfgbyte( )
{
  // wait for buff to clear
  keyb_send_cmd(KEYB_CMD_RDRAM);

  keyb_config_t config;
  config.config_byte = keyb_read_data( );

  return config;
}
    

/* Write the configuration byte */
static void keyb_write_cfgbyte(keyb_config_t config)
{
  keyb_send_cmd(KEYB_CMD_WRRAM);

  keyb_send_data(config.config_byte);
}

/* Enable the keyboard port */
static void keyb_enable_port(keyb_port_t port)
{
  switch(port) {
  case PORT1:
    keyb_send_cmd(KEYB_CMD_ENA_PORT1);
    break;
  case PORT2:
    keyb_send_cmd(KEYB_CMD_ENA_PORT2);
    break;
  }
}

/* Disable the keyboard port */
static void keyb_disable_port(keyb_port_t port)
{
  switch(port) {
  case PORT1:
    keyb_send_cmd(KEYB_CMD_DIS_PORT1);
    break;
  case PORT2:
    keyb_send_cmd(KEYB_CMD_DIS_PORT2);
    break;
  }
}

/* Test the controller */
static int keyb_selftest( )
{
  keyb_send_cmd(KEYB_CMD_SELFTEST);

  // return 0x55 on success
  uint8_t result = keyb_read_data( );
  return result;
}

/* Test the port */
static int keyb_test_port(keyb_port_t port)
{
  switch(port) {
  case PORT1:   keyb_send_cmd(KEYB_CMD_PORTTEST1); break;
  case PORT2:   keyb_send_cmd(KEYB_CMD_PORTTEST2); break;
  }

  uint8_t result = keyb_read_data( );
  return result;
}


void keyboard_handler(int irqnum )
{
  if (irqnum != IRQ_OFFSET+1) {
    kprintf("Keyboard error - handler called on %d\n", irqnum);
    return;
  }
  
  while(keyb_outp_ready()) {
    uint8_t byte = keyb_read_data( );

    if (! fifo_full(keyboard_fifo))
      fifo_add(keyboard_fifo, &byte);
    else {
      kprintf("keyboard dropped char %x\n", byte);
    }
  }
}


static const char *keyb_test_errs[] = {
  "OK test passed",
  "Err - clock line stuck low",
  "Err - clock line stuck high",
  "Err - data line stuck low",
  "Err - data line stuck high"
};

static uint8_t keyboard_drv_getch( )
{
  uint8_t ch = 0x00;

  if (fifo_avail(keyboard_fifo)>0)
    fifo_get(keyboard_fifo, &ch);

  return ch;
}

// fail
static int keyboard_drv_putch(uint8_t ch)
{
  return 0;
}

static int keyboard_drv_isempty( )
{
  return fifo_empty(keyboard_fifo);
}

static int keyboard_drv_isfull( )
{
  return fifo_full(keyboard_fifo);
}

static void keyboard_make_chrdev( )
{
  chrdev_t *dev = kmalloc(sizeof(chrdev_t));

  kstrncpy(dev->driver_name,"PS2 Keyboard", 31);
  dev->getch = keyboard_drv_getch;
  dev->putch = keyboard_drv_putch;
  dev->isempty = keyboard_drv_isempty;
  dev->isfull = keyboard_drv_isfull;

  chrdev_register(dev);

  stdin = dev;
}
  
void keyboard_init( )
{
  // construct and initialize the keyboard fifo
  keyboard_fifo = (fifo_t *) kmalloc(fifo_sizeof());
  fifo_init(keyboard_fifo, 32, 1);

  // disable the ports
  keyb_disable_port(PORT1);
  keyb_disable_port(PORT2);

  // flush the buffer
  while (keyb_outp_ready()) {
    uint8_t byte __attribute__((unused));
    byte = keyb_read_data( );
  }

  // disable interrupts and translation
  // ---- and detect port2
  keyb_config_t cfg = keyb_read_cfgbyte( );
  if (cfg.port2_clk_en == 0) {
    kbd_config.num_ports = 1;
    kbd_config.port1_present = 1;
    kbd_config.port2_present = 0;
  }
  else {
    kbd_config.num_ports = 2;
    kbd_config.port1_present = 1;
    kbd_config.port2_present = 1;
  }

  cfg.port1_intr_en = 0;
  cfg.port2_intr_en = 0;
  cfg.port1_xlat_en = 0;

  keyb_write_cfgbyte( cfg );

  // pertform self-test
  uint8_t byte = keyb_selftest( );
  if (byte != KEYB_SELFTEST_OK) {
    kprintf("Error - PS/2 controller did not pass self-test\n");
    return;
  }

  // perform interface tests
  byte = keyb_test_port(PORT1);
  if (byte != 0x00) {
    kprintf("Error - PORT1 did not pass self-test: %s\n",keyb_test_errs[byte]);
    kbd_config.port1_present = 0;
  }

  if (kbd_config.port2_present) {
    byte = keyb_test_port(PORT2);
    if (byte != 0x00) {
      kprintf("Error - PORT2 did not pass self-test: %s\n", keyb_test_errs[byte]);
      kbd_config.port2_present = 0;
    }
  }
  
  kbd_config.num_ports = kbd_config.port1_present + kbd_config.port2_present;
  if (kbd_config.num_ports == 0) {
    kprintf("Error - no PS2 devices left, giving up\n");
    return;
  }

  cfg = keyb_read_cfgbyte( );
  if (kbd_config.port1_present)  {
    cfg.port1_intr_en = 1;
    cfg.port1_xlat_en = 1;
  }

  if (kbd_config.port2_present) {
    cfg.port2_intr_en = 1;
  }
  
  keyb_write_cfgbyte( cfg ); 


  if (kbd_config.port1_present) 
    keyb_enable_port(PORT1);

  if (kbd_config.port2_present)
    keyb_enable_port(PORT2);
  
  register_handler(IRQ_OFFSET+1, keyboard_handler);


  // wrap the character driver to the keyboard
  keyboard_make_chrdev( );
}

  

