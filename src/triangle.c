#include <math.h>
#include "display.h"
#include "triangle.h"
#include "swap.h"

vec3_t get_triangle_normal(vec4_t transformed_vertices[3]) {
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

    return normal;
}

vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
    // Find the vectors betweenthe vertices ABC and point p
    vec2_t ac = vec2_sub(c, a);
    vec2_t ab = vec2_sub(b, a);
    vec2_t pc = vec2_sub(c, p);
    vec2_t pb = vec2_sub(b, p);
    vec2_t ap = vec2_sub(p, a);

    // Side note: we can also frame this business as using the determinant,
    // as it is defined as the signed area of the parallelogram formed by two
    // vectors. A bonus is we don't have to jump through the logical hoops of
    // justifying why we can use a operation defined for vectors in R^3 on
    // vectors in R^2.

    // Area of the full parallelogram (triangle ABC) using cross product
    float area_parallelogram_abc = ac.x * ab.y - ac.y * ab.x;

    // Alpha = area of parallelogram-PBC over the area of the full parallelogram-ABC
    float alpha = (pc.x * pb.y - pc.y * pb.x) / area_parallelogram_abc;

    // Beta = area of parallelogram-APC over the area of the full parallelogram-ABC
    float beta = (ac.x * ap.y - ac.y * ap.x) / area_parallelogram_abc;
    
    // Because barycentric coordinates add up to 1, Gamma must complement alpha and beta
    float gamma = 1.0 - alpha - beta;

    vec3_t weights = { alpha, beta, gamma };
    return weights;
}

void draw_solid_pixel(
    int x, int y, uint32_t color,
    vec4_t point_a, vec4_t point_b, vec4_t point_c
) {
    vec2_t p = { x, y };
    vec2_t a = vec2_from_vec4(point_a); 
    vec2_t b = vec2_from_vec4(point_b); 
    vec2_t c = vec2_from_vec4(point_c); 
    vec3_t weights = barycentric_weights(a, b, c, p);

    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    // Interpolate and invert 1/w
    float interpolated_reciprocal_w = alpha * (1/point_a.w) + beta * (1/point_b.w) + gamma * (1/point_c.w);
    float inv_interpolated_reciprocal_w = 1 - interpolated_reciprocal_w;
    
    // Only draw the pixel if the depth value is less than the current one
    if (inv_interpolated_reciprocal_w < get_z_buffer_at(x, y)) {
        draw_pixel(x, y, color);
        update_z_buffer_at(x, y, inv_interpolated_reciprocal_w);
    }
}

void draw_filled_triangle(
        int x0, int y0, float z0, float w0, 
        int x1, int y1, float z1, float w1, 
        int x2, int y2, float z2, float w2, 
        uint32_t color
) {
    // Sort vertices by y-coordinate (y0 < y1 < y2)
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&z1, &z2);
        float_swap(&w1, &w2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
    }

    vec4_t point_a = { x0, y0, z0, w0 };
    vec4_t point_b = { x1, y1, z1, w1 };
    vec4_t point_c = { x2, y2, z2, w2 };

    // Render the upper part of the triangle (flat-bottom)
    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    if (y1 - y0 != 0) inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
    if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y1 - y0 != 0) {
        for (int y = y0; y <= y1; y++) {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start) int_swap(&x_start, &x_end);

            for (int x = x_start; x <= x_end; x++) {
                draw_solid_pixel(x, y, color, point_a, point_b, point_c);
            }
        }
    }

    // Render the bottom part of the triangle (flat-top)
    inv_slope_1 = 0;
    inv_slope_2 = 0;

    if (y2 - y1 != 0) inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
    if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y2 - y1 != 0) {
        for (int y = y1; y <= y2; y++) {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start) int_swap(&x_start, &x_end);

            for (int x = x_start; x <= x_end; x++) {
                draw_solid_pixel(x, y, color, point_a, point_b, point_c);
            }
        }
    }
}

void draw_texel(
        int x, int y, upng_t *texture,
        vec4_t point_a, vec4_t point_b, vec4_t point_c,
        tex2_t a_uv, tex2_t b_uv, tex2_t c_uv
) {
    vec2_t p = { x, y };
    vec2_t a = vec2_from_vec4(point_a); 
    vec2_t b = vec2_from_vec4(point_b); 
    vec2_t c = vec2_from_vec4(point_c); 
    vec3_t weights = barycentric_weights(a, b, c, p);

    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    // Store interpolated values for current pixel
    float interpolated_u;
    float interpolated_v;
    float interpolated_reciprocal_w;

    // Perform the interpolation of all U and V values using barycentric weights and 1/w factor
    interpolated_u = alpha * (a_uv.u/point_a.w) + beta * (b_uv.u/point_b.w) + gamma * (c_uv.u/point_c.w);
    interpolated_v = alpha * (a_uv.v/point_a.w) + beta * (b_uv.v/point_b.w) + gamma * (c_uv.v/point_c.w);

    // Also interpolate the value of 1/w for the current pixel
    interpolated_reciprocal_w = alpha * (1/point_a.w) + beta * (1/point_b.w) + gamma * (1/point_c.w);

    interpolated_u /= interpolated_reciprocal_w;
    interpolated_v /= interpolated_reciprocal_w;

    // Get the mesh texture width and height dimensions
    int texture_width = upng_get_width(texture);
    int texture_height = upng_get_height(texture);

    // Map the UV coordinates to the full texture width and height
    int tex_x = abs((int)(interpolated_u * texture_width));
    int tex_y = abs((int)(interpolated_v * texture_height));

    // Guard against buffer overflow by wrapping overshooting coordinates
    int i = texture_width * tex_y + tex_x;
    int m = texture_width * texture_height;

    // Adjust 1/w so the pixels that are closer have smaller values
    float inv_interpolated_reciprocal_w = 1 - interpolated_reciprocal_w;
    
    // Only draw the pixel if the depth value is less than the current one
    if (inv_interpolated_reciprocal_w < get_z_buffer_at(x, y)) {
        // Get the buffer of colors from the texture
        uint32_t *texture_buffer = (uint32_t *)upng_get_buffer(texture);
        // Update the color and z-buffers
        draw_pixel(x, y, texture_buffer[i % m]);
        update_z_buffer_at(x, y, inv_interpolated_reciprocal_w);
    }
}

void draw_textured_triangle(
        int x0, int y0, float z0, float w0, float u0, float v0,
        int x1, int y1, float z1, float w1, float u1, float v1,
        int x2, int y2, float z2, float w2, float u2, float v2,
        upng_t *texture
) {
    // Sort vertices by y-coordinate (y0 < y1 < y2)
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&z1, &z2);
        float_swap(&w1, &w2);
        float_swap(&u1, &u2);
        float_swap(&v1, &v2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }

    // Flip the V component to account for inverted UV-coordinates (V grows downward)
    v0 = 1 - v0;
    v1 = 1 - v1;
    v2 = 1 - v2;

    // Create vector points after sorting vetices
    vec4_t point_a = { x0, y0, z0, w0 };
    vec4_t point_b = { x1, y1, z1, w1 };
    vec4_t point_c = { x2, y2, z2, w2 };

    // Create texture coordinates
    tex2_t a_uv = { u0, v0 };
    tex2_t b_uv = { u1, v1 };
    tex2_t c_uv = { u2, v2 };

    // Render the upper part of the triangle (flat-bottom)
    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    if (y1 - y0 != 0) inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
    if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y1 - y0 != 0) {
        for (int y = y0; y <= y1; y++) {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start) int_swap(&x_start, &x_end);

            for (int x = x_start; x <= x_end; x++) {
                draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
            }
        }
    }

    // Render the bottom part of the triangle (flat-top)
    inv_slope_1 = 0;
    inv_slope_2 = 0;

    if (y2 - y1 != 0) inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
    if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y2 - y1 != 0) {
        for (int y = y1; y <= y2; y++) {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start) int_swap(&x_start, &x_end);

            for (int x = x_start; x <= x_end; x++) {
                draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
            }
        }
    }
}
