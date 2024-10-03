#ifndef CPU_H
#define CPU_H

class cpu {
    public:
        bool running = true;
        
        // fuck you
        unsigned char mram[1500000];
        unsigned char vram[1500000];

        unsigned int width = 160;
        unsigned int height = 144;
        unsigned int size_modifier = 5;

        unsigned short prog_counter = 0x0000;
        unsigned short prog_counter_copy = 0x0000;

        unsigned short stack[256];
        unsigned short stack_pointer = 0x00;

        struct {
            unsigned char a;
            unsigned char b;
            unsigned char c;
            unsigned char d;

            unsigned char e;
            unsigned char f;
            unsigned char h;
            unsigned char l;
        } registers;

        struct {
            bool f_zero = false;
            bool f_subtract = false;
            bool f_half_carry = false;
            bool f_carry = false;
        } f_flags;

        cpu();

        bool load_rom(const char* rom);
        void read();

        short get_af();
        short get_bc();
        short get_de();
        short get_hl();

        void set_af(unsigned short sh_af);
        void set_bc(unsigned short sh_bc);
        void set_de(unsigned short sh_de);
        void set_hl(unsigned short sh_hl);

        void build_f();
};

#endif