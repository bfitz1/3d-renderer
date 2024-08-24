#include <math.h>
#include "display.h"
#include "triangle.h"
#include "swap.h"

void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int Mx, int My, uint32_t color) {
    // Find the two slopes starting from the top vertex (x0, y0)
    float inv_slope1 = (x1 - x0) / (float)(y1 - y0);
    float inv_slope2 = (Mx - x0) / (float)(My - y0);

    // Start x_start and x_end from the top vertex (x0, y0)
    float x_start = x0;
    float x_end = x0;
    for (int y = y0; y <= y1; y++) {
        draw_line(x_start, y, x_end, y, color);
        x_start += inv_slope1;
        x_end += inv_slope2;
    }
}

void fill_flat_top_triangle(int x1, int y1, int Mx, int My, int x2, int y2, uint32_t color) {
    float inv_slope1 = (x1 - x2) / (float)(y1 - y2);
    float inv_slope2 = (Mx - x2) / (float)(My - y2);

    float x_start = x2;
    float x_end = x2;
    for (int y = y2; y >= y1; y--) {
        draw_line(x_start, y, x_end, y, color);
        x_start -= inv_slope1;
        x_end -= inv_slope2;
    }
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

void draw_texel(
        int x, int y, uint32_t *texture,
        vec2_t point_a, vec2_t point_b, vec2_t point_c,
        float u0, float v0, float u1, float v1, float u2, float v2
) {
    vec2_t point_p = { x, y };
    vec3_t weights = barycentric_weights(point_a, point_b, point_c, point_p);

    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    // Perform the interpolation of all U and V values using barycentric weights
    float interpolated_u = alpha * u0 + beta * u1 + gamma * u2;
    float interpolated_v = alpha * v0 + beta * v1 + gamma * v2;

    // Map the UV coordinates to the full texture width and height
    int tex_x = abs((int)(interpolated_u * texture_width));
    int tex_y = abs((int)(interpolated_v * texture_height));

    // Guard against buffer overflow by wrapping overshooting coordinates
    int i = texture_width * tex_y + tex_x;
    int m = texture_width * texture_height;
    draw_pixel(x, y, texture[i % m]);
}

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }

    if (y1 == y2) {
        // Only need to draw a single flat-bottom triangle
        fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
    } else if (y0 == y1) {
        // Only need to draw a single flat-top triangle
        fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
    } else {
        // Calculate the new vertex (Mx, My) using triangle similarity
        int My = y1;
        int Mx = (float)((x2 - x0) * (My - y0)) / (float)(y2 - y0) + x0;

        fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);
        fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
    }
}

void draw_textured_triangle(
        int x0, int y0, float u0, float v0,
        int x1, int y1, float u1, float v1,
        int x2, int y2, float u2, float v2,
        uint32_t *texture
) {
    // Sort vertices by y-coordinate (y0 < y1 < y2)
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&u1, &u2);
        float_swap(&v1, &v2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }

    // Create vector points after sorting vetices
    vec2_t point_a = { x0, y0 };
    vec2_t point_b = { x1, y1 };
    vec2_t point_c = { x2, y2 };

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
                // draw_pixel(x, y, x % 8 == 0 ? 0xFFFF00FF : 0xFF000000); // Cool, but not what we want
                draw_texel(x, y, texture, point_a, point_b, point_c, u0, v0, u1, v1, u2, v2);
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
                // draw_pixel(x, y, x % 8 == 0 ? 0xFFFF00FF : 0xFF000000);
                draw_texel(x, y, texture, point_a, point_b, point_c, u0, v0, u1, v1, u2, v2);
            }
        }
    }
}
