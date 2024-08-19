#include <math.h>
#include "matrix.h"
#include "vector.h"

mat4_t mat4_identity(void) {
    return (mat4_t) {{
        { 1, 0, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 0, 1, 0 },
        { 0, 0, 0, 1 },
    }};
}

mat4_t mat4_make_scale(float sx, float sy, float sz) {
    mat4_t m = mat4_identity();
    m.m[0][0] = sx;
    m.m[1][1] = sy;
    m.m[2][2] = sz;
    return m;
}

mat4_t mat4_make_translation(float tx, float ty, float tz) {
    mat4_t m = mat4_identity();
    m.m[0][3] = tx;
    m.m[1][3] = ty;
    m.m[2][3] = tz;
    return m;
}

mat4_t mat4_make_rotation_x(float angle) {
    float c = cos(angle);
    float s = sin(angle);

    mat4_t m = mat4_identity();
    m.m[1][1] = c;
    m.m[2][1] = s;
    m.m[1][2] = -s;
    m.m[2][2] = c;
    return m;
}

mat4_t mat4_make_rotation_y(float angle) {
    float c = cos(angle);
    float s = sin(angle);

    mat4_t m = mat4_identity();
    m.m[0][0] = c;
    m.m[2][0] = -s;
    m.m[0][2] = s;
    m.m[2][2] = c;
    return m;
}

mat4_t mat4_make_rotation_z(float angle) {
    float c = cos(angle);
    float s = sin(angle);

    mat4_t m = mat4_identity();
    m.m[0][0] = c;
    m.m[1][0] = s;
    m.m[0][1] = -s;
    m.m[1][1] = c;
    return m;
}

vec4_t mat4_mul_vec4(mat4_t m, vec4_t v) {
    vec4_t result;
    result.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w;
    result.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w;
    result.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w;
    result.w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w;
    return result;
}

mat4_t mat4_mul_mat4(mat4_t a, mat4_t b) {
    mat4_t m = {0};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                m.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return m;
}
