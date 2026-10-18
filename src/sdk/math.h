#pragma once
#include <cmath>
#include <cstdint>

struct Vec2 { float x, y; };
struct Vec3 {
    float x, y, z;
    Vec3 operator+(Vec3 o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(Vec3 o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(float s) const { return {x*s, y*s, z*s}; }
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    float length_2d() const { return std::sqrt(x*x + y*y); }
};
struct Mat4 { float m[4][4]; };

static inline float dot(const Vec3& a, const Vec3& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

static inline void normalize_angle(Vec3& a) {
    while (a.x > 89.f)   a.x -= 180.f;
    while (a.x < -89.f)  a.x += 180.f;
    while (a.y > 180.f)  a.y -= 360.f;
    while (a.y < -180.f) a.y += 360.f;
    a.z = 0.f;
}

// angle from local eye to target world point. Source: yaw=atan2(dy,dx), pitch=-atan2(dz,len2d)
static inline Vec3 angle_to(const Vec3& from, const Vec3& to) {
    Vec3 d = to - from;
    return {
        -std::atan2f(d.z, d.length_2d()) * (180.f / 3.14159265f),
         std::atan2f(d.y, d.x)           * (180.f / 3.14159265f),
         0.f
    };
}

static inline float angle_dist(Vec3 a, Vec3 b) {
    Vec3 d{ b.x - a.x, b.y - a.y, 0.f };
    while (d.y > 180.f) d.y -= 360.f;
    while (d.y < -180.f) d.y += 360.f;
    return std::sqrt(d.x*d.x + d.y*d.y);
}

static inline bool world_to_screen(const Vec3& world, Vec2& out,
                                    const Mat4& vm, int sw, int sh) {
    float w = vm.m[3][0]*world.x + vm.m[3][1]*world.y + vm.m[3][2]*world.z + vm.m[3][3];
    if (w < 0.01f) return false;
    float x = vm.m[0][0]*world.x + vm.m[0][1]*world.y + vm.m[0][2]*world.z + vm.m[0][3];
    float y = vm.m[1][0]*world.x + vm.m[1][1]*world.y + vm.m[1][2]*world.z + vm.m[1][3];
    out.x = (sw * 0.5f) * (1.f + x / w);
    out.y = (sh * 0.5f) * (1.f - y / w);
    return true;
}

// 3D distance to crosshair in world-space, projected — used for FOV check
static inline float screen_dist(const Vec2& a, const Vec2& b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}