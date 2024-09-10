#pragma once

#include "vector.h"

typedef struct {
    vec3_t position;
    vec3_t direction;
    vec3_t forward_velocity;
    float yaw;
    float pitch;
} camera_t;

void init_camera(camera_t c);
vec3_t get_camera_position(void);
vec3_t get_camera_direction(void);
vec3_t get_camera_forward_velocity(void);
float get_camera_yaw(void);
float get_camera_pitch(void);
void set_camera_position(vec3_t position);
void set_camera_direction(vec3_t direction);
void set_camera_forward_velocity(vec3_t forward_velocity);
void set_camera_yaw(float yaw);
void set_camera_pitch(float pitch);
