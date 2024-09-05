#include <math.h>

#include "vector.h"
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

polygon_t create_polygon_from_triangle(vec3_t v0, vec3_t v1, vec3_t v2) {
    polygon_t polygon = {
        .vertices = { v0, v1, v2 },
        .num_vertices = 3,
    };

    return polygon;
}

void clip_polygon_against_plane(polygon_t *polygon, int plane) {
    vec3_t plane_point = frustum_planes[plane].point;
    vec3_t plane_normal = frustum_planes[plane].normal;

    // The array of inside vertices that will be part of the final polygon returned via parameter
    vec3_t inside_vertices[MAX_NUM_POLY_VERTICES];
    int num_inside_vertices = 0;

    // Start current and previous vertex with the first and last polygon vertices
    vec3_t *current_vertex = &polygon->vertices[0];
    vec3_t *previous_vertex = &polygon->vertices[polygon->num_vertices - 1];

    // Compute the dot product to determine which partition the vertex exists in
    float current_dot = 0;
    float previous_dot = vec3_dot(vec3_sub(*previous_vertex, plane_point), plane_normal);

    // Loop while the current vertex is different than the last vertex
    while (current_vertex != &polygon->vertices[polygon->num_vertices]) {
        current_dot = vec3_dot(vec3_sub(*current_vertex, plane_point), plane_normal);

        // If we changed from inside to outside or vice-versa
        if (current_dot * previous_dot < 0) {
            // Calculate the interpolation factor, t = dotQ1 / (dotQ1 - dotQ2)
            float t = previous_dot / (previous_dot - current_dot);
            // Calculate the intersection point, I = Q1 + t(Q2 - Q1)
            vec3_t intersection_point = vec3_clone(current_vertex);
            intersection_point = vec3_sub(intersection_point, *previous_vertex); // Hey wait a minute!
            intersection_point = vec3_mul(intersection_point, t);
            intersection_point = vec3_add(intersection_point, *previous_vertex); 
            // Insert the new intersection point in the list of "inside vertices"
            inside_vertices[num_inside_vertices] = vec3_clone(&intersection_point);
            num_inside_vertices += 1;
        }

        // If current point is inside the plane
        if (current_dot > 0) {
            // Insert current vertex in the list of "inside vertices"
            inside_vertices[num_inside_vertices] = vec3_clone(current_vertex);
            num_inside_vertices += 1;
        }

        // Move to the next vertex
        previous_dot = current_dot;
        previous_vertex = current_vertex;
        current_vertex += 1;
    }

    // Copy all the vertices from the inside_vertices into the destination polygon (out parameter)
    for (int i = 0; i < num_inside_vertices; i += 1) {
        polygon->vertices[i] = vec3_clone(&inside_vertices[i]);
    } 
    polygon->num_vertices = num_inside_vertices;
}

void clip_polygon(polygon_t *polygon) {
    clip_polygon_against_plane(polygon, LEFT_FRUSTUM_PLANE);
    clip_polygon_against_plane(polygon, RIGHT_FRUSTUM_PLANE);
    clip_polygon_against_plane(polygon, TOP_FRUSTUM_PLANE);
    clip_polygon_against_plane(polygon, BOTTOM_FRUSTUM_PLANE);
    clip_polygon_against_plane(polygon, NEAR_FRUSTUM_PLANE);
    clip_polygon_against_plane(polygon, FAR_FRUSTUM_PLANE);
}
