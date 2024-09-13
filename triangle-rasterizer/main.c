#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)
#define WIDTH 800
#define HEIGHT 600
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

typedef struct {
    float x, y;
} vec2_t;

typedef struct {
    uint8_t r, g, b;
} color_t;

// Back to globals! Don't feel like the separation is worth it for this
// small experiment
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
uint32_t *color_buffer = NULL;
SDL_Texture *color_buffer_texture = NULL;

bool is_running = true;

vec2_t vertices[4] = {
    { 100, 20 },
    { 200, 280 },
    {  30, 210 },
    { 125, 450 },
};

color_t colors[3] = {
    { .r = 0xFF, .g = 0x00, .b = 0x00 },
    { .r = 0x00, .g = 0xFF, .b = 0x00 },
    { .r = 0x00, .g = 0x00, .b = 0xFF },
};

// Assumes a clockwise orientation of vertices
int edge_cross(vec2_t *a, vec2_t *b, vec2_t *c) {
    vec2_t ab = { b->x - a->x, b->y - a->y };
    vec2_t ac = { c->x - a->x, c->y - a->y };
    return ab.x * ac.y - ab.y * ac.x;
}

bool is_top_left(vec2_t *start, vec2_t *end) {
    vec2_t edge = { end->x - start->x, end->y - start->y };

    bool is_top_edge = edge.y == 0 && edge.x > 0;
    bool is_left_edge = edge.y < 0;

    return is_top_edge || is_left_edge;
}

void set_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
    color_buffer[WIDTH * y + x] = color;
}

void fill_triangle(vec2_t *v0, vec2_t *v1, vec2_t *v2) {
    // Find the bounding box containing the entire triangle
    int xmin = MIN(v0->x, MIN(v1->x, v2->x));
    int ymin = MIN(v0->y, MIN(v1->y, v2->y));
    int xmax = MAX(v0->x, MAX(v1->x, v2->x));
    int ymax = MAX(v0->y, MAX(v1->y, v2->y));

    // Compute the constant delta_s that will be used for the horizontal and vertical steps
    int delta_w0_col = v1->y - v2->y;
    int delta_w1_col = v2->y - v0->y;
    int delta_w2_col = v0->y - v1->y;

    int delta_w0_row = v2->x - v1->x;
    int delta_w1_row = v0->x - v2->x;
    int delta_w2_row = v1->x - v0->x;

    // Parallelogram area is fine since that extra 1/2 term gets
    // divided away later
    float area = edge_cross(v0, v1, v2);

    int bias0 = is_top_left(v1, v2) ? 0 : -1;
    int bias1 = is_top_left(v2, v0) ? 0 : -1;
    int bias2 = is_top_left(v0, v1) ? 0 : -1;
    
    vec2_t p0 = { xmin, ymin };
    int w0_row = edge_cross(v1, v2, &p0) + bias0;
    int w1_row = edge_cross(v2, v0, &p0) + bias1;
    int w2_row = edge_cross(v0, v1, &p0) + bias2;

    for (int y = ymin; y < ymax; y += 1) {
        float w0 = w0_row;
        float w1 = w1_row;
        float w2 = w2_row;
        for (int x = xmin; x < xmax; x += 1) {
            // A point is inside the triangle if it is on the "right" side
            // of every edge going clockwise.
            // Note that these values are constant along their edge, so we can
            // recompute them more efficiently using deltas
            bool is_inside = w0 >= 0 && w1 >= 0 && w2 >= 0;
            if (is_inside) {
                // Barycentric weights
                float alpha = w0 / (float)area;
                float beta = w1 / (float)area;
                float gamma = w2 / (float)area;

                int r = (alpha) * colors[0].r + (beta) * colors[1].r + (gamma) * colors[2].r;
                int g = (alpha) * colors[0].g + (beta) * colors[1].g + (gamma) * colors[2].g;
                int b = (alpha) * colors[0].b + (beta) * colors[1].b + (gamma) * colors[2].b;

                uint32_t interpolated_color = 0xFF << 24 | b << 16 | g << 8 | r;

                set_pixel(x, y, interpolated_color);
            }
            w0 += delta_w0_col;
            w1 += delta_w1_col;
            w2 += delta_w2_col;
        }
        w0_row += delta_w0_row;
        w1_row += delta_w1_row;
        w2_row += delta_w2_row;
    }
}

void clear_buffer(uint32_t color) {
    for (int i = 0; i < WIDTH * HEIGHT; i += 1)
        color_buffer[i] = color;
}

void render_buffer(void) {
    SDL_UpdateTexture(
        color_buffer_texture,
        NULL,
        color_buffer,
        (int) sizeof (uint32_t) * WIDTH
    );
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

bool setup(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

    window = SDL_CreateWindow(
        "TRIANGLE RASTERIZER",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        0
    );

    if (!window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error initializing SDL renderer.\n");
        return false;
    }

    color_buffer = (uint32_t *) malloc(sizeof (uint32_t) * WIDTH * HEIGHT);
    if (!color_buffer) {
        fprintf(stderr, "Error creating color buffer.\n");
        return false;
    }

    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH,
        HEIGHT
    );

    return true;
}

void process_input(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
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
}

void render(void) {
    clear_buffer(0xFF000000);

    vec2_t v0 = vertices[0];
    vec2_t v1 = vertices[1]; 
    vec2_t v2 = vertices[2];
    vec2_t v3 = vertices[3];
    
    fill_triangle(&v0, &v1, &v2);
    fill_triangle(&v3, &v2, &v1);

    render_buffer();
}

int main(void) {
    is_running = setup();

    while (is_running) {
        process_input();
        render();
    }

    return 0;
}
