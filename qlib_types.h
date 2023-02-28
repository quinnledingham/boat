#ifndef QLIB_TYPES_H
#define QLIB_TYPES_H

typedef int8_t s8
typedef int16_t s16
typedef int32_t s32
typedef int64_t s64
typedef s8 b8
typedef s32 b32

typedef uint8_t u8
typedef uint16_t u16
typedef uint32_t u32
typedef uint64_t u64

typedef float r32
typedef double r64
typedef r32 f32
typedef r64 f64

#define function static
#define local_persist static
#define global_variable static

#define DEG2RAD 0.0174533f
#define PI 3.14159265359f
#define ARRAY_COUNT(n) (sizeof(n) / sizeof(n[0]));

union v2
{
    struct
    {
        r32 x, y;
    };
    struct
    {
        r32 u, v;
    };
    struct
    {
        r32 width, height;
    };
    r32 E[2];
};
v2 operator+(const v2 &l, const v2 &r) { return { l.x + r.x, l.y + r.y }; }

union v2s
{
    struct
    {
        s32 x, y;
    };
    struct
    {
        s32 u, v;
    };
    struct
    {
        s32 width, height;
    };
    s32 E[2];
};
v2 cv2(const v2s v) { return { (r32)v.x, (r32)v.y }; }

struct str
{
    const char *memory;
    u32 length;
    
    const char* get() { return memory; }
};

function u32
game()
{
    // window()
    // load_assets()
    // init_vars()
    u32 run_time_ms = 0;
    
    while(1)
    {
        // input()
        
        // update()
        
        // draw()
        
        // getfps()
        
    }
    
    return 0;
}

int main(int argc, char *argv[]) { return game(); }

#endif //QLIB_TYPES_H
