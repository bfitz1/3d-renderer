#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h> // Search the SDL2 directory (system path) for SDL.h

#include "display.h"
#include "vector.h"

// Declare an array of vectors/points
const int N_POINTS = 9*9*9;
vec3_t cube_points[N_POINTS]; // 9x9x9 cube

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

    int point_count = 0;

    // Start loading my array of vectors
    // From -1 to 1 (in this 9x9x9 cube)
    for (float x = -1; x <= 1; x += 0.25) {
        for (float y = -1; y <= 1; y += 0.25) {
            for (float z = -1; z <= 1; z += 0.25) {
                vec3_t new_point = { .x = x, .y = y, .z = z };
                cube_points[point_count++] = new_point;
            }
        }
    }
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

    while (is_running) {
        process_input();
        update();
        render();
    }
    
    destroy_window();

    return 0;
}
