#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER_SIZE 512

void parse_vertex(char *line) {
    // We know vertices have the format:
    // v <f> <f> <f> \0
    // After "v ", scan for " ", place a null terminator, read the float
    printf("v=[ ");
    char *sub = line + 2;
    char *ptr = strchr(sub, ' ');
    while (ptr) {
        *ptr = 0;
        printf("%f ", atof(sub));
        sub = ptr + 1;
        ptr = strchr(sub, ' ');
    }
    printf("]\n");
}

void parse_texture_coordinate(char *line) {
    // We know texture coordinates have the format:
    // vt f f f \0
    printf("vt=[ ");
    char *sub = line + 3;
    char *ptr = strchr(sub, ' ');
    while (ptr) {
        *ptr = 0;
        printf("%f ", atof(sub));
        sub = ptr + 1;
        ptr = strchr(sub, ' ');
    }
    printf("]\n");
}

void parse_face(char *line) {
    // Big change! Be able to handle four possible formats:
    // - Case 1: Vertex faces only
    //   f v1 v2 v3 \0
    // - Case 2: Vertex faces and textures
    //   f v1/vt1 v2/vt2 v3/vt3 \0
    // - Case 3: Vertex faces, textures and normals
    //   f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 \0
    // - Case 4: Vertices and Normals
    //   f v1//vn1 v2//vn2 v3//vn3 \0
    //

    // We're only interested in the first number of each triple
    // After "f ", scan for "/", place a null terminator, read the decimal,
    // reset after " "

    printf("f=[ ");
    char *sub = line + 2;
    char *ptr = strchr(sub, ' ');
    int i = 1;
    while (ptr) {
        *ptr = 0;
        char *s = strchr(sub, '/');
        if (s == NULL) { // Case 1
            printf("v%d=%d ", i, atoi(sub));
        } else if (strchr(s + 1, '/') == NULL) { // Case 2
            *s = 0;
            printf("v%d=%d ", i, atoi(sub));
            sub = s + 1;
            printf("vt%d=%d ", i, atoi(sub));
        } else if (strchr(s + 1, '/') == s + 1) { // Case 4
            *s = 0;
            printf("v%d=%d ", i, atoi(sub));
            sub = s + 2;
            printf("vn%d=%d ", i, atoi(sub));
        } else { // Case 3
            *s = 0;
            printf("v%d=%d ", i, atoi(sub));
            sub = s + 1;
            s = strchr(sub, '/');
            *s = 0;
            printf("vt%d=%d ", i, atoi(sub));
            sub = s + 1;
            printf("vn%d=%d ", i, atoi(sub));
        }
        sub = ptr + 1;
        ptr = strchr(sub, ' ');
        i += 1;
    }
    printf("]\n");
}

void load_obj(char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "oh no file no open\n");
        exit(1);
    }

    char line[MAX_BUFFER_SIZE] = {0};

    char *result = fgets(line, MAX_BUFFER_SIZE-2, file);
    while (result) {
        int len = strlen(line);
        line[len] = ' ';
        line[len + 1] = '\0';

        if (strncmp(line, "v ", 2) == 0)  parse_vertex(line);
        if (strncmp(line, "vt ", 3) == 0) parse_texture_coordinate(line);
        if (strncmp(line, "f ", 2) == 0)  parse_face(line);

        result = fgets(line, MAX_BUFFER_SIZE-2, file);
    }

    fclose(file);
}

int not_main(int argc, char *argv[]) {
    if (argc < 2) {
        load_obj("./assets/cube.obj");
    } else {
        load_obj(argv[1]);
    }

    return 0;
}
