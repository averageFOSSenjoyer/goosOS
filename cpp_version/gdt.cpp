#include "gdt.h"

uint32_t __stack_chk_fail_local(){
    return 0;
}

global_descriptor_table::global_descriptor_table():
    null_segment_selector(0,0,0), 
    unused_segment_selector(0,0,0),
    code_segment_selector(0,64*1024*1024,0x9A),
    data_segement_selector(0,64*1024*1024,0x92) {
        uint32_t i[2];
        i[1] = (uint32_t) this;
        i[0] = sizeof(global_descriptor_table) << 16;
        
        __asm__ volatile("lgdt (%0)": :"p" (((uint8_t*) i ) + 2)); 
    }

global_descriptor_table::~global_descriptor_table() {}

uint16_t global_descriptor_table::_data_segement_selector() {
    return (uint8_t*) &data_segement_selector - (uint8_t*) this;
}

uint16_t global_descriptor_table::_code_segment_selector() {
    return (uint8_t*) &code_segment_selector - (uint8_t*) this;
}

global_descriptor_table::segment_descriptor::segment_descriptor(uint32_t base, uint32_t limit, uint8_t access) {
    uint8_t* target = (uint8_t*) this;
    if (limit <= 65536) {
        target[6] = 0x40;
    }
    else {
        if ((limit & 0xFFF) != 0xFFF) {
            limit = (limit >> 12) - 1;
        }
        else {
            limit = limit >> 12;
        }
        target[6] = 0xC0;
    }
    target[0] = limit & 0xFF;
    target[1] = (limit >> 8) & 0xFF;
    target[6] = target[6] | ((limit >> 16) & 0xF);

    target[2] = base & 0xFF;
    target[3] = (base >> 8) & 0xFF;
    target[4] = (base >> 16) & 0xFF;
    target[7] = (base >> 24) & 0xFF;

    target[5] = access;
}

uint32_t global_descriptor_table::segment_descriptor::base() {
    uint8_t* target = (uint8_t*) this;
    uint32_t result = target[7];
    result = (result << 8) + target[4];
    result = (result << 8) + target[3];
    result = (result << 8) + target[2];
    return result;
}

uint32_t global_descriptor_table::segment_descriptor::limit() {
    uint8_t* target = (uint8_t*) this;
    uint32_t result = target[6] & 0xF;
    result = (result << 8) + target[1];
    result = (result << 8) + target[0];

    if ((target[6] & 0xC0) == 0xC0) {
        result = (result << 12) | 0xFFF;
    }

    return result;
}