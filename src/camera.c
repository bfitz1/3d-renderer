#include "camera.h"
#include "vector.h"

static camera_t camera;

void init_camera(camera_t c) {
    camera = c;
}

vec3_t get_camera_position(void) {
    return camera.position;
}

vec3_t get_camera_direction(void) {
    return camera.direction;
}

vec3_t get_camera_forward_velocity() {
    return camera.forward_velocity;
}

float get_camera_yaw(void) {
    return camera.yaw;
}

float get_camera_pitch(void) {
    return camera.pitch;
}

void set_camera_position(vec3_t position) {
    camera.position = position;
}

void set_camera_direction(vec3_t direction) {
    camera.direction = direction;
}

void set_camera_forward_velocity(vec3_t forward_velocity) {
    camera.forward_velocity = forward_velocity;
}

void set_camera_yaw(float yaw) {
    camera.yaw = yaw;
}

void set_camera_pitch(float pitch) {
    camera.pitch = pitch;
}
