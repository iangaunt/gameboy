#include <iostream>
#include <SDL2/SDL.h>

#include <cpu.h>
#include <graphics.h>

using std::cout;
using std::endl;

int main(int argc, char *argv[]) {
    graphics* gfx = new graphics(160, 144, 3, "gameboy");
    cpu* c = new cpu();

    bool loaded = c->load_rom("C:/Users/ianga/Desktop/Codespaces/gb/roms/pokemon_red.gb");
    if (!loaded) return -1;

    bool quit = false;  
    while (!quit && c->running) {
        quit = gfx->fetch_input();
        c->read();
    }

    return 0;
}