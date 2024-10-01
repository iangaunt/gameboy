#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL.h>

class graphics {
    public:
        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Texture* texture;
        SDL_Event event;

        unsigned int width;
        unsigned int height;
        unsigned int size_modifier;

        graphics(unsigned int width, unsigned int height, unsigned int size_modifier, const char* title);
        ~graphics();

        bool fetch_input();
        void update_graphics();
        void end_graphics();
        
};

#endif