#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mesh.h"
#include "triangle.h"
#include "array.h"
#include "texture.h"

#define MAX_BUFFER_SIZE 512

mesh_t mesh = {
    .vertices = NULL,
    .faces = NULL,
    .rotation = { .x = 0, .y = 0, .z = 0 },
    .scale = { 1.0, 1.0, 1.0 },
    .translation = { 0, 0, 0 },
};

vec3_t cube_vertices[N_CUBE_VERTICES] = {
    { .x = -1, .y = -1, .z = -1 }, // 1
    { .x = -1, .y =  1, .z = -1 }, // 2
    { .x =  1, .y =  1, .z = -1 }, // 3
    { .x =  1, .y = -1, .z = -1 }, // 4
    { .x =  1, .y =  1, .z =  1 }, // 5
    { .x =  1, .y = -1, .z =  1 }, // 6
    { .x = -1, .y =  1, .z =  1 }, // 7
    { .x = -1, .y = -1, .z =  1 }, // 8
};

face_t cube_faces[N_CUBE_FACES] = {
    // front
    { .a = 1, .b = 2, .c = 3, .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 }, .color = 0xFFFF0000 },
    { .a = 1, .b = 3, .c = 4, .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 }, .color = 0xFFFF0000 },
    // right
    { .a = 4, .b = 3, .c = 5, .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 }, .color = 0xFF00FF00 },
    { .a = 4, .b = 5, .c = 6, .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 }, .color = 0xFF00FF00 },
    // back
    { .a = 6, .b = 5, .c = 7, .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 }, .color = 0xFF0000FF },
    { .a = 6, .b = 7, .c = 8, .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 }, .color = 0xFF0000FF },
    // left
    { .a = 8, .b = 7, .c = 2, .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 }, .color = 0xFFFFFF00 },
    { .a = 8, .b = 2, .c = 1, .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 }, .color = 0xFFFFFF00 },
    // top
    { .a = 2, .b = 7, .c = 5, .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 }, .color = 0xFFFF00FF },
    { .a = 2, .b = 5, .c = 3, .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 }, .color = 0xFFFF00FF },
    // bottom
    { .a = 6, .b = 8, .c = 1, .a_uv = { 0, 1 }, .b_uv = { 0, 0 }, .c_uv = { 1, 0 }, .color = 0xFF00FFFF },
    { .a = 6, .b = 1, .c = 4, .a_uv = { 0, 1 }, .b_uv = { 1, 0 }, .c_uv = { 1, 1 }, .color = 0xFF00FFFF },
};

void load_cube_mesh_data(void) {
    for (int i = 0; i < N_CUBE_VERTICES; i++) {
        vec3_t cube_vertex = cube_vertices[i];
        array_push(mesh.vertices, cube_vertex)
    }
    for (int i = 0; i < N_CUBE_FACES; i++) {
        face_t cube_face = cube_faces[i];
        array_push(mesh.faces, cube_face);
    }
}

static vec3_t obj_file_parse_vertex(char *line) {
    float vertex_indices[3];
    int i = 0;

    // Vertex data should have the form
    // v <float> <float> <float> 
    char *sub = line + 2;
    char *ptr = strchr(sub, ' ');
    while (ptr) {
        *ptr = 0;
        vertex_indices[i] = atof(sub);

        sub = ptr + 1;
        ptr = strchr(sub, ' ');
        i += 1;
    }

    return (vec3_t) {
        .x = vertex_indices[0],
        .y = vertex_indices[1],
        .z = vertex_indices[2],
    };
}

static tex2_t obj_file_parse_texture_coordinate(char *line) {
    float texture_indices[2];
    int i = 0;

    // Texture coordinate data should have the form
    // vt <float> <float>
    char *sub = line + 3;
    char *ptr = strchr(sub, ' ');
    while (ptr) {
        *ptr = 0;
        texture_indices[i] = atof(sub);

        sub = ptr + 1;
        ptr = strchr(sub, ' ');
        i += 1;
    }

    return (tex2_t) {
        .u = texture_indices[0],
        .v = texture_indices[1],
    };
}

static face_t obj_file_parse_face(char *line, tex2_t *texture_coordinates) {
    int vertex_indices[3];
    int texture_indices[3];
    int normal_indices[3];
    int i = 0;

    // There are four possible formats for face elements
    // C1 - Vertex indices:
    //      f <int> <int> <int>
    // C2 - Vertex texture coordinate indices:
    //      f <int>/<int> <int>/<int> <int>/<int>
    // C3 - Vertex normal indices:
    //      f <int>/<int>/<int> <int>/<int>/<int> <int>/<int>/<int>
    // C4 - Vertex normal indices without texture coordinate indices
    //      f <int>//<int> <int>//<int> <int>//<int>

    char *sub = line + 2;
    char *ptr = strchr(sub, ' ');
    while (ptr) {
        *ptr = 0;

        char *s = strchr(sub, '/');
        if (s == NULL) { // C1
            vertex_indices[i] = atoi(sub);
        } else if (strchr(s + 1, '/') == NULL) { // C2
            *s = 0;
            vertex_indices[i] = atoi(sub);
            sub = s + 1;
            texture_indices[i] = atoi(sub);
        } else if (strchr(s + 1, '/') == s + 1) { // C4
            *s = 0;
            vertex_indices[i] = atoi(sub);
            sub = s + 2;
            normal_indices[i] = atoi(sub);
        } else { // C3
            *s = 0;
            vertex_indices[i] = atoi(sub);
            sub = s + 1;
            s = strchr(sub, '/');
            *s = 0;
            texture_indices[i] = atoi(sub);
            sub = s + 1;
            normal_indices[i] = atoi(sub);
        }

        sub = ptr + 1;
        ptr = strchr(sub, ' ');
        i += 1;
    }

    return (face_t) {
        .a = vertex_indices[0] - 1,
        .b = vertex_indices[1] - 1,
        .c = vertex_indices[2] - 1,
        .a_uv = texture_coordinates[texture_indices[0] - 1],
        .b_uv = texture_coordinates[texture_indices[1] - 1],
        .c_uv = texture_coordinates[texture_indices[2] - 1],
        .color = 0xFFFFFFFF,
    };
}

// Read the contents of an .obj file into mesh.vertices and mesh.faces
void load_obj_file_data(char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "oh no file no open\n");
        exit(1);
    }

    char line[MAX_BUFFER_SIZE] = {0};
    tex2_t *texture_coordinates = NULL;

    char *result = fgets(line, MAX_BUFFER_SIZE-2, file);
    while (result) {
        int len = strlen(line);
        line[len] = ' ';
        line[len + 1] = '\0';

        if (strncmp(line, "v ", 2) == 0) 
            array_push(mesh.vertices, obj_file_parse_vertex(line));
        if (strncmp(line, "vt ", 3) == 0)
            array_push(texture_coordinates, obj_file_parse_texture_coordinate(line));
        if (strncmp(line, "f ", 2) == 0)
            array_push(mesh.faces, obj_file_parse_face(line, texture_coordinates));

        result = fgets(line, MAX_BUFFER_SIZE-2, file);
    }

    array_free(texture_coordinates);
    fclose(file);
}
