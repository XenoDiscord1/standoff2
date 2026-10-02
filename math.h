#pragma once
#include <cmath>

// ── Column-major 4×4 (OpenGL convention) ─────────────────────────────────
struct Vec2 { float x, y; };
struct Vec3 { float x, y, z; };
struct Vec4 { float x, y, z, w; };

struct Mat4 {
    float m[16]; // m[col*4 + row]
};

inline Vec4 mat4_mul_vec4(const Mat4& M, const Vec4& v) {
    return {
        M.m[0]*v.x + M.m[4]*v.y + M.m[ 8]*v.z + M.m[12]*v.w,
        M.m[1]*v.x + M.m[5]*v.y + M.m[ 9]*v.z + M.m[13]*v.w,
        M.m[2]*v.x + M.m[6]*v.y + M.m[10]*v.z + M.m[14]*v.w,
        M.m[3]*v.x + M.m[7]*v.y + M.m[11]*v.z + M.m[15]*v.w,
    };
}

// Project a world-space point through MVP → screen pixel.
// vp: glGetIntegerv(GL_VIEWPORT) result [x, y, w, h]
// Returns false if point is behind camera (clip.w <= 0).
inline bool world_to_screen(const Mat4& mvp, float wx, float wy, float wz,
                             const int vp[4], float& sx, float& sy)
{
    Vec4 clip = mat4_mul_vec4(mvp, {wx, wy, wz, 1.f});
    if (clip.w <= 0.f) return false;
    float ndcx =  clip.x / clip.w;
    float ndcy =  clip.y / clip.w;
    sx = (ndcx + 1.f) * 0.5f * (float)vp[2] + (float)vp[0];
    sy = (1.f - ndcy) * 0.5f * (float)vp[3] + (float)vp[1];
    return true;
}
