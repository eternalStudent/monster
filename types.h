
/*       NUMBERS        */
//----------------------//

typedef unsigned char byte;

#include <stdint.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

#define MAX_INT16 0x7fff
#define MAX_INT32 0x7fffffff
#define MAX_INT64 0x7fffffffffffffff

#define MIN_INT16 0x8000
#define MIN_INT32 0x80000000
#define MIN_INT64 0x8000000000000000

typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#define MAX_UINT16 0xffff
#define MAX_UINT32 0xffffffff
#define MAX_UINT64 0xffffffffffffffff

typedef float float32;
typedef double float64;

inline float32 INF32() {
    union {uint32 u; float32 f;} data;
    data.u = 0x7f800000;
    return data.f;
}

inline float32 M_INF32() {
    union {uint32 u; float32 f;} data;
    data.u = 0xff800000;
    return data.f;
}


/*      GEOMETRY        */
//----------------------//

union Point2 {
    struct {float32 x, y;};
    struct {float32 width, height;};
    float32 e[2];
};
typedef Point2 Dimensions2;

union Point3 {
    struct {float32 x, y, z;};
    struct {float32 r, g, b;};
    struct {float32 width, height, depth;};
    float32 e[3];
};
typedef Point3 Dimensions3;

union Point2i {
    struct {int32 x, y;};
    struct {int32 width, height;};
    int32 e[2];
};
typedef Point2i Dimensions2i;

union Box2 {
    struct {Point2 p0, p1;};
    struct {float32 x0, y0, x1, y1;};
};

inline Box2 BOX2_UNIT() {
    Box2 unit;
    unit.x0 = 0.0f;
    unit.y0 = 0.0f;
    unit.x1 = 1.0f;
    unit.y1 = 1.0f;
    return unit;
}

inline bool IsInsideBox(Box2 box, Point2 pos) {
    return (box.x0 <= pos.x && pos.x <= box.x1) &&
        (box.y0 <= pos.y && pos.y <= box.y1);
}

inline Box2 BOX2(float32 x0, float32 y0, float32 x1, float32 y1) {
    Box2 result;
    result.x0 = x0;
    result.y0 = y0;
    result.x1 = x1;
    result.y1 = y1;
    return result;
}

inline Box2 Box_Move(Box2 box, Point2 p) {
    return BOX2(box.x0+p.x, box.y0+p.y, box.x1+p.x, box.y1+p.y);
}

union Box3 {
    struct {Point2 p0, p1, p2;};
    struct {float32 x0, y0, x1, y1, z0, z1;};
};

struct Sphere2 {
    union {
        Point2 p;
        struct {float32 x, y;};
    };
    float32 r;
};

struct Sphere3 {
    union {
        Point3 p;
        struct {float32 x, y, z;};
    };
    float32 r;
};


/*      PHYSICS         */
//----------------------//

typedef float32 pixels;
typedef float32 milliseconds;
typedef float32 pixels_per_millisec;
typedef float32 pixels_per_millisec_2;

typedef Point2  Vector2;
typedef Vector2 Position2;
typedef Vector2 Velocity2;
typedef Vector2 Acceleration2;

typedef Point3  Vector3;
typedef Vector3 Position3;
typedef Vector3 Velocity3;
typedef Vector3 Accelaration3;

union Vector4 {
    struct {float32 x, y, z, w;};
    struct {float32 r, g, b, a;};
    float32 e[4];
};


/*       IMAGES         */
//----------------------//

struct Image {
    union {
        Dimensions2i dimensions;
        struct { int32 width, height;};
    };
    int32 channels;
    byte* data;
};

typedef Vector3 RGB;
typedef Vector4 RGBA;