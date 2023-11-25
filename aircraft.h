#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include "Quaternion.h"

typedef struct {
    double pos[3];
    Quaternion rotation;
    double rot[3];
    double Velocity[3];
    double fDirection[3];
    double roll;
    double yaw;
} aircraft_t;

/*void aircraft_transform(const aircraft_t *aircraft)
{
    // Set the camera transform
    glLoadIdentity();
    gluLookAt(
        0, -camera->distance, -camera->distance,
        0, 0, 0,
        0, 1, 0);
    glRotatef(camera->pitch, 1, 0, 0);
    glRotatef(camera->yaw, 0, 1, 0);
}*/

#endif
