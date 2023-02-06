#ifndef MATH_H
#define MATH_H

#define V3_EPSILON 0.000001f
#define MAT4_EPSILON 0.000001f
#define QUAT_EPSILON 0.000001f
#define DEG2RAD 0.0174533f
#define PI 3.14159265359f

inline v3
operator+(const v3 &l, const v3 &r)
{
    return {l.x + r.x, l.y + r.y, l.z + r.z};
}

inline v3
operator-(const v3 &l, const v3 &r)
{
    return {l.x - r.x, l.y - r.y, l.z - r.z};
}

inline v3
operator*(const v3 &v, float f)
{
    return {v.x * f, v.y * f, v.z * f};
}

inline bool
operator==(const v3 &v, float f)
{
    if (v.x == f, v.y == f, v.z == f)
        return true;
    else
        return false;
}

inline f32
dot_product(const v3 &l, const v3 &r)
{
    return (l.x * r.x) + (l.y * r.y) + (l.z * r.z);
}

inline v3
cross_product(const v3 &l, const v3 &r)
{
    return 
    {
        (l.y * r.z - l.z * r.y),
        (l.z * r.x - l.x * r.z),
        (l.x * r.y - l.y * r.x)
    };
}

inline real32
length_squared(const v3 &v)
{
    return (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
}

inline void
normalize(v3 &v)
{
    if (real32 len_sq = length_squared(v) < V3_EPSILON)
        return;
    else
    {
        real32 inverse_length = 1.0f / sqrtf(len_sq);
        v.x *= inverse_length;
        v.y *= inverse_length;
        v.z *= inverse_length;
    }
}

inline v3
normalized(const v3 &v)
{
    if (real32 len_sq = length_squared(v) < V3_EPSILON)
        return v;
    else
    {
        real32 inverse_length = 1.0f / sqrtf(len_sq);
        return {v.x * inverse_length, v.y * inverse_length, v.z * inverse_length};
    }
}

inline m4x4
get_frustum(real32 l, real32 r, real32 b, real32 t, real32 n, real32 f)
{
    if (l == r || t == b || n == f)
    {
        error("Invalid frustum");
        return {};
    }
    
    return
    {
        (2.0f * n) / (r - l), 0, 0, 0,
        0, (2.0f * n) / (t - b), 0, 0,
        (r + l) / (r - l), (t + b) / (t - b), (-(f + n)) / (f - n), -1,
        0, 0, (-2 * f * n) / (f - n), 0
    };
}

inline m4x4
perspective_projection(real32 fov, real32 aspect_ratio, real32 n, real32 f)
{
    real32 y_max = n * tanf(fov * PI / 360.0f);
    real32 x_max = y_max * aspect_ratio;
    return get_frustum(-x_max, x_max, -y_max, y_max, n, f);
}

inline m4x4
look_at(const v3 &position, const v3 &target, const v3 &up)
{
    v3 f = normalized(target - position) * -1.0f;
    v3 r = cross_product(up, f);
    if (r == 0)
        return {};
    normalize(r);
    v3 u = normalized(cross_product(f, r));
    v3 t = {-dot_product(r, position), -dot_product(u, position), -dot_product(f, position)};
    
    return
    {
        r.x, u.x, f.x, 0,
        r.y, u.y, f.y, 0,
        r.z, u.z, f.z, 0,
        t.x, t.y, t.z, 1
    };
}

inline m4x4 create_transform_m4x4(v3 position, v4 rotation, v3 scale)
{
    v3 x;
    v3 y;
    v3 z;
    
    x = x * scale.x;
    y = y * scale.y;
    z = z * scale.z;
    
    return
    {
        x.x, x.y, x.z, 0,
        y.x, y.y, y.z, 0,
        z.x, z.y, z.z, 0,
        position.x, position.y, position.z, 1
    };
}

#endif //MATH_H
