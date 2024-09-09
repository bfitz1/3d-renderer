#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

// Test for render mode work with bit twiddling
enum render_mode { 
    MODE_DOT = 0x1,
    MODE_WIRE = 0x2,
    MODE_SOLID = 0x4,
    MODE_TEXTURE = 0x8,
    MODE_WIREDOT = MODE_DOT | MODE_WIRE,
    MODE_SOLIDWIRE = MODE_SOLID | MODE_WIRE,
    MODE_TEXTUREWIRE = MODE_TEXTURE | MODE_WIRE,
}; // display_mode;

// I _could_ pull this into the enum, but since the presented options
// are intended to be mutually exclusive it leads to a bunch of cases
// which are awkward to toggle.
// bool cull_backfaces;
// bool show_depth;

// extern SDL_Window *window;
// extern SDL_Renderer *renderer;
// extern uint32_t *color_buffer;
// extern float *z_buffer;
// extern SDL_Texture* color_buffer_texture;
// extern int window_width;
// extern int window_height;

int get_window_width(void);
int get_window_height(void);
int get_render_mode(void);
void set_render_mode(int mode);
bool get_cull_backfaces(void);
void set_cull_backfaces(bool setting);
void toggle_cull_backfaces(void);
bool get_show_depth(void);
void set_show_depth(bool setting);
void toggle_show_depth(void);
bool initialize_window(void);
void draw_grid(int gridsize);
void draw_checker(int tilesize);
void draw_pixel(int x, int y, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_rect(int posx, int posy, int width, int height, uint32_t color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void render_color_buffer(void);
void render_z_buffer(void);
void clear_color_buffer(uint32_t color);
void clear_z_buffer(void);
float get_z_buffer_at(int x, int y);
void update_z_buffer_at(int x, int y, float value);
void destroy_window(void);
