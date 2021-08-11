#ifndef V3D
#define V3D

// Vector with 3 double fields

#include <math.h> // sqrt

typedef struct V3d {
    double x;
    double y;
    double z;
} V3d;

// Return a + b
static inline struct V3d V3d_Add(const struct V3d a, const struct V3d b) {
    return (struct V3d) {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z
    };
}

// Return a - b
static inline struct V3d V3d_Sub(const struct V3d a, const struct V3d b) {
    return (struct V3d) {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z
    };
}

// Return a with all members multiplied by mul
static inline struct V3d V3d_Mul(const struct V3d a, const double mul) {
    return (struct V3d) {
        .x = a.x * mul,
        .y = a.y * mul,
        .z = a.z * mul
    };
}

// Return a with all members divided by div
static inline struct V3d V3d_Div(const struct V3d a, const double div) {
    return (struct V3d) {
        .x = a.x / div,
        .y = a.y / div,
        .z = a.z / div
    };
}

// Return magnitude of a
static inline double V3d_Mag(const struct V3d a) {
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

// Return unit vector in direction of a
static inline struct V3d V3d_Unit(const struct V3d a) {
    const double mag = V3d_Mag(a);
    return (struct V3d) {
        .x = a.x / mag,
        .y = a.y / mag,
        .z = a.z / mag
    };
}

// Return midpoint of a and b
static inline struct V3d V3d_Midpoint(const struct V3d a, const struct V3d b) {
    return V3d_Div(V3d_Add(a, b), 2.0);
}

// Return dot product of a and b
static inline double V3d_Dot(const struct V3d a, const struct V3d b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Return distance between a and b
static inline double V3d_Distance(const struct V3d a, const struct V3d b) {
    return V3d_Mag(V3d_Sub(a, b));
}

#endif // V3D
