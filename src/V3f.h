#ifndef V3F
#define V3F

// Vector with 3 double fields
// Should probably be renamed to V3d

#include <math.h> // sqrt

typedef struct V3f {
    double x;
    double y;
    double z;
} V3f;

// Return a + b
static inline struct V3f V3f_Add(const struct V3f a, const struct V3f b) {
    return (struct V3f) {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z
    };
}

// Return a - b
static inline struct V3f V3f_Sub(const struct V3f a, const struct V3f b) {
    return (struct V3f) {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z
    };
}

// Return a with all members multiplied by mul
static inline struct V3f V3f_Mul(const struct V3f a, const double mul) {
    return (struct V3f) {
        .x = a.x * mul,
        .y = a.y * mul,
        .z = a.z * mul
    };
}

// Return a with all members divided by div
static inline struct V3f V3f_Div(const struct V3f a, const double div) {
    return (struct V3f) {
        .x = a.x / div,
        .y = a.y / div,
        .z = a.z / div
    };
}

// Return magnitude of a
static inline double V3f_Mag(const struct V3f a) {
    return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

// Return unit vector in direction of a
static inline struct V3f V3f_Unit(const struct V3f a) {
    const double mag = V3f_Mag(a);
    return (struct V3f) {
        .x = a.x / mag,
        .y = a.y / mag,
        .z = a.z / mag
    };
}

// Return midpoint of a and b
static inline struct V3f V3f_Midpoint(const struct V3f a, const struct V3f b) {
    return V3f_Div(V3f_Add(a, b), 2.0);
}

// Return dot product of a and b
static inline double V3f_Dot(const struct V3f a, const struct V3f b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Return distance between a and b
static inline double V3f_Distance(const struct V3f a, const struct V3f b) {
    return V3f_Mag(V3f_Sub(a, b));
}

#endif // V3F
