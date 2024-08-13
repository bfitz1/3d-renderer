#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h> // Search the SDL2 directory (system path) for SDL.h

#include "display.h"
#include "vector.h"

bool is_running = false;

void setup(void) {
    // Allocate memory (in bytes) to hold the color buffer
    color_buffer = (uint32_t *) malloc(sizeof (uint32_t) * window_width * window_height);

    // Create an SDL texture to display the color buffer
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );
}

void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
    case SDL_QUIT:
        is_running = false;
        break;
    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            is_running = false;
        }
        break;
    }
}

void update(void) {
    // TODO: 
}

void render(void) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderClear(renderer);

    draw_grid(30);
    draw_pixel(20, 20, 0xFFFFFF00);
    draw_rect(60, 60, 200, 150, 0xFFCCCCCC);
    render_color_buffer();
    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

int main(void) {
    is_running = initialize_window();

    setup();

    vec3_t myvector = { 2.0, 3.0, -4.0 };

    while (is_running) {
        process_input();
        update();
        render();
    }
    
    destroy_window();

    return 0;
}
