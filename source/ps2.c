#include "kstdlib.h"
#include "handler.h"
#include "i386.h"
#include "pic.h"
#include "fifo.h"
#include "keyboard.h"

static keyboard_handler_t *keyboard_handler = NULL;

static void delay_8042( )
{
    int j;
    for (j = 0; j < 100000; j++) 
        nop();
}
// abstarct the 8042 i/o port addresses
static void write_8042_command(uint8_t cmd_value)
{
    outb(0x64, cmd_value);
}

static uint8_t read_8042_status(void)
{
    return inb(0x64);
}

static void write_8042_data(uint8_t data_value)
{
    outb(0x60, data_value);
}

static uint8_t read_8042_data(void)
{
    return inb(0x60);
}

static void ps2_write_reg(uint8_t command_byte)
{
    // wait for command port to become available
    while ((read_8042_status() & (1 << 1)) == 1) ;
    delay_8042( );
    write_8042_command(command_byte);
}

static void ps2_write_reg2(uint8_t command_byte1, uint8_t command_byte_2)
{
    while ((read_8042_status() & (1 << 1)) == 1) ;
    write_8042_command(command_byte1);
    delay_8042( );

    while ((read_8042_status() & (1 << 1)) == 1) ;
    write_8042_data(command_byte_2);
}

static uint8_t ps2_read_reg(uint8_t reg)
{
    while ((read_8042_status() & (1 << 1)) == 1) ;
    write_8042_command(reg);
    delay_8042( );

    // wait for data register to become full
    while((read_8042_status() & (1 << 0)) == 0) ;
    uint8_t byte = read_8042_data( );

    return byte;
}

static uint8_t ps2_read_data( )
{
    while((read_8042_status() & (1 << 0)) == 0) ;
    uint8_t byte = read_8042_data( );
    return byte;
}

static void ps2_disable_ports( )
{
    ps2_write_reg(0xad);
    

    ps2_write_reg(0xa7);
}

void ps2_handler(int intr_num)
{
    uint8_t scan_code = read_8042_data( );
    keyboard_handler->keyboard_send_char(keyboard_handler, scan_code);
}

static void ps2_flush()
{
    while (read_8042_status() & (1 << 0))
        read_8042_data();
}

static void ps2_test_selftest( )
{

    ps2_write_reg(0xaa);
    delay_8042( );

    uint8_t selftest = ps2_read_data( );
    delay_8042( );

    if (selftest != 0x55) {
        kprintf("Self test failed.\n");
    }
    else
    {
        kprintf("Self test OK\n");
    }
    
}

void ps2_init(keyboard_handler_t *handler)
{
    keyboard_handler = handler;

    kdisable_interrupts( );
    pic_disable_interrupt(IRQ1);

    ps2_disable_ports( );
    ps2_flush( );

    uint8_t cfg = ps2_read_reg(0x20);
    cfg = cfg & ~(1 << 0 | 1 << 1 | 1 << 6);
    ps2_write_reg2(0x60, cfg);
    ps2_test_selftest();

    ps2_write_reg(0xae);

    cfg = ps2_read_reg(0x20);
    cfg = cfg | (1 << 0 | 1 << 1);
    ps2_write_reg2(0x60, cfg);

   // register interrupt handlers
    register_handler(33,  ps2_handler);
    pic_enable_interrupt(IRQ1);

    kenable_interrupts( );
}