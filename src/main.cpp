#include <iostream>
#include <SDL2/SDL.h>

#include <cpu.h>
#include <graphics.h>

using std::cout;
using std::endl;

int main(int argc, char *argv[]) {
    graphics* gfx = new graphics(64, 32, 10, "gameboy");
    cpu* c = new cpu();

    bool quit = false;  
    while (!quit && c->running) {
        quit = gfx->fetch_input();
    }

    return 0;
}