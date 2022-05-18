#include "interrupt.h"

void prints(char* str);

interrupt_manager::gate_descriptor interrupt_manager::interrupt_descriptor_table[256];

void interrupt_manager::set_interrupt_descriptor_table_entry(
    uint8_t interrupt_number, 
    uint16_t code_segment_selector_offset,
    void (*handler)(),
    uint8_t descriptor_privilege_level,
    uint8_t descriptor_type
    ) {
    const uint8_t IDT_DESC_PRESENT = 0x80;
    interrupt_descriptor_table[interrupt_number].handler_address_low = ((uint32_t) handler) & 0xFFFF;
    interrupt_descriptor_table[interrupt_number].handler_address_high = (((uint32_t) handler) >> 16) & 0xFFFF;
    interrupt_descriptor_table[interrupt_number].gdt_code_segment_selector = code_segment_selector_offset;
    interrupt_descriptor_table[interrupt_number].access = IDT_DESC_PRESENT | descriptor_type | ((descriptor_privilege_level & 3) << 5);
    interrupt_descriptor_table[interrupt_number].reserved = 0;
}

interrupt_manager::interrupt_manager(global_descriptor_table* gdt) :
    pic_master_command(0x20),
    pic_master_data(0x21),
    pic_slave_command(0xA0),
    pic_slave_data(0xA1)
{
    uint16_t code_segment = gdt->_code_segment_selector();
    const uint8_t IDT_INTERRUPT_GATE = 0xE;
    for (uint16_t i = 0; i < 256; i++) {
        set_interrupt_descriptor_table_entry(i, code_segment, &ignore_interrupt_request, 0, IDT_INTERRUPT_GATE);
    }
    set_interrupt_descriptor_table_entry(0x20, code_segment, &handle_interrupt_request0x00, 0, IDT_INTERRUPT_GATE);
    set_interrupt_descriptor_table_entry(0x21, code_segment, &handle_interrupt_request0x01, 0, IDT_INTERRUPT_GATE);

    pic_master_command.write(0x11);
    pic_slave_command.write(0x11);

    pic_master_data.write(0x20);
    pic_slave_data.write(0x28);

    pic_master_data.write(0x04);
    pic_slave_data.write(0x02);

    pic_master_data.write(0x01);
    pic_slave_data.write(0x01);

    pic_master_data.write(0x00);
    pic_slave_data.write(0x00);

    interrupt_descriptor_table_pointer idt;
    idt.size = 256 * sizeof(gate_descriptor) - 1;
    idt.base = (uint32_t) interrupt_descriptor_table;
    __asm__ volatile(
        "lidt %0"
        :

        :
        "m" (idt)
    );
}

interrupt_manager::~interrupt_manager() {}

void interrupt_manager::activate() {
    __asm__ (
        "sti"
    );
}

uint32_t interrupt_manager::handle_interrupt(uint8_t interrupt_number, uint32_t esp) {
    prints("interrupt");
    return esp;
}