#include <bitset>
#include <fstream>
#include <iostream>

#include <cpu.h>

using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ios_base;

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

bool cpu::load_rom(const char* rom) {
	ifstream in(rom, ios_base::binary | ios_base::ate);

	if (in.is_open()) {
		std::streampos size = in.tellg();
		char* buffer = new char[size];

		in.seekg(0, ios::beg);
		in.read(buffer, size);
		in.close();

		for (int i = 0; i < size; ++i) {
			mram[i] = buffer[i];
		}
        return true;
	} else {
        cout << "Error: problem loading rom at " << rom << endl;
        return false;
    }

    return false;
}

void cpu::read() {
    unsigned int opcode = static_cast<unsigned int>(mram[prog_counter]);

    switch (opcode) {
        // NOP: Advances the program counter by 1.
        case 0x00: {
            prog_counter++;
            break;
        }

        // STOP: Stops the system clock and osc. circuit. if pc is 0x1000.
        case 0x10: {
            if (static_cast<unsigned int>(mram[prog_counter + 1]) == 0x00) {
                running = false;
            }
            break;
        }

        // JR NZ, s8: If the Z flag is 0, then read the next signed 8 bytes. Else, move forward.
        case 0x20: {
            char s8 = static_cast<char>(mram[prog_counter + 1]);
            prog_counter += (!f_flags.f_zero ? s8 : 1);

            break;
        }

        // JR NC, s8: If the CY flag is 0, then read the next signed 8 bytes. Else, move forward.
        case 0x30: {
            char s8 = static_cast<char>(mram[prog_counter + 1]);
            prog_counter += (!f_flags.f_carry ? s8 : 1);

            break;
        }

        // LD B, B: Loads the contents of register B into register B.
        case 0x40: {
            registers.b = registers.b;
            prog_counter++;

            break;
        }

        // LD D, B: Loads the contents of register B into register D.
        case 0x50: {
            registers.d = registers.b;
            prog_counter++;

            break;
        }

        // LD H, B: Loads the contents of register B into register H.
        case 0x60: {
            registers.h = registers.b;
            prog_counter++;

            break;
        }

        // LD (HL), B: Store the contents of register B in the memory location specified by register pair HL.
        case 0x70: {
            mram[get_hl()] = registers.b;
            prog_counter++;

            break;
        }

        // ADD A, B: Add the contents of register B to the contents of register A, and store the results in register A.
        case 0x80: {
            unsigned char sum = registers.a + registers.b;

            f_flags.f_zero = sum == 0x0;
            f_flags.f_subtract = false;
            f_flags.f_carry = registers.a > sum;
            f_flags.f_half_carry = (registers.a & 0xF) + (sum & 0xF) > 0xF;
            build_f();

            registers.a = sum;
            prog_counter++;

            break;
        }

        // SUB B: Subtract the contents of register B from the contents of register A, and store the results in register A.
        case 0x90: { 
            unsigned char diff = registers.a - registers.b;

            f_flags.f_zero = diff == 0x0;
            f_flags.f_subtract = true;
            f_flags.f_carry = registers.a < diff;
            f_flags.f_half_carry = (registers.a & 0xF) < (registers.b & 0xF);
            build_f();

            registers.a = diff;
            prog_counter++;

            break;
        }

        // AND B: Take the logical AND for each bit of the contents of register B and the contents of register A, 
        // and store the results in register A.
        case 0xA0: {
            unsigned char and_res = registers.a & registers.b;
            registers.a = and_res;

            f_flags.f_zero = (and_res == 0x0);
            f_flags.f_subtract = false;
            f_flags.f_half_carry = true;
            f_flags.f_carry = false;
            build_f();

            prog_counter++;

            break;
        }

        // OR B: Take the logical OR for each bit of the contents of register B and the contents of register A, 
        // and store the results in register A.
        case 0xB0: {
            unsigned char op_res = (registers.a | registers.b);
            registers.a = op_res;
            
            f_flags.f_zero = (op_res == 0x0);
            f_flags.f_subtract = false;
            f_flags.f_half_carry = false;
            f_flags.f_carry = false;
            build_f();

            prog_counter++;

            break;
        }

        // TODO
        case 0xC0: { break; }
        case 0xD0: { break; }

        // LD (a8), A: Store the contents of register A in the internal RAM, port register, or mode register 
        // at the address in the range 0xFF00-0xFFFF specified by the 8-bit immediate operand a8.
        case 0xE0: {
            char a8 = static_cast<unsigned char>(mram[prog_counter + 1]);
            short mem_loc = 0xFF00 & a8;

            mram[mem_loc] = registers.a;
            prog_counter++;

            break;
        }

        
        // LD A, (a8): Load into register A the contents of the internal RAM, port register, or mode 
        // register at the address in the range 0xFF00-0xFFFF specified by the 8-bit immediate operand a8.
        case 0xF0: {
            char a8 = static_cast<unsigned char>(mram[prog_counter + 1]);
            short mem_loc = 0xFF00 & a8;

            registers.a = mram[mem_loc];
            prog_counter++;

            break;
        }

        // LD BC, d16: Load the 2 bytes of immediate data into register pair BC.
        case 0x01: {
            char d8 = static_cast<unsigned char>(mram[prog_counter + 1]);
            char d16 = static_cast<unsigned char>(mram[prog_counter + 2]);

            short new_bc = d8 << 8 & d16;
            set_bc(new_bc);
            prog_counter++;

            break;
        }

        // LD DE; d16: Load the 2 bytes of immediate data into register pair DE.
        case 0x11: {
            unsigned char d8 = static_cast<unsigned char>(mram[prog_counter + 1]);
            unsigned char d16 = static_cast<unsigned char>(mram[prog_counter + 2]);

            unsigned short new_de = d8 << 8 & d16;
            set_de(new_de);
            prog_counter++;
            
            break;
        }

        // LD HL; d16: Load the 2 bytes of immediate data into register pair HL.
        case 0x21: {
            unsigned char d8 = static_cast<unsigned char>(mram[prog_counter + 1]);
            unsigned char d16 = static_cast<unsigned char>(mram[prog_counter + 2]);

            unsigned short new_hl = d8 << 8 & d16;
            set_hl(new_hl);
            prog_counter++;
            
            break;
        }

        // LD SP; d16: Load the 2 bytes of immediate data into register pair SP.
        case 0x31: {
            char d8 = static_cast<unsigned char>(mram[prog_counter + 1]);
            char d16 = static_cast<unsigned char>(mram[prog_counter + 2]);

            unsigned short new_sp = d8 << 8 & d16;
            stack_pointer = new_sp;
            prog_counter++;
            
            break;
        }

        // LD B, C: Loads the contents of register C into register B.
        case 0x41: {
            registers.b = registers.c;
            prog_counter++;

            break;
        }

        // LD D, C: Loads the contents of register C into register D.
        case 0x51: {
            registers.d = registers.c;
            prog_counter++;

            break;
        }

        // LD H, C: Loads the contents of register C into register H.
        case 0x61: {
            registers.h = registers.c;
            prog_counter++;

            break;
        }

        // LD H, C: Store the contents of register C in the memory location 
        // specified by register pair HL.
        case 0x71: {
            mram[get_hl()] = registers.c;
            prog_counter++;

            break;
        }

        // ADD A, C: Add the contents of register C to the contents of register A, and store the results in register A.
        case 0x81: {
            unsigned char sum = registers.a + registers.c;

            f_flags.f_zero = sum == 0x0;
            f_flags.f_carry = registers.a > sum;
            f_flags.f_half_carry = (registers.a & 0xF) + (sum & 0xF) > 0xF;
            build_f();

            registers.a = sum;
            prog_counter++;

            break;
        }

        // SUB C: Subtract the contents of register C from the contents of register A, and store the results in register A.
        case 0x91: { 
            unsigned char diff = registers.a - registers.c;

            f_flags.f_zero = diff == 0x0;
            f_flags.f_subtract = true;
            f_flags.f_carry = registers.a < diff;
            f_flags.f_half_carry = (registers.a & 0xF) < (registers.c & 0xF);
            build_f();

            registers.a = diff;
            prog_counter++;

            break;
        }

        // AND C: Take the logical AND for each bit of the contents of register C and the contents of register A, 
        // and store the results in register A.
        case 0xA1: {
            unsigned char res = registers.a & registers.c;
            registers.a = res;

            f_flags.f_zero = (res == 0x0);
            f_flags.f_subtract = false;
            f_flags.f_half_carry = true;
            f_flags.f_carry = false;
            build_f();

            prog_counter++;

            break;
        }

        // OR C: Take the logical OR for each bit of the contents of register C and the contents of register A, 
        // and store the results in register A.
        case 0xB1: {
            unsigned char res = registers.a | registers.c;
            registers.a = res;

            f_flags.f_zero = (res == 0x0);
            f_flags.f_subtract = false;
            f_flags.f_half_carry = false;
            f_flags.f_carry = false;
            build_f();

            prog_counter++;

            break;
        }

        // POP BC:
        // Pop the contents from the memory stack into register pair into register pair BC by doing the following:
        // Load the contents of memory specified by stack pointer SP into the lower portion of BC.
        // Add 1 to SP and load the contents from the new memory location into the upper portion of BC.
        // By the end, SP should be 2 more than its initial value.
        case 0xC1: {
            unsigned char n8 = mram[stack_pointer];
            unsigned char n16 = mram[stack_pointer + 1];

            set_bc(n16 << 8 | n8);
            stack_pointer += 2;

            prog_counter++;

            break;
        }

        // POP DE:
        // Pop the contents from the memory stack into register pair into register pair DE by doing the following:
        // Load the contents of memory specified by stack pointer SP into the lower portion of DE.
        // Add 1 to SP and load the contents from the new memory location into the upper portion of DE.
        // By the end, SP should be 2 more than its initial value.
        case 0xD1: {
            unsigned char n8 = mram[stack_pointer];
            unsigned char n16 = mram[stack_pointer + 1];

            set_de(n16 << 8 | n8);
            stack_pointer += 2;

            prog_counter++;

            break;
        }

        // POP HL:
        // Pop the contents from the memory stack into register pair into register pair HL by doing the following:
        // Load the contents of memory specified by stack pointer SP into the lower portion of HL.
        // Add 1 to SP and load the contents from the new memory location into the upper portion of HL.
        // By the end, SP should be 2 more than its initial value.
        case 0xE1: {
            unsigned char n8 = mram[stack_pointer];
            unsigned char n16 = mram[stack_pointer + 1];

            set_hl(n16 << 8 | n8);
            stack_pointer += 2;

            prog_counter++;

            break;
        }

        // POP AF:
        // Pop the contents from the memory stack into register pair into register pair AF by doing the following:
        // Load the contents of memory specified by stack pointer SP into the lower portion of AF.
        // Add 1 to SP and load the contents from the new memory location into the upper portion of AF.
        // By the end, SP should be 2 more than its initial value.
        case 0xF1: {
            unsigned char n8 = mram[stack_pointer];
            unsigned char n16 = mram[stack_pointer + 1];

            set_af(n16 << 8 | n8);
            stack_pointer += 2;

            prog_counter++;

            break;
        }

        // LD (BC), A: Store the contents of register A in the memory location specified by register pair BC.
        case 0x02: {
            mram[get_bc()] = registers.a;
            prog_counter++;

            break;
        }

        // LD (DE), A: Store the contents of register A in the memory location specified by register pair DE.
        case 0x12: {
            mram[get_de()] = registers.a;
            prog_counter++;

            break;
        }

        // LD (HL+), A: Store the contents of register A into the memory location specified by register pair 
        // HL, and simultaneously increment the contents of HL.
        case 0x22: {
            mram[get_hl()] = registers.a;
            set_hl(get_hl() + 1);
            prog_counter++;

            break;
        }

        // LD (HL-), A: Store the contents of register A into the memory location specified by register pair 
        // HL, and simultaneously decrement the contents of HL.
        case 0x32: {
            mram[get_hl()] = registers.a;
            set_hl(get_hl() - 1);
            prog_counter++;

            break;
        }

        // LD B, D: Load the contents of register D into register B.
        case 0x42: {
            registers.b = registers.d;
            prog_counter++;

            break;
        }

        // LD D, D: Load the contents of register D into register D.
        case 0x52: {
            registers.d = registers.d;
            prog_counter++;

            break;
        }

        // LD H, D: Load the contents of register D into register H.
        case 0x62: {
            registers.h = registers.d;
            prog_counter++;

            break;
        }

        // LD (HL), D: Store the contents of register D in the memory location specified by register pair HL.
        case 0x72: {
            mram[get_hl()] = registers.d;
            prog_counter++;

            break;
        }

        // ADD A, D: Add the contents of register D to the contents of register A, and store the results in register A.
        case 0x82: {
            unsigned char sum = registers.a + registers.d;

            f_flags.f_zero = sum == 0x0;
            f_flags.f_subtract = false;
            f_flags.f_carry = registers.a > sum;
            f_flags.f_half_carry = (registers.a & 0xF) + (sum & 0xF) > 0xF;
            build_f();

            registers.a = sum;
            prog_counter++;

            break;
        }

        // SUB D: Subtract the contents of register D from the contents of register A, and store the results in register A.
        case 0x92: { 
            unsigned char diff = registers.a - registers.d;

            f_flags.f_zero = diff == 0x0;
            f_flags.f_subtract = true;
            f_flags.f_carry = registers.a < diff;
            f_flags.f_half_carry = (registers.a & 0xF) < (registers.d & 0xF);
            build_f();

            registers.a = diff;
            prog_counter++;

            break;
        }

        // AND D: Take the logical AND for each bit of the contents of register D and the contents of register A, 
        // and store the results in register A.
        case 0xA2: {
            unsigned char and_res = registers.a & registers.d;
            registers.a = and_res;

            f_flags.f_zero = (and_res == 0x0);
            f_flags.f_subtract = false;
            f_flags.f_half_carry = true;
            f_flags.f_carry = false;
            build_f();

            prog_counter++;

            break;
        }

        // OR D: Take the logical OR for each bit of the contents of register D and the contents of register A, 
        // and store the results in register A.
        case 0xB2: {
            unsigned char op_res = (registers.a | registers.d);
            registers.a = op_res;
            
            f_flags.f_zero = (op_res == 0x0);
            f_flags.f_subtract = false;
            f_flags.f_half_carry = false;
            f_flags.f_carry = false;
            build_f();

            prog_counter++;

            break;
        }

        // JP NZ, a16: Load the 16-bit immediate operand a16 into the program counter PC if the Z flag is 0. 
        // If the Z flag is 0, then the subsequent instruction starts at address a16. If not, the contents 
        // of PC are incremented, and the next instruction following the current JP instruction is executed (as usual).
        case 0xC2: {
            char a8 = static_cast<unsigned char>(mram[prog_counter + 1]);
            char a16 = static_cast<unsigned char>(mram[prog_counter + 2]);

            unsigned short new_a16 = a8 << 8 & a16; 
            if (!f_flags.f_zero) {
                prog_counter = new_a16;
            } else {
                prog_counter++;
            }

            break;
        }

        // JP NC, a16: Load the 16-bit immediate operand a16 into the program counter PC if the CY flag is 0. 
        // If the CY flag is 0, then the subsequent instruction starts at address a16. If not, the contents 
        // of PC are incremented, and the next instruction following the current JP instruction is executed (as usual).
        case 0xD2: {
            char a8 = static_cast<unsigned char>(mram[prog_counter + 1]);
            char a16 = static_cast<unsigned char>(mram[prog_counter + 2]);

            unsigned short new_a16 = a8 << 8 & a16; 
            if (!f_flags.f_carry) {
                prog_counter = new_a16;
            } else {
                prog_counter++;
            }

            break;
        }

        // LD (C), A: Store the contents of register A in the internal RAM, port register, or mode register at the 
        // address in the range 0xFF00-0xFFFF specified by register C.
        case 0xE2: {
            unsigned short hram_loc = 0xFF00 & registers.c;
            mram[hram_loc] = registers.a;

            prog_counter++;
            break;
        }

        // LD A, (C): Load into register A the contents of the internal RAM, port register, or mode register at the 
        // address in the range 0xFF00-0xFFFF specified by register C.
        case 0xF2: {
            unsigned short hram_loc = 0xFF00 & registers.c;
            registers.a = mram[hram_loc];

            prog_counter++;
            break;
        }

        // INC BC: Increment the contents of register pair BC by 1.
        case 0x03: {
            set_bc(get_bc() + 1);
            prog_counter++;

            break;
        }

        // INC DE: Increment the contents of register pair DE by 1.
        case 0x13: {
            set_de(get_de() + 1);
            prog_counter++;

            break;
        }

        // INC HL: Increment the contents of register pair HL by 1.
        case 0x23: {
            set_hl(get_hl() + 1);
            prog_counter++;

            break;
        }
        
        // INC SP: Increment the contents of register pair SP by 1.
        case 0x33: {
            stack_pointer++;
            prog_counter++;

            break;
        }

        // LD B, E: Load the contents of register E into register B.
        case 0x43: {
            registers.b = registers.e;
            prog_counter++;

            break;
        }

        // LD D, E: Load the contents of register E into register D.
        case 0x53: {
            registers.d = registers.e;
            prog_counter++;

            break;
        }

        // LD D, E: Load the contents of register E into register H.
        case 0x63: {
            registers.h = registers.e;
            prog_counter++;

            break;
        }
        
        // LD (HL), E: Store the contents of register E in the memory location specified by register pair HL.
        case 0x73: {
            mram[get_hl()] = registers.e;
            prog_counter++;

            break;
        }

        // ADD A, E: Add the contents of register E to the contents of register A, and store the results in register A.
        case 0x83: {
            unsigned char sum = registers.a + registers.e;

            f_flags.f_zero = sum == 0x0;
            f_flags.f_subtract = false;
            f_flags.f_carry = registers.a > sum;
            f_flags.f_half_carry = (registers.a & 0xF) + (sum & 0xF) > 0xF;
            build_f();

            registers.a = sum;
            prog_counter++;

            break;
        }

        // SUB E: Subtract the contents of register E from the contents of register A, and store the results in register A.
        case 0x93: { 
            unsigned char diff = registers.a - registers.e;

            f_flags.f_zero = diff == 0x0;
            f_flags.f_subtract = true;
            f_flags.f_carry = registers.a < diff;
            f_flags.f_half_carry = (registers.a & 0xF) < (registers.e & 0xF);
            build_f();

            registers.a = diff;
            prog_counter++;

            break;
        }

        // AND E: Take the logical AND for each bit of the contents of register E and the contents of register A, 
        // and store the results in register A.
        case 0xA3: {
            unsigned char and_res = registers.a & registers.e;
            registers.a = and_res;

            f_flags.f_zero = (and_res == 0x0);
            f_flags.f_subtract = false;
            f_flags.f_half_carry = true;
            f_flags.f_carry = false;
            build_f();

            prog_counter++;

            break;
        }

        // OR E: Take the logical OR for each bit of the contents of register E and the contents of register A, 
        // and store the results in register A.
        case 0xB3: {
            unsigned char op_res = (registers.a | registers.e);
            registers.a = op_res;
            
            f_flags.f_zero = (op_res == 0x0);
            f_flags.f_subtract = false;
            f_flags.f_half_carry = false;
            f_flags.f_carry = false;
            build_f();

            prog_counter++;

            break;
        }

        // JP a16: Load the 16-bit immediate operand a16 into the program counter (PC). a16 specifies the 
        // address of the subsequently executed instruction.
        case 0xC3: {
            char a8 = static_cast<unsigned char>(mram[prog_counter + 1]);
            char a16 = static_cast<unsigned char>(mram[prog_counter + 2]);

            unsigned short new_a16 = a8 << 8 & a16; 
            prog_counter = new_a16;

            break;
        }
    }
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

void cpu::build_f() {
    unsigned char new_f = 0b11110000;
    if (!f_flags.f_zero) new_f -= 128;
    if (!f_flags.f_subtract) new_f -= 64;
    if (!f_flags.f_half_carry) new_f -= 32;
    if (!f_flags.f_carry) new_f -= 16;

    registers.f = new_f;
}