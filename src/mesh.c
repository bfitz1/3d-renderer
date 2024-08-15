#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mesh.h"
#include "array.h"

mesh_t mesh = {
    .vertices = NULL,
    .faces = NULL,
    .rotation = { .x = 0, .y = 0, .z = 0 },
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
    { .a = 1, .b = 2, .c = 3 },
    { .a = 1, .b = 3, .c = 4 },
    // right
    { .a = 4, .b = 3, .c = 5 },
    { .a = 4, .b = 5, .c = 6 },
    // back
    { .a = 6, .b = 5, .c = 7 },
    { .a = 6, .b = 7, .c = 8 },
    // left
    { .a = 8, .b = 7, .c = 2 },
    { .a = 8, .b = 2, .c = 1 },
    // top
    { .a = 2, .b = 7, .c = 5 },
    { .a = 2, .b = 5, .c = 3 },
    // bottom
    { .a = 6, .b = 8, .c = 1 },
    { .a = 6, .b = 1, .c = 4 },
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
    // To be moved over to our vec3_t struct
    float vertex_data[3];
    int i = 0;

    // We expect a line in the form of:
    // v f f f \0
    // We can carve out "f" using strchr, convert to float, and store
    // in an array.
    char *sub = line + 2;
    char *ptr = strchr(sub, ' ');
    while (ptr) {
        *ptr = 0;
        vertex_data[i++] = atof(sub);
        sub = ptr + 1;
        ptr = strchr(sub, ' ');
    }
    return (vec3_t) {
        .x = vertex_data[0],
        .y = vertex_data[1],
        .z = vertex_data[2],
    };
}

static face_t obj_file_parse_face(char *line) {
    // To be moved over to our face_t struct
    int face_data[3];
    int i = 0;

    // We expect a line in the form of:
    // f d/d/d d/d/d d/d/d \0
    // We can carve out the first "d" of each group using strchr,
    // convert to integer, and store in an array.
    char *sub = line + 2;
    char *ptr = strchr(sub, '/');
    while (ptr) {
        *ptr = 0;
        face_data[i++] = atoi(sub);
        sub = strchr(ptr + 1, ' ') + 1;
        ptr = strchr(sub, '/');
    }

    return (face_t) {
        .a = face_data[0],
        .b = face_data[1],
        .c = face_data[2],
    };
}

// Read the contents of an .obj file into mesh.vertices and mesh.faces
void load_obj_file_data(char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Problem opening %s\n", filename);
        return;
    }

    const int LINE_MAX = 128; // Hopefully this is enough!
    char line[LINE_MAX] = {0};
    int i = 0;
    int c;
    // I forgot about fgets lmao
    while (i < LINE_MAX-2) { // Reserve two spaces for ' ' and '\0' at the end
        c = fgetc(file);
        if (c == '\n' || c == EOF) {
            line[i] = ' ';
            line[i+1] = '\0';
            // Works the same, but using strncmp for this check is
            // probably clearer
            if (strstr(line, "v ") == line) {
                // I opted for my own parsing function because everyone's
                // scared of sscanf
                array_push(mesh.vertices, obj_file_parse_vertex(line));
            } else if (strstr(line, "f ") == line) {
                array_push(mesh.faces, obj_file_parse_face(line));
            }
            i = 0;
            if (c == EOF) break;
        } else {
            line[i++] = c;
        }            
    }

    if (i >= LINE_MAX-2) {
        fprintf(stderr, "File input was truncated, aborting");
    }

    fclose(file);
}
