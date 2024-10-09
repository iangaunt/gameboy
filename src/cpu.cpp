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
            set_f(sum == 0x0, false, ((registers.a & 0xF) + (sum & 0xF) > 0xF), registers.a > sum);

            registers.a = sum;
            prog_counter++;

            break;
        }

        // SUB B: Subtract the contents of register B from the contents of register A, and store the results in register A.
        case 0x90: { 
            unsigned char diff = registers.a - registers.b;
            set_f(diff == 0x0, true, ((registers.a & 0xF) < (registers.b & 0xF)), registers.a < diff);

            registers.a = diff;
            prog_counter++;

            break;
        }

        // AND B: Take the logical AND for each bit of the contents of register B and the contents of register A, 
        // and store the results in register A.
        case 0xA0: {
            unsigned char and_res = registers.a & registers.b;
            registers.a = and_res;
            set_f(and_res == 0x0, false, true, false);

            prog_counter++;

            break;
        }

        // OR B: Take the logical OR for each bit of the contents of register B and the contents of register A, 
        // and store the results in register A.
        case 0xB0: {
            unsigned char op_res = (registers.a | registers.b);
            registers.a = op_res;
            set_f(op_res == 0x0, false, false, false);

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
            set_f(sum == 0x0, false, ((registers.a & 0xF) + (sum & 0xF) > 0xF), registers.a > sum);

            registers.a = sum;
            prog_counter++;

            break;
        }

        // SUB C: Subtract the contents of register C from the contents of register A, and store the results in register A.
        case 0x91: { 
            unsigned char diff = registers.a - registers.c;
            set_f(diff == 0x0, true, ((registers.a & 0xF) < (registers.c & 0xF)), registers.a < diff);

            registers.a = diff;
            prog_counter++;

            break;
        }

        // AND C: Take the logical AND for each bit of the contents of register C and the contents of register A, 
        // and store the results in register A.
        case 0xA1: {
            unsigned char res = registers.a & registers.c;
            registers.a = res;
            set_f(res == 0x0, false, true, false);

            prog_counter++;

            break;
        }

        // OR C: Take the logical OR for each bit of the contents of register C and the contents of register A, 
        // and store the results in register A.
        case 0xB1: {
            unsigned char res = registers.a | registers.c;
            registers.a = res;
            set_f(res == 0x0, false, false, false);

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
            set_f(sum == 0x0, false, (registers.a & 0xF) + (sum & 0xF) > 0xF, registers.a > sum);

            registers.a = sum;
            prog_counter++;

            break;
        }

        // SUB D: Subtract the contents of register D from the contents of register A, and store the results in register A.
        case 0x92: { 
            unsigned char diff = registers.a - registers.d;
            set_f(diff == 0x0, true, ((registers.a & 0xF) < (registers.d & 0xF)), registers.a < diff);

            registers.a = diff;
            prog_counter++;

            break;
        }

        // AND D: Take the logical AND for each bit of the contents of register D and the contents of register A, 
        // and store the results in register A.
        case 0xA2: {
            unsigned char and_res = registers.a & registers.d;
            registers.a = and_res;
            set_f(and_res == 0x0, false, true, false);

            prog_counter++;

            break;
        }

        // OR D: Take the logical OR for each bit of the contents of register D and the contents of register A, 
        // and store the results in register A.
        case 0xB2: {
            unsigned char op_res = (registers.a | registers.d);
            registers.a = op_res;
            set_f(op_res == 0x0, false, false, false);

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
            set_f(sum == 0x0, false, (registers.a & 0xF) + (sum & 0xF) > 0xF, registers.a > sum);

            registers.a = sum;
            prog_counter++;

            break;
        }

        // SUB E: Subtract the contents of register E from the contents of register A, and store the results in register A.
        case 0x93: { 
            unsigned char diff = registers.a - registers.e;
            set_f(diff == 0x0, true, ((registers.a & 0xF) < (registers.e & 0xF)), registers.a < diff);

            registers.a = diff;
            prog_counter++;

            break;
        }

        // AND E: Take the logical AND for each bit of the contents of register E and the contents of register A, 
        // and store the results in register A.
        case 0xA3: {
            unsigned char and_res = registers.a & registers.e;
            registers.a = and_res;
            set_f(and_res == 0x0, false, true, false);

            prog_counter++;

            break;
        }

        // OR E: Take the logical OR for each bit of the contents of register E and the contents of register A, 
        // and store the results in register A.
        case 0xB3: {
            unsigned char op_res = (registers.a | registers.e);
            registers.a = op_res;
            set_f(op_res == 0x0, false, false, false);

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

        // TODO? Maybe?
        case 0xF3: { break; }

        // INC B: Increment the contents of register B by 1.
        case 0x04: {
            unsigned char sum = registers.b + 1;
            set_f(sum == 0x0, false, ((registers.b & 0xF) + (sum & 0xF) > 0xF), f_flags.f_carry);

            registers.b = sum;
            prog_counter++;

            break;
        }

        // INC D: Increment the contents of register D by 1.
        case 0x14: {
            unsigned char sum = registers.d + 1;
            set_f(sum == 0x0, false, ((registers.d & 0xF) + (sum & 0xF) > 0xF), f_flags.f_carry);

            registers.d = sum;
            prog_counter++;

            break;
        }

        // INC D: Increment the contents of register H by 1.
        case 0x24: {
            unsigned char sum = registers.h + 1;
            set_f(sum == 0x0, false, ((registers.h & 0xF) + (sum & 0xF) > 0xF), f_flags.f_carry);

            registers.h = sum;
            prog_counter++;

            break;
        }

        // INC (HL): Increment the contents of memory specified by register pair HL by 1.
        case 0x34: {
            unsigned char sum = mram[get_hl()] + 1;
            set_f(sum == 0x0, false, ((mram[get_hl()] & 0xF) + (sum & 0xF) > 0xF), f_flags.f_carry);

            mram[get_hl()] = sum;
            prog_counter++;

            break;
        }

        // LD B, H: Load the contents of register H into register B.
        case 0x44: {
            registers.b = registers.h;
            prog_counter++;

            break;
        }

        // LD D, H: Load the contents of register H into register D.
        case 0x54: {
            registers.d = registers.h;
            prog_counter++;

            break;
        }

        // LD H, H: Load the contents of register H into register H.
        case 0x64: {
            registers.h = registers.h;
            prog_counter++;

            break;
        }

        // LD (HL), H: Store the contents of register H in the memory location specified by register pair HL.
        case 0x74: {
            mram[get_hl()] = registers.h;
            prog_counter++;

            break;
        }

        // ADD A, H: Add the contents of register H to the contents of register A, and store the results in register A.
        case 0x84: {
            unsigned char sum = registers.a + registers.h;
            set_f(sum == 0x0, false, ((registers.a & 0xF) + (sum & 0xF) > 0xF), registers.a > sum);

            registers.a = sum;
            prog_counter++;

            break;
        }

        // SUB H: Subtract the contents of register H from the contents of register A, and store the results in register A.
        case 0x94: { 
            unsigned char diff = registers.a - registers.h;
            set_f(diff == 0x0, true, ((registers.a & 0xF) < (registers.h & 0xF)), registers.a < diff);

            registers.a = diff;
            prog_counter++;

            break;
        }

        // AND H: Take the logical AND for each bit of the contents of register H and the contents of register A, 
        // and store the results in register A.
        case 0xA4: {
            unsigned char and_res = registers.a & registers.h;
            registers.a = and_res;
            set_f(and_res == 0x0, false, true, false);

            prog_counter++;

            break;
        }

        // OR E: Take the logical OR for each bit of the contents of register E and the contents of register A, 
        // and store the results in register A.
        case 0xB4: {
            unsigned char op_res = (registers.a | registers.h);
            registers.a = op_res;
            set_f(op_res == 0x0, false, false, false);

            prog_counter++;

            break;
        }

        // TODO
        case 0xC4: { break; }
        case 0xD4: { break; }

        // DEC B: Decrement the contents of register B by 1.
        case 0x05: {
            unsigned char diff = registers.b - 1;
            set_f(diff == 0x0, true, ((registers.b & 0xF) + (diff & 0xF) > 0xF), f_flags.f_carry);

            registers.b = diff;
            prog_counter++;

            break;
        }

        // DEC D: Decrement the contents of register D by 1.
        case 0x15: {
            unsigned char diff = registers.d - 1;
            set_f(diff == 0x0, true, ((registers.d & 0xF) + (diff & 0xF) > 0xF), f_flags.f_carry);
            
            registers.d = diff;
            prog_counter++;

            break;
        }

        // DEC H: Decrement the contents of register H by 1.
        case 0x25: {
            unsigned char diff = registers.h - 1;
            set_f(diff == 0x0, true, ((registers.h & 0xF) + (diff & 0xF) > 0xF), f_flags.f_carry);
            
            registers.h = diff;
            prog_counter++;

            break;
        }

        // DEC (HL): Decrement the contents of memory specified by register pair HL by 1.
        case 0x35: {
            unsigned char diff = mram[get_hl()] - 1;
            set_f(diff == 0x0, true, ((mram[get_hl()] & 0xF) + (diff & 0xF) > 0xF), f_flags.f_carry);
            
            mram[get_hl()] = diff;
            prog_counter++;

            break;
        }

        // LD B, L: Loads the contents of register L into register B.
        case 0x45: {
            registers.b = registers.l;
            prog_counter++;

            break;
        }
        
        // LD D, L: Loads the contents of register L into register D.
        case 0x55: {
            registers.d = registers.l;
            prog_counter++;

            break;
        }

        // LD H, L: Loads the contents of register L into register H.
        case 0x65: {
            registers.h = registers.l;
            prog_counter++;

            break;
        }

        // LD D, L: Store the contents of register L in the memory location specified by register pair HL.
        case 0x75: {
            mram[get_hl()] = registers.l;
            prog_counter++;

            break;
        }

        // ADD A, L: Add the contents of register L to the contents of register A, and store the results in register A.
        case 0x85: {
            unsigned char sum = registers.a + registers.l;
            set_f(sum == 0x0, false, ((registers.a & 0xF) + (sum & 0xF) > 0xF), registers.a > sum);

            registers.a = sum;
            prog_counter++;

            break;
        }

        // SUB L: Subtract the contents of register L from the contents of register A, and store the results in register A.
        case 0x95: { 
            unsigned char diff = registers.a - registers.l;
            set_f(diff == 0x0, true, ((registers.a & 0xF) < (registers.l & 0xF)), registers.a < diff);

            registers.a = diff;
            prog_counter++;

            break;
        }

        // AND L: Take the logical AND for each bit of the contents of register L and the contents of register A, 
        // and store the results in register A.
        case 0xA5: {
            unsigned char and_res = registers.a & registers.l;
            registers.a = and_res;
            set_f(and_res == 0x0, false, true, false);

            prog_counter++;

            break;
        }

        // OR E: Take the logical OR for each bit of the contents of register E and the contents of register A, 
        // and store the results in register A.
        case 0xB5: {
            unsigned char op_res = (registers.a | registers.l);
            registers.a = op_res;
            set_f(op_res == 0x0, false, false, false);

            prog_counter++;

            break;
        }

        // TODO
        case 0xC5: { break; }
        case 0xD5: { break; }
        case 0xE5: { break; }
        case 0xF5: { break; }

        // LD B, d8: Load the 8-bit immediate operand d8 into register B.
        case 0x06: {
            unsigned char d8 = mram[prog_counter + 1];
            registers.b = d8;

            prog_counter++;
            break;
        }

        // LD D, d8: Load the 8-bit immediate operand d8 into register D.
        case 0x16: {
            unsigned char d8 = mram[prog_counter + 1];
            registers.d = d8;

            prog_counter++;
            break;
        }

        // LD H, d8: Load the 8-bit immediate operand d8 into register H.
        case 0x26: {
            unsigned char d8 = mram[prog_counter + 1];
            registers.h = d8;

            prog_counter++;
            break;
        }

        // LD (HL), d8: Store the contents of 8-bit immediate operand d8 in the memory location specified by register pair HL.
        case 0x36: {
            unsigned char d8 = mram[prog_counter + 1];
            mram[get_hl()] = d8;

            prog_counter++;
            break;
        }

        // LD B, (HL): Load the 8-bit contents of memory specified by register pair HL into register B.
        case 0x46: {
            registers.b = mram[get_hl()];

            prog_counter++;
            break;
        }

        // LD D, (HL): Load the 8-bit contents of memory specified by register pair HL into register D.
        case 0x56: {
            registers.d = mram[get_hl()];

            prog_counter++;
            break;
        }

        // LD H, (HL): Load the 8-bit contents of memory specified by register pair HL into register H.
        case 0x66: {
            registers.h = mram[get_hl()];

            prog_counter++;
            break;
        }

        // TODO: HALT
        case 0x76: { break; }

        // ADD A, (HL): Add the contents of memory specified by register pair HL to the contents 
        // of register A, and store the results in register A.
        case 0x86: {
            unsigned char sum = registers.a + mram[get_hl()];
            set_f(sum == 0x0, false, ((registers.a & 0xF) + (sum & 0xF) > 0xF), registers.a > sum);

            registers.a = sum;
            prog_counter++;

            break;
        }

        // SUB (HL): Subtract the contents of register L from the contents of register A, and store the results in register A.
        case 0x96: { 
            unsigned char diff = registers.a - mram[get_hl()];
            set_f(diff == 0x0, true, ((registers.a & 0xF) < (mram[get_hl()] & 0xF)), registers.a < diff);

            registers.a = diff;
            prog_counter++;

            break;
        }

        // AND (HL): Take the logical AND for each bit of the contents of memory specified by register 
        // pair HL and the contents of register A, and store the results in register A.
        case 0xA6: {
            unsigned char and_res = registers.a & mram[get_hl()];
            registers.a = and_res;
            set_f(and_res == 0x0, false, true, false);

            prog_counter++;

            break;
        }

        // OR (HL): Take the logical OR for each bit of the contents of memory specified by register 
        // pair HL and the contents of register A, and store the results in register A.
        case 0xB6: {
            unsigned char op_res = (registers.a | mram[get_hl()]);
            registers.a = op_res;
            set_f(op_res == 0x0, false, false, false);

            prog_counter++;

            break;
        }

        // ADD A, d8: Add the contents of the 8-bit immediate operand d8 to the 
        // contents of register A, and store the results in register A.
        case 0xC6: {
            unsigned char sum = registers.a + mram[prog_counter + 1];
            set_f(sum == 0x0, false, ((registers.a & 0xF) + (sum & 0xF) > 0xF), registers.a > sum);

            registers.a = sum;
            prog_counter++;

            break;
        }

        // SUB d8: Subtract the contents of the 8-bit immediate operand d8 from the 
        // contents of register A, and store the results in register A.
        case 0xD6: {
            unsigned char diff = registers.a - mram[prog_counter + 1];
            set_f(diff == 0x0, true, ((registers.a & 0xF) < (mram[prog_counter + 1] & 0xF)), registers.a < diff);

            registers.a = diff;
            prog_counter++;

            break;
        }

        // AND d8: Take the logical AND for each bit of the contents of 8-bit immediate
        // operand d8 and the contents of register A, and store the results in register A.
        case 0xE6: {
            unsigned char and_res = registers.a & mram[prog_counter + 1];
            registers.a = and_res;
            set_f(and_res == 0x0, false, true, false);

            prog_counter++;

            break;
        }

        // OR d8: Take the logical OR for each bit of the contents of the 8-bit immediate 
        // operand d8 and the contents of register A, and store the results in register A.
        case 0xF6: {
            unsigned char op_res = (registers.a | mram[prog_counter + 1]);
            registers.a = op_res;
            set_f(op_res == 0x0, false, false, false);

            prog_counter++;

            break;
        }

        // RLCA: Rotate the contents of register A to the left. That is, the contents of 
        // bit 0 are copied to bit 1, and the previous contents of bit 1 (before the copy operation) are 
        // copied to bit 2. The same operation is repeated in sequence for the rest of the register. The 
        // contents of bit 7 are placed in both the CY flag and bit 0 of register A.
        case 0x07: {
            unsigned short sh = static_cast<unsigned short>(registers.a);
            sh *= 2;

            sh += (sh >> 8);
            f_flags.f_carry = sh >> 8;
            build_f();

            registers.a = static_cast<unsigned char>(sh);
            prog_counter++;
            break;
        }

        // RLCA: Rotate the contents of register A to the left, through the carry (CY) flag. 
        // That is, the contents of bit 0 are copied to bit 1, and the previous contents of bit 1 
        // (before the copy operation) are copied to bit 2. The same operation is repeated in sequence
        // for the rest of the register. The previous contents of the carry flag are copied to bit 0.
        case 0x17: {
            unsigned short sh = static_cast<unsigned short>(registers.a);
            sh *= 2;

            sh += f_flags.f_carry ? 1 : 0;
            f_flags.f_carry = sh >> 8;
            build_f();

            registers.a = static_cast<unsigned char>(sh);
            prog_counter++;
            break;
        }

        // TODO: DAA
        case 0x27: { break; }

        // SCF: Set the carry flag CY.
        case 0x37: {
            f_flags.f_carry = true;
            build_f();
        }

        // LD B, A: Load the contents of register A into register B.
        case 0x47: {
            registers.b = registers.a;

            prog_counter++;
            break;
        }

        // LD D, A: Load the contents of register A into register D.
        case 0x57: {
            registers.d = registers.a;

            prog_counter++;
            break;
        }

        // LD H, A: Load the contents of register A into register H.
        case 0x67: {
            registers.h = registers.a;

            prog_counter++;
            break;
        }

        // LD (HL), A: Store the contents of register A in the memory location specified by register pair HL.
        case 0x77: {
            mram[get_hl()] = registers.a;

            prog_counter++;
            break;
        }

        // ADD A, A: Add the contents of register A to the contents of register A, and store the results in register A.
        case 0x87: {
            unsigned char sum = registers.a + registers.a;
            set_f(sum == 0x0, false, ((registers.a & 0xF) + (sum & 0xF) > 0xF), registers.a > sum);

            registers.a = sum;
            prog_counter++;

            break;
        }

        // SUB A: Subtract the contents of register A from the contents of register A, and store the results in register A.
        case 0x97: {
            unsigned char diff = registers.a - registers.a;
            set_f(diff == 0x0, true, ((registers.a & 0xF) < (registers.a & 0xF)), registers.a < diff);

            registers.a = diff;
            prog_counter++;

            break;
        }

        // AND A: Take the logical AND for each bit of the contents of register A and 
        // the contents of register A, and store the results in register A.
        case 0xA7: {
            unsigned char and_res = registers.a & registers.a;
            registers.a = and_res;
            set_f(and_res == 0x0, false, true, false);

            prog_counter++;

            break;
        }

        // OR A: Take the logical OR for each bit of the contents of register A and the 
        // contents of register A, and store the results in register A.
        case 0xB7: {
            unsigned char op_res = (registers.a | registers.a);
            registers.a = op_res;
            set_f(op_res == 0x0, false, false, false);

            prog_counter++;

            break;
        }

        // RST 0: Push the current value of the program counter PC onto the memory stack, and load into PC 
        // the 1st byte of page 0 memory addresses, 0x00. The next instruction is fetched from the address 
        // specified by the new content of PC (as usual).
        case 0xC7: {
            stack_pointer--;
            mram[stack_pointer] = prog_counter >> 8;

            stack_pointer--;
            mram[stack_pointer] = prog_counter & 0x00ff;
            
            prog_counter = 0x00;
            break;
        }

        // RST 2: Push the current value of the program counter PC onto the memory stack, and load into PC 
        // the 3rd byte of page 0 memory addresses, 0x10. The next instruction is fetched from the address 
        // specified by the new content of PC (as usual).
        case 0xD7: {
            stack_pointer--;
            mram[stack_pointer] = prog_counter >> 8;

            stack_pointer--;
            mram[stack_pointer] = prog_counter & 0x00ff;
            
            prog_counter = 0x10;
            break;
        }

        // RST 4: Push the current value of the program counter PC onto the memory stack, and load into PC 
        // the 5th byte of page 0 memory addresses, 0x20. The next instruction is fetched from the address 
        // specified by the new content of PC (as usual).
        case 0xE7: {
            stack_pointer--;
            mram[stack_pointer] = prog_counter >> 8;

            stack_pointer--;
            mram[stack_pointer] = prog_counter & 0x00ff;
            
            prog_counter = 0x20;
            break;
        }

        // RST 6: Push the current value of the program counter PC onto the memory stack, and load into PC 
        // the 5th byte of page 0 memory addresses, 0x30. The next instruction is fetched from the address 
        // specified by the new content of PC (as usual).
        case 0xF7: {
            stack_pointer--;
            mram[stack_pointer] = prog_counter >> 8;

            stack_pointer--;
            mram[stack_pointer] = prog_counter & 0x00ff;
            
            prog_counter = 0x30;
            break;
        }

        // TODO: LD (a16), SP: Store the lower byte of stack pointer SP at the address specified by the 16-bit 
        // immediate operand a16, and store the upper byte of SP at address a16 + 1.
        case 0x08: { break; }

        // JR s8: Jump s8 steps from the current address in the program counter (PC). (Jump relative.)
        case 0x18: {
            char s8 = static_cast<signed char>(mram[prog_counter + 1]);
            prog_counter += s8;

            break;
        }
        
        // JR Z, s8: If the Z flag is 1, jump s8 steps from the current address stored in the program counter (PC). 
        // If not, the instruction following the current JP instruction is executed (as usual).
        case 0x28: {
            char s8 = static_cast<signed char>(mram[prog_counter + 1]);
            prog_counter += f_flags.f_zero ? s8 : 1;

            break;
        }

        // JR CY, s8: If the CY flag is 1, jump s8 steps from the current address stored in the program counter (PC). 
        // If not, the instruction following the current JP instruction is executed (as usual).
        case 0x38: {
            char s8 = static_cast<signed char>(mram[prog_counter + 1]);
            prog_counter += f_flags.f_carry ? s8 : 1;

            break;
        }

        // LD C, B: Load the contents of register B into register C.
        case 0x48: {
            registers.c = registers.b;

            prog_counter++;
            break;
        }

        // LD E, B: Load the contents of register B into register E.
        case 0x58: {
            registers.e = registers.b;

            prog_counter++;
            break;
        }

        // LD L, B: Load the contents of register B into register L.
        case 0x68: {
            registers.l = registers.b;

            prog_counter++;
            break;
        }

        // LD A, B: Load the contents of register B into register A.
        case 0x78: {
            registers.a = registers.b;

            prog_counter++;
            break;
        }

        // ADC A, B: Add the contents of register B and the CY flag to the contents of register A, and 
        // store the results in register A.
        case 0x88: {
            unsigned char sum = registers.a + registers.b;
            if (f_flags.f_carry) sum += 1;

            set_f(sum == 0x0, false, ((registers.a & 0xF) + (sum & 0xF) > 0xF), registers.a > sum);

            registers.a = sum;
            prog_counter++;

            break;
        }

        // SBC A, B: Subtract the contents of register B and the CY flag from the contents of register A, 
        // and store the results in register A.
        case 0x98: {
            unsigned char diff = registers.a - registers.b;
            if (f_flags.f_carry) diff -= 1;
            set_f(diff == 0x0, true, ((registers.a & 0xF) < (registers.b & 0xF)), registers.a < diff);

            registers.a = diff;
            prog_counter++;

            break;
        }

        // XOR B: Take the logical exclusive-OR for each bit of the contents of register B and the 
        // contents of register A, and store the results in register A.
        case 0xA8: {
            unsigned char xor_res = (registers.a ^ registers.b);
            registers.a = xor_res;
            set_f(xor_res == 0x0, false, false, false);

            prog_counter++;

            break;
        }

        // CP B: Compare the contents of register B and the contents of register A by calculating A - B, 
        // and set the Z flag if they are equal. The execution of this instruction does not affect the 
        // contents of register A.
        case 0xB8: {
            unsigned char diff = registers.a - registers.b;
            set_f(diff == 0x0, true, ((registers.a & 0xF) < (registers.b & 0xF)), registers.a < diff);

            prog_counter++;

            break;
        }

        // TODO
        case 0xC8: { break; }
        case 0xD8: { break; }

        // ADD SP, s8: Add the contents of the 8-bit signed (2's complement) immediate operand s8 and 
        // the stack pointer SP and store the results in SP.
        case 0xE8: {
            char s8 = static_cast<char>(mram[prog_counter + 1]);
            unsigned short sp_sum = stack_pointer + s8;
            
            set_f(false, false, ((stack_pointer & 0xF) + (sp_sum & 0xF) > 0xF), stack_pointer > sp_sum);

            stack_pointer = sp_sum;
            prog_counter++;

            break;
        }

        // LD HL, SP+s8: Add the 8-bit signed operand s8 (values -128 to +127) to the stack pointer SP, 
        // and store the result in register pair HL.
        case 0xF8: {
            signed char s8 = static_cast<signed char>(mram[prog_counter + 1]);
            unsigned short sp_sum = stack_pointer + s8;
            
            set_f(false, false, ((stack_pointer & 0xF) + (sp_sum & 0xF) > 0xF), stack_pointer > sp_sum);
            set_hl(sp_sum);
            
            prog_counter++;

            break;
        }

        // RST 1: Push the current value of the program counter PC onto the memory stack, and load into PC 
        // the 8th byte of page 0 memory addresses, 0x08. The next instruction is fetched from the address 
        // specified by the new content of PC (as usual).
        case 0xCF: {
            stack_pointer--;
            mram[stack_pointer] = prog_counter >> 8;

            stack_pointer--;
            mram[stack_pointer] = prog_counter & 0x00ff;
            
            prog_counter = 0x08;
            break;
        }

        // RST 3: Push the current value of the program counter PC onto the memory stack, and load into PC 
        // the 8th byte of page 0 memory addresses, 0x18. The next instruction is fetched from the address 
        // specified by the new content of PC (as usual).
        case 0xDF: {
            stack_pointer--;
            mram[stack_pointer] = prog_counter >> 8;

            stack_pointer--;
            mram[stack_pointer] = prog_counter & 0x00ff;
            
            prog_counter = 0x18;
            break;
        }

        // RST 5: Push the current value of the program counter PC onto the memory stack, and load into PC 
        // the 8th byte of page 0 memory addresses, 0x28. The next instruction is fetched from the address 
        // specified by the new content of PC (as usual).
        case 0xEF: {
            stack_pointer--;
            mram[stack_pointer] = prog_counter >> 8;

            stack_pointer--;
            mram[stack_pointer] = prog_counter & 0x00ff;
            
            prog_counter = 0x28;
            break;
        }

        // RST 7: Push the current value of the program counter PC onto the memory stack, and load into PC 
        // the 8th byte of page 0 memory addresses, 0x38. The next instruction is fetched from the address 
        // specified by the new content of PC (as usual).
        case 0xFF: {
            stack_pointer--;
            mram[stack_pointer] = prog_counter >> 8;

            stack_pointer--;
            mram[stack_pointer] = prog_counter & 0x00ff;

            prog_counter = 0x38;
            break;
        }

        default: {
            cout << std::hex << "unknown opcode " << opcode << endl;
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

void cpu::set_f(bool fz, bool fs, bool fh, bool fcy) {
    f_flags.f_zero = fz;
    f_flags.f_subtract = fs;
    f_flags.f_half_carry = fh;
    f_flags.f_carry = fcy;

    build_f();
}

void cpu::build_f() {
    unsigned char new_f = 0b11110000;
    if (!f_flags.f_zero) new_f -= 128;
    if (!f_flags.f_subtract) new_f -= 64;
    if (!f_flags.f_half_carry) new_f -= 32;
    if (!f_flags.f_carry) new_f -= 16;

    registers.f = new_f;
}