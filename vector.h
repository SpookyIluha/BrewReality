#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

inline void vector_scale(double* a, double scale){
    a[0] *= scale;
    a[1] *= scale;
    a[2] *= scale;
}

inline void vector_add(double* a, double* b, double* out){
    out[0] = a[0] + b[0];
    out[1] = a[1] + b[1];
    out[2] = a[2] + b[2];
}

inline void vector_cross(double* a, double* b, double* out){
    out[0] = a[1] * b[2] - a[2] * b[1];
    out[1] = a[0] * b[2] - a[2] * b[0];
    out[2] = a[0] * b[1] - a[1] * b[0];
}

inline void vector_normalize(double* a){
    double factor = 1/sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    a[0] *= factor;
    a[1] *= factor;
    a[2] *= factor;
}

#endif
