#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h> 

#include "display.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
uint32_t *color_buffer = NULL;
float *z_buffer = NULL;
SDL_Texture* color_buffer_texture = NULL;
int window_width = 800;
int window_height = 600;

bool initialize_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

    // Use SDL to query what is the fullscreen max width and height
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    window_width = display_mode.w;
    window_height = display_mode.h;

    // Create an SDL Window
    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_BORDERLESS
    );
    if (!window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }

    // Create an SDL renderer
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    
    return true;
}

void draw_grid(int gridsize) {
    for (int y = 0; y < window_height; y += gridsize) {
        for (int x = 0; x < window_width; x += gridsize) {
            color_buffer[window_width * y + x] = 0xFF999999;
        }
    }
}

void draw_checker(int tilesize) {
    for (int y = 0; y < window_height / tilesize; y++) {
        for (int x = 0; x < window_width / tilesize; x++) {
            if (x % 2 == y % 2) {
                draw_rect(x * tilesize, y * tilesize, tilesize, tilesize, 0xFF151515);
            }
        }
    }
}

void draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < window_width && y >= 0 && y < window_height) {
        color_buffer[window_width * y + x] = color;
    }
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = x1 - x0;
    int dy = y1 - y0;

    int longest_side_length = abs(dx) >= abs(dy) ? abs(dx) : abs(dy);

    // Increment for each step (at least one of these has a step of 1)
    float x_inc = dx / (float)longest_side_length;
    float y_inc = dy / (float)longest_side_length;

    float current_x = x0;
    float current_y = y0;

    for (int i = 0; i <= longest_side_length; i++) {
        draw_pixel(round(current_x), round(current_y), color);
        current_x += x_inc;
        current_y += y_inc;
    }
}

void draw_rect(int posx, int posy, int width, int height, uint32_t color) {
    for (int y = posy; y < posy + height; y += 1) {
        for (int x = posx; x < posx + width; x += 1) {
            draw_pixel(x, y, color);
        }
    }
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

void render_color_buffer(void) {
    SDL_UpdateTexture(
        color_buffer_texture,
        NULL,
        color_buffer,
        (int) window_width * sizeof (uint32_t)
    );
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void render_z_buffer(void) {
    // Place z-buffer values into color buffer as RGBA in order to
    // visualize depth
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            if (z_buffer[window_width * y + x] < 1.0) {
                uint8_t c = 0xFF * (1 - z_buffer[window_width * y + x]);
                color_buffer[window_width * y + x] = 0xFF000000 | (c << 16) | (c << 8) | c;
            }
        }
    }

    render_color_buffer();
}

void clear_color_buffer(uint32_t color) {
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
           color_buffer[window_width * y + x] = color; 
        }
    }
}

void clear_z_buffer(void) {
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            z_buffer[window_width * y + x] = 1.0;
        }
    }
}

void destroy_window(void) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
