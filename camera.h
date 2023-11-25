#ifndef CAMERA_H
#define CAMERA_H
#include "aircraft.h"

typedef struct {
    float distance;
    float pitch;
    float yaw;
} camera_t;

void camera_transform(const camera_t *camera, aircraft_t *aircraft)
{
    // Set the camera transform
    glLoadIdentity();
    glTranslatef(0,0,camera->distance);
    glRotatef(camera->pitch, 1, 0, 0);
    glRotatef(camera->yaw, 0, 1, 0);
    glTranslatef(-aircraft->pos[0],-aircraft->pos[1],-aircraft->pos[2]);
}

#endif
