#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h> // Search the SDL2 directory (system path) for SDL.h

#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "triangle.h"
#include "array.h"

//triangle_t triangles_to_render[N_MESH_FACES];
triangle_t *triangles_to_render = NULL;

vec3_t camera_position = { .x = 0, .y = 0, .z = 0 };

float fov_factor = 640;

bool is_running = false;
int previous_frame_time = 0;

void setup(void) {
    // Configure some render options
    display_mode = MODE_SOLIDWIRE;
    cull_backfaces = true;

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

    // Load the cube values in the mesh data structure
    // load_cube_mesh_data();
    load_obj_file_data("./assets/cube.obj");
}

void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
    case SDL_QUIT:
        is_running = false;
        break;
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            is_running = false; break;
        case SDLK_1:
            display_mode = MODE_WIREDOT; break;
        case SDLK_2: 
            display_mode = MODE_WIRE; break;
        case SDLK_3:
            display_mode = MODE_SOLID; break;
        case SDLK_4: 
            display_mode = MODE_SOLIDWIRE; break;
        case SDLK_c:
            cull_backfaces = true; break;
        case SDLK_d:
            cull_backfaces = false; break;
        }
        break;
    }
}

// Function that receives a 3D vector and returns a projected 2D point
vec2_t project(vec3_t point) {
    vec2_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z,
    };

    return projected_point;
}

void update(void) {
    // It's a black box, but it's preferable to just spinning the CPU uselessly
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    // Discouraged; use a proper delay function so the CPU can focus on
    // other tasks if needed
    // while (!SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME));

    previous_frame_time = SDL_GetTicks();

    // Initialize the array of triangles to render
    triangles_to_render = NULL;

    mesh.rotation.x += 0.005;
    mesh.rotation.y += 0.005;
    mesh.rotation.z += 0.005;

    // Loop all triangle faces
    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];
        
        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a-1];
        face_vertices[1] = mesh.vertices[mesh_face.b-1];
        face_vertices[2] = mesh.vertices[mesh_face.c-1];

        triangle_t projected_triangle;

        vec3_t transformed_vertices[3];

        // Loop all three vertices of this current face and apply transformations
        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];

            transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            // Translate the vertex away from the camera
            transformed_vertex.z += 5;

            // Save transformed vertex in the array of transformed vertices
            transformed_vertices[j] = transformed_vertex;
        }

        if (cull_backfaces) {
            // Check backface culling
            // Something I didn't realize earlier but is worth stating explicity:
            // Assume that A -> B -> C is a clockwise rotation
            vec3_t vector_a = transformed_vertices[0];
            vec3_t vector_b = transformed_vertices[1];
            vec3_t vector_c = transformed_vertices[2];

            // Get the vector subtraction of B - A, and C - A
            vec3_t vector_ab = vec3_sub(vector_b, vector_a);
            vec3_normalize(&vector_ab);
            vec3_t vector_ac = vec3_sub(vector_c, vector_a);
            vec3_normalize(&vector_ac);

            // Compute the face normal using the cross product
            // IMPORTANT: The order of vectors depends on the coordinate system handedness!
            vec3_t normal = vec3_cross(vector_ab, vector_ac);
            vec3_normalize(&normal);

            // Get the camera ray vector
            vec3_t camera_ray = vec3_sub(camera_position, vector_a);

            // Compute alignment of camera ray and face normal using the dot product
            float dot_normal_camera = vec3_dot(normal, camera_ray);

            // Bypass triangles looking away from the camera
            if (dot_normal_camera < 0) {
                continue;
            }
        }

        // Loop all three vertices to perform projection
        for (int j = 0; j < 3; j++) {
            // Project the current vertex
            vec2_t projected_point = project(transformed_vertices[j]);

            // Scale and translate projected point to the middle of the screen
            projected_point.x += (window_width / 2);
            projected_point.y += (window_height / 2);

            projected_triangle.points[j] = projected_point;
        }

        // Save the projected triangle in the array of triangles to render
        array_push(triangles_to_render, projected_triangle);
    }
}

void render(void) {
    draw_grid(30);

    // Loop all projected points and render them
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        triangle_t triangle = triangles_to_render[i];

        if (display_mode & MODE_DOT) {
            // Draw vertex points
            draw_rect(triangle.points[0].x, triangle.points[0].y, 4, 4, 0xFFFF0000);
            draw_rect(triangle.points[1].x, triangle.points[1].y, 4, 4, 0xFFFF0000);
            draw_rect(triangle.points[2].x, triangle.points[2].y, 4, 4, 0xFFFF0000);
        }

        if (display_mode & MODE_SOLID) {
            // Connect points in the triangle
            draw_filled_triangle(
                triangle.points[0].x,
                triangle.points[0].y,
                triangle.points[1].x,
                triangle.points[1].y,
                triangle.points[2].x,
                triangle.points[2].y,
                0xFF0000FF
            );
        }

        if (display_mode & MODE_WIRE) {
            draw_triangle(
                triangle.points[0].x,
                triangle.points[0].y,
                triangle.points[1].x,
                triangle.points[1].y,
                triangle.points[2].x,
                triangle.points[2].y,
                0xFF00FF00
            );
        }
    }

    // Clear the array of triangles to render every frame loop
    array_free(triangles_to_render);

    render_color_buffer();
    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

// Free any dynamically-allocated memory
void free_resources(void) {
    free(color_buffer);
    array_free(mesh.faces);
    array_free(mesh.vertices);
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
    free_resources();

    return 0;
}
