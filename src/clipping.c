#include <math.h>

#include "clipping.h"

#define NUM_PLANES 6
plane_t frustum_planes[NUM_PLANES];

void init_frustum_planes(float fov, float znear, float zfar) {
    frustum_planes[LEFT_FRUSTUM_PLANE] = (plane_t) {
        .point = { .x = 0, .y = 0, .z = 0 },
        .normal = { .x = cos(fov/2), .y = 0, .z = sin(fov/2) },
    };

    frustum_planes[RIGHT_FRUSTUM_PLANE] = (plane_t) {
        .point = { .x = 0, .y = 0, .z = 0 },
        .normal = { .x = -cos(fov/2), .y = 0, .z = sin(fov/2) },
    };

    frustum_planes[TOP_FRUSTUM_PLANE] = (plane_t) {
        .point = { .x = 0, .y = 0, .z = 0 },
        .normal = { .x = 0, .y = -cos(fov/2), .z = sin(fov/2) },
    };

    frustum_planes[BOTTOM_FRUSTUM_PLANE] = (plane_t ){
        .point = { .x = 0, .y = 0, .z = 0 },
        .normal = { .x = 0, .y = cos(fov/2), .z = sin(fov/2) },
    };

    frustum_planes[NEAR_FRUSTUM_PLANE] = (plane_t) {
        .point = { .x = 0, .y = 0, .z = znear },
        .normal = { .x = 0, .y = 0, .z = 1 },
    };

    frustum_planes[FAR_FRUSTUM_PLANE] = (plane_t) {
        .point = { .x = 0, .y = 0, .z = zfar },
        .normal = { .x = 0, .y = 0, .z = -1 },
    };
}
