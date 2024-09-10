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
#include "texture.h"
#include "upng.h"
#include "camera.h"
#include "clipping.h"

#define MAX_TRIANGLES_PER_MESH 10000
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
int num_triangles_to_render = 0;

// Transformation matrices
mat4_t world_matrix;
mat4_t proj_matrix;
mat4_t view_matrix;

bool is_running = false;
int previous_frame_time = 0;
float delta_time = 0;

void setup(char *model, char *texture) {
    // Configure some render options
    set_render_mode(MODE_TEXTURE);
    set_cull_backfaces(true);
    set_show_depth(false);

    // Initialize the light source
    init_light(vec3_new(0, 0, 1));

    // Initialize the camera
    // I like how less wordy this is, but there is the risk of overly
    // relying on default behavior i.e I'm assuming unspecified struct
    // members are zeros.
    init_camera((camera_t) { 
        .direction = vec3_new(0, 0, 1),
    });

    // Initialize the perspective matrix
    float aspecty = (float)get_window_height() / get_window_width();    
    float aspectx = (float)get_window_width() / get_window_height();    
    float fovy = 60 * M_PI / 180; // 60deg, or pi/3
    float fovx = atan(tan(fovy / 2) * aspectx) * 2.0;
    float znear = 1.0;
    float zfar = 20.0;
    proj_matrix = mat4_make_perspective(fovy, aspecty, znear, zfar);

    // Initialize frustum planes with a point and a normal
    init_frustum_planes(fovy, fovx, znear, zfar);

    // Manually load the hardcoded texture data from the static array
    // mesh_texture = (uint32_t *)REDBRICK_TEXTURE;

    // Load the cube values in the mesh data structure
    // load_cube_mesh_data();
    load_obj_file_data(model);

    // Load the texture information from an external PNG file
    load_png_texture_data(texture);
}

void process_input(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            // Process controls
            case SDLK_ESCAPE:
                is_running = false; break;
            // Display modes
            case SDLK_1:
                set_render_mode(MODE_WIREDOT); break;
            case SDLK_2: 
                set_render_mode(MODE_WIRE); break;
            case SDLK_3:
                set_render_mode(MODE_SOLID); break;
            case SDLK_4: 
                set_render_mode(MODE_SOLIDWIRE); break;
            case SDLK_5:
                set_render_mode(MODE_TEXTURE); break;
            case SDLK_6:
                set_render_mode(MODE_TEXTUREWIRE); break;
            // Display options
            case SDLK_c:
                toggle_cull_backfaces(); break;
            case SDLK_z:
                toggle_show_depth(); break;
            // Camera movement controls
            case SDLK_w:
                set_camera_forward_velocity(vec3_mul(get_camera_direction(), 5*delta_time));
                set_camera_position(vec3_add(get_camera_position(), get_camera_forward_velocity()));
                break;
            case SDLK_s:
                set_camera_forward_velocity(vec3_mul(get_camera_direction(), 5*delta_time));
                set_camera_position(vec3_sub(get_camera_position(), get_camera_forward_velocity()));
                break;
            case SDLK_a:
                set_camera_yaw(get_camera_yaw() - 1*delta_time);
                break;
            case SDLK_d:
                set_camera_yaw(get_camera_yaw() + 1*delta_time);
                break;
            case SDLK_i:
                set_camera_pitch(get_camera_pitch() + 1*delta_time);
                break;
            case SDLK_k:
                set_camera_pitch(get_camera_pitch() - 1*delta_time);
                break;
            case SDLK_UP: 
                set_camera_position(vec3_add(get_camera_position(), vec3_new(0, 3*delta_time, 0)));
                break;
            case SDLK_DOWN: 
                set_camera_position(vec3_sub(get_camera_position(), vec3_new(0, 3*delta_time, 0)));
                break;
            }
            break;
        }
    }
}

void update(void) {
    // It's a black box, but it's preferable to just spinning the CPU uselessly
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    // Get a delta time factor converted to seconds to be used to update our game objects
    delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0;

    // Discouraged; use a proper delay function so the CPU can focus on
    // other tasks if needed
    // while (!SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME));

    previous_frame_time = SDL_GetTicks();

    // Initialize the counter of triangles to render for the current frame
    num_triangles_to_render = 0;

    mesh.rotation.x += 0.6 * delta_time;
    // mesh.rotation.y += 0.6 * delta_time;;
    // mesh.rotation.z += 0.5 * delta_time;;
    // mesh.scale.x += 0.002 * delta_time;;
    // mesh.scale.y += 0.001 * delta_time;;
    // mesh.translation.x += 0.01 * delta_time;;
    mesh.translation.z = 5.0;

    // Create a scale and translation matrix that will be used to multiply the mesh vertices
    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);
    mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);

    // Compute the camera direction to determine the target point
    mat4_t camera_rotation = mat4_identity();
    camera_rotation = mat4_mul_mat4(mat4_make_rotation_x(get_camera_pitch()), camera_rotation);
    camera_rotation = mat4_mul_mat4(mat4_make_rotation_y(get_camera_yaw()), camera_rotation);
    set_camera_direction(vec3_from_vec4(mat4_mul_vec4(camera_rotation, vec4_from_vec3(vec3_new(0, 0, 1)))));

    // Offset the camera position in the direction where the camera is pointing at
    vec3_t target = vec3_add(get_camera_position(), get_camera_direction());
    
    // Create the view matrix looking at a hard-coded target point
    vec3_t up_direction = { 0, 1, 0 };

    mat4_t view_matrix = mat4_look_at(get_camera_position(), target, up_direction);

    // Create a world matrix combining scale, rotation, and translation matrices
    // Using matrices also means we can lift these computations outside of the loop
    world_matrix = mat4_identity();
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
        face_vertices[0] = mesh.vertices[mesh_face.a];
        face_vertices[1] = mesh.vertices[mesh_face.b];
        face_vertices[2] = mesh.vertices[mesh_face.c];

        vec4_t transformed_vertices[3];

        // Loop all three vertices of this current face and apply transformations
        for (int j = 0; j < 3; j++) {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            // Apply the world matrix to the original vector
            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

            // Apply the view matrix to the transformed vector
            transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

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
        vec3_t origin = { 0, 0, 0 };
        vec3_t camera_ray = vec3_sub(origin, vector_a);

        // Compute alignment of camera ray and face normal using the dot product
        float dot_normal_camera = vec3_dot(normal, camera_ray);

        if (get_cull_backfaces()) {
            // Bypass triangles looking away from the camera
            if (dot_normal_camera < 0) {
                continue;
            }
        }

        // Create a polygon from the orignal transformed triangle to be clipped
        polygon_t polygon = create_polygon_from_triangle(
            vec3_from_vec4(transformed_vertices[0]),
            vec3_from_vec4(transformed_vertices[1]),
            vec3_from_vec4(transformed_vertices[2]),
            mesh_face.a_uv,
            mesh_face.b_uv,
            mesh_face.c_uv
        );

        // Clip the polygon (in place) and return a new polygon with potential new vertices
        clip_polygon(&polygon);

        // Break the clipped polygon into triangles
        triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
        int num_triangles_after_clipping = 0;

        triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

        // Loops all the assembled triangles after clipping
        for (int t = 0; t < num_triangles_after_clipping; t += 1) {
            triangle_t triangle_after_clipping = triangles_after_clipping[t];

            vec4_t projected_points[3];

            // Loop all three vertices to perform projection
            for (int j = 0; j < 3; j++) {
                // Project the current vertex
                projected_points[j] = mat4_mul_vec4_project(proj_matrix, triangle_after_clipping.points[j]);

                // Scale into the view
                projected_points[j].x *= (get_window_width() / 2.);
                projected_points[j].y *= (get_window_height() / 2.);

                // Invert the y values to account for flipped screen y-coordinates
                projected_points[j].y *= -1;

                // Translate projected point to the middle of the screen
                projected_points[j].x += (get_window_width() / 2.);
                projected_points[j].y += (get_window_height() / 2.);
            }

            // Calculate the color intensity based on (inverted) light sources and face normals
            float dot_normal_light = vec3_dot(normal, get_light_direction()); 
            float intensity = -dot_normal_light;
            uint32_t color = light_apply_intensity(mesh_face.color, intensity);

            triangle_t triangle_to_render = {
                .points = {
                    { projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w },
                    { projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w },
                    { projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w },
                },
                .texcoords = {
                    { triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v },
                    { triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v },
                    { triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v }
                },
                .color = color,
            };

            // Save the projected triangle in the array of triangles to render
            if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH) {
                triangles_to_render[num_triangles_to_render] = triangle_to_render;
                num_triangles_to_render += 1;
            }
        }
    }
}

void render(void) {
    clear_color_buffer(0xFF000000);
    clear_z_buffer();

    draw_checker(180 / 4 /* GCD scaled down */);

    // Loop all projected points and render them
    for (int i = 0; i < num_triangles_to_render; i++) {
        triangle_t triangle = triangles_to_render[i];

        if (get_render_mode() & MODE_SOLID) {
            // Connect points in the triangle
            draw_filled_triangle(
                triangle.points[0].x,
                triangle.points[0].y,
                triangle.points[0].z,
                triangle.points[0].w,
                triangle.points[1].x,
                triangle.points[1].y,
                triangle.points[1].z,
                triangle.points[1].w,
                triangle.points[2].x,
                triangle.points[2].y,
                triangle.points[2].z,
                triangle.points[2].w,
                triangle.color
            );
        }

        if (get_render_mode() & MODE_TEXTURE) {
            draw_textured_triangle(
                // P0
                triangle.points[0].x,
                triangle.points[0].y,
                triangle.points[0].z,
                triangle.points[0].w,
                triangle.texcoords[0].u,
                triangle.texcoords[0].v,
                // P1
                triangle.points[1].x,
                triangle.points[1].y,
                triangle.points[1].z,
                triangle.points[1].w,
                triangle.texcoords[1].u,
                triangle.texcoords[1].v,
                // P2
                triangle.points[2].x,
                triangle.points[2].y,
                triangle.points[2].z,
                triangle.points[2].w,
                triangle.texcoords[2].u,
                triangle.texcoords[2].v,
                // Texture
                mesh_texture
            );
        }

        if (get_render_mode() & MODE_WIRE) {
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

        if (get_render_mode() & MODE_DOT) {
            // Draw vertex points
            draw_rect(triangle.points[0].x, triangle.points[0].y, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[1].x, triangle.points[1].y, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[2].x, triangle.points[2].y, 6, 6, 0xFFFF0000);
        }

    }

    if (get_show_depth()) {
        render_z_buffer();
    } else {
        render_color_buffer();
    }
}

// Free any dynamically-allocated memory
void free_resources(void) {
    upng_free(png_texture);
    array_free(mesh.faces);
    array_free(mesh.vertices);
}

int main(int argc, char *argv[]) {
    char *model = "./assets/f22.obj";
    char *texture = "./assets/f22.png";

    if (argc == 3) {
        model = argv[1];
        texture = argv[2];
    }

    is_running = initialize_window();

    setup(model, texture);

    while (is_running) {
        process_input();
        update();
        render();
    }
    
    destroy_window();
    free_resources();

    return 0;
}
