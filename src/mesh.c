#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mesh.h"
#include "triangle.h"
#include "array.h"
#include "texture.h"

#define MAX_BUFFER_SIZE 512

#define MAX_NUMBER_MESHES 10
static mesh_t meshes[MAX_NUMBER_MESHES];
static int mesh_count = 0;

void load_mesh(char *obj_filename, char *png_filename, vec3_t scale, vec3_t translation, vec3_t rotation) {
    load_mesh_obj_data(&meshes[mesh_count], obj_filename);
    load_mesh_png_data(&meshes[mesh_count], png_filename);
    meshes[mesh_count].scale = scale;
    meshes[mesh_count].translation = translation;
    meshes[mesh_count].rotation = rotation;
    mesh_count += 1;
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
void load_mesh_obj_data(mesh_t *mesh, char *obj_filename) {
    FILE *file = fopen(obj_filename, "r");
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
            array_push(mesh->vertices, obj_file_parse_vertex(line));
        if (strncmp(line, "vt ", 3) == 0)
            array_push(texture_coordinates, obj_file_parse_texture_coordinate(line));
        if (strncmp(line, "f ", 2) == 0)
            array_push(mesh->faces, obj_file_parse_face(line, texture_coordinates));

        result = fgets(line, MAX_BUFFER_SIZE-2, file);
    }

    array_free(texture_coordinates);
    fclose(file);
}

void load_mesh_png_data(mesh_t *mesh, char *png_filename) {
    upng_t *png_image = upng_new_from_file(png_filename);
    if (png_image != NULL) {
        upng_decode(png_image);
        if (upng_get_error(png_image) == UPNG_EOK) {
            mesh->texture = png_image;
        }
    }
}

int get_num_meshes(void) {
    return mesh_count;
}

mesh_t *get_mesh(int index) {
    return &meshes[index];
}

void free_meshes(void) {
    for (int i = 0; i < mesh_count; i += 1) {
        upng_free(meshes[i].texture);
        array_free(meshes[i].faces);
        array_free(meshes[i].vertices);
    }
}
