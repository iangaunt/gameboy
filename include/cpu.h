#ifndef CPU_H
#define CPU_H

class cpu {
    public:
        bool running = true;
        
        unsigned char mram[8000];
        unsigned char vram[8000];

        unsigned int width = 160;
        unsigned int height = 144;
        unsigned int size_modifier = 5;

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

        cpu();
        ~cpu();

        short get_af();
        short get_bc();
        short get_de();
        short get_hl();

        void set_af(unsigned short sh_af);
        void set_bc(unsigned short sh_bc);
        void set_de(unsigned short sh_de);
        void set_hl(unsigned short sh_hl);
};

#endif