#include <iostream>

#include <cpu.h>

cpu::cpu() {
    registers.a = 0x0;
    registers.b = 0x0;
    registers.c = 0x0;
    registers.d = 0x0;
    registers.e = 0x0;
    registers.f = 0x0;
    registers.h = 0x0;
    registers.l = 0x0;
}

short cpu::get_af() {
    unsigned short sh_af = registers.a << 8;
    sh_af |= registers.f;
    return sh_af;
}

void cpu::set_af(unsigned short sh_af) {
    registers.a = static_cast<unsigned char>((sh_af & 0xff00) >> 8);
    registers.f = static_cast<unsigned char>(sh_af & 0x00ff);
}

short cpu::get_bc() {
    unsigned short sh_bc = registers.b << 8;
    sh_bc |= registers.c;
    return sh_bc;
}

void cpu::set_bc(unsigned short sh_bc) {
    registers.b = static_cast<unsigned char>((sh_bc & 0xff00) >> 8);
    registers.c = static_cast<unsigned char>(sh_bc & 0x00ff);
}

short cpu::get_de() {
    unsigned short sh_de = registers.d << 8;
    sh_de |= registers.e;
    return sh_de;
}

void cpu::set_de(unsigned short sh_de) {
    registers.d = static_cast<unsigned char>((sh_de & 0xff00) >> 8);
    registers.e = static_cast<unsigned char>(sh_de & 0x00ff);
}

short cpu::get_hl() {
    unsigned short sh_hl = registers.h << 8;
    sh_hl |= registers.l;
    return sh_hl;
}

void cpu::set_hl(unsigned short sh_hl) {
    registers.h = static_cast<unsigned char>((sh_hl & 0xff00) >> 8);
    registers.l = static_cast<unsigned char>(sh_hl & 0x00ff);
}