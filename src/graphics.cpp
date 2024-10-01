#include <iostream>

#include <graphics.h>
#include <SDL2/SDL.h>

using std::cout;
using std::endl;

graphics::graphics(unsigned int width, unsigned int height, unsigned int size_modifier, const char* title) {
    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow(
        title, 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        width * size_modifier, height * size_modifier, 
        SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (window == NULL) {
        cout << "Couldn't create SDL window" << endl;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetLogicalSize(renderer, width * size_modifier, height * size_modifier);

    texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_ARGB8888, 
        SDL_TEXTUREACCESS_STREAMING, 
        width, height
    );
}

void graphics::update_graphics() {
    SDL_UpdateTexture(texture, nullptr, {}, 64 * sizeof(unsigned int));
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}

bool graphics::fetch_input() {
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: {
                return true;
                break;
            }

            case SDL_KEYDOWN: {
				switch (event.key.keysym.sym) {
					case SDLK_ESCAPE: { return true; }
                }
                break;
            }

            default: {
                break;
            }
        }
    }

    return false;
}

void graphics::end_graphics() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}