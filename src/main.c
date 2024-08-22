#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h> // Search the SDL2 directory (system path) for SDL.h

#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "triangle.h"
#include "array.h"
#include "matrix.h"
#include "light.h"

triangle_t *triangles_to_render = NULL;

vec3_t camera_position = { .x = 0, .y = 0, .z = 0 };
mat4_t proj_matrix;

bool is_running = false;
int previous_frame_time = 0;

void setup(void) {
    // Configure some render options
    display_mode = MODE_SOLID;
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

    // Initialize the perspective matrix
    float fov = 60 * M_PI / 180;
    float aspect = (float)window_height / (float)window_width;    
    float znear = 0.1;
    float zfar = 100.0;
    proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);

    // Load the cube values in the mesh data structure
    // load_cube_mesh_data();
    load_obj_file_data("./assets/f22.obj");
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

    mesh.rotation.x += 0.01;
    // mesh.rotation.y += 0.01;
    // mesh.rotation.z += 0.01;
    // mesh.scale.x += 0.002;
    // mesh.scale.y += 0.001;
    // mesh.translation.x += 0.01;
    mesh.translation.z = 5.0;

    // Create a scale and translation matrix that will be used to multiply the mesh vertices
    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);
    mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);

    // Create a world matrix combining scale, rotation, and translation matrices
    // Using matrices also means we can lift these computations outside of the loop
    mat4_t world_matrix = mat4_identity();
    world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
    world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
    world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
    world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
    world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

    // Loop all triangle faces
    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];
        
        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a-1];
        face_vertices[1] = mesh.vertices[mesh_face.b-1];
        face_vertices[2] = mesh.vertices[mesh_face.c-1];

        vec4_t transformed_vertices[3];

        // Loop all three vertices of this current face and apply transformations
        for (int j = 0; j < 3; j++) {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            // Multiply the world matrix by the original vector
            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

            // Save transformed vertex in the array of transformed vertices
            transformed_vertices[j] = transformed_vertex;
        }

        // Check backface culling
        // Something I didn't realize earlier but is worth stating explicity:
        // Assume that A -> B -> C is a clockwise rotation
        vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
        vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);
        vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);

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

        if (cull_backfaces) {
            // Bypass triangles looking away from the camera
            if (dot_normal_camera < 0) {
                continue;
            }
        }

        vec4_t projected_points[3];

        // Loop all three vertices to perform projection
        for (int j = 0; j < 3; j++) {
            // Project the current vertex
            projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);

            // Scale into the view
            projected_points[j].x *= (window_width / 2.);
            projected_points[j].y *= (window_height / 2.);

            // Translate projected point to the middle of the screen
            projected_points[j].x += (window_width / 2.);
            projected_points[j].y += (window_height / 2.);
        }

        // Calculate the color intensity based on (inverted) light sources and face normals
        float dot_normal_light = vec3_dot(normal, light.direction);
        float intensity = -dot_normal_light;
        uint32_t color = light_apply_intensity(mesh_face.color, intensity);

        // Calculate the average depth for each face based on the vertices
        // after transformation
        float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;

        triangle_t projected_triangle = {
            .points = {
                { projected_points[0].x, projected_points[0].y },
                { projected_points[1].x, projected_points[1].y },
                { projected_points[2].x, projected_points[2].y },
            },
            .color = color,
            .avg_depth = avg_depth,
        };

        // Save the projected triangle in the array of triangles to render
        array_push(triangles_to_render, projected_triangle);
    }

    // Sort by avg_depth in descending order
    for (int i = 1; i < array_length(triangles_to_render); i++) {
        for (int j = i; j > 0; j--) {
            if (triangles_to_render[j].avg_depth > triangles_to_render[j-1].avg_depth) {
                triangle_t temp = triangles_to_render[j];
                triangles_to_render[j] = triangles_to_render[j-1];
                triangles_to_render[j-1] = temp;
            }
        }
    }
}

void render(void) {
    SDL_RenderClear(renderer);

    draw_grid(30);

    // Loop all projected points and render them
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++) {
        triangle_t triangle = triangles_to_render[i];

        if (display_mode & MODE_DOT) {
            // Draw vertex points
            draw_rect(triangle.points[0].x, triangle.points[0].y, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[1].x, triangle.points[1].y, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[2].x, triangle.points[2].y, 6, 6, 0xFFFF0000);
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
                triangle.color
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
                0xFFFFFFFF
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
