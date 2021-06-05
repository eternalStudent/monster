#include <stdint.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef unsigned char byte;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float float32;
typedef double float64;

typedef float32 pixels;
typedef float32 milliseconds;
typedef float32 pixels_per_millisec;
typedef float32 pixels_per_millisec_2;

union Vector2f{
    struct{
        float32 x, y;
    };
    struct{
        float32 u, v;
    };
    struct{
        float32 width, height;
    };
    float32 e[2];
};

typedef Vector2f Dimensions2f;
typedef Vector2f Position2f;
typedef Vector2f Velocity2f;

union Vector2i{
    struct{
        int32 x, y;
    };
    struct{
        int32 width, height;
    };
    int32 e[2];
};

typedef Vector2i Dimensions2i;

union Vector3f{
    struct{
        float32 x, y, z;
    };
    struct{
        float32 r, g, b;
    };
    float32 e[3];
};

typedef Vector3f RGB;

union Vector3i{
    struct{
        int32 x, y, z;
    };
    int32 e[3];
};

union Vector4{
    struct {
        float32 x, y, z, w;
    };
    struct {
        float32 r, g, b, a;
    };
    float32 e[4];
};

typedef Vector4 RGBA;

union Rectanglef{
    struct{
        Vector2f pos0, pos1;
    };
    struct{
        float32 x0, y0, x1, y1;
    };
};

struct Image {
    union {
        Dimensions2i dimensions;
        struct {
            int32 width;
            int32 height;
        };
    };
    int32 channels;
    byte* data;
};

struct HitBox {
    union {
        Position2f pos;
        struct { pixels x, y; };
    };
    pixels radius, height;
};

// the following types should probably be moved?

#define LDN  1
#define LUP  2
#define RDN  3
#define RUP  4

// NOTE: I'm not actually currently using pos at all
struct MouseEvent {
    int32 type;
    union{
        Position2f pos;
        struct{pixels x, y;};
    };
};

struct MouseEventQueue{
    union{
        void* data;
        MouseEvent* table;
    };
    int32 size;
    int32 capacity;
    int32 front;
    int32 rear;
};