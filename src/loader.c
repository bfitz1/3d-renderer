#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mesh.h"

void parse_vertex(char *line) {
    // We know vertices have the format:
    // v <f> <f> <f> \0
    // After "v ", scan for " ", place a null terminator, read the float
    printf("v [ ");
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

void parse_face(char *line) {
    // We know faces have the format:
    // f d/d/d d/d/d d/d/d \0
    // We're only interested in the first number of each triple
    // After "f ", scan for "/", place a null terminator, read the decimal,
    // reset after " "

    printf("f [ ");
    char *sub = line + 2;
    char *ptr = strchr(sub, '/');
    while (ptr) {
        *ptr = 0;
        printf("%d ", atoi(sub));
        sub = strchr(ptr + 1, ' ') + 1;
        ptr = strchr(sub, '/');
    }
    printf("]\n");
}

void load_obj(char *path) {
    // For now:
    // 1. Open the file
    // 2. For each line
    // 2.1 Read line
    // 2.2 Prine line
    // 3. Close the file

    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "oh no file no open\n");
        exit(1);
    }

    char line[128] = {0};
    int i = 0;
    int c;
    while (i < 128-2) { // Reserve two spaces for my own use
        c = fgetc(file);
        if (c == '\n' || c == EOF) {
            line[i] = ' ';
            line[i+1] = '\0';
            if (strstr(line, "v ") == line) {
                parse_vertex(line);
            } else if (strstr(line, "f ") == line) {
                parse_face(line);
            }
            i = 0;
            if (c == EOF) break;
        } else {
            line[i++] = c;
        }            
    }

    if (i >= 128-2) {
        fprintf(stderr, "truncated the input; look into that\n");
        exit(1);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        load_obj("./assets/cube.obj");
    } else {
        load_obj(argv[1]);
    }

    return 0;
}
