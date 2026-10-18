// aimbot.cpp
#include "aimbot.h"
#include "sdk/client.h"
#include "core/process.h"
#include "core/offsets.h"
#include "core/crypt.h"
#include <cmath>

namespace features {
AimbotSettings g_aimbot;

static float fov_to_pixels(float fov_deg, int screen_w) {
    // CS2 default FOV 90, horizontal. pixels_per_degree approx
    return (screen_w / 90.f) * fov_deg;
}

void aimbot_tick(const std::vector<Entity>& enemies) {
    if (!g_aimbot.enabled) return;
    if (!(GetAsyncKeyState(VK_XBUTTON2) & 0x8000)) return; // mouse 5

    auto vm = client::view_matrix();
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    Vec2 center{ sw * 0.5f, sh * 0.5f };
    float fov_px = fov_to_pixels(g_aimbot.fov_deg, sw);

    const Entity* best = nullptr;
    float best_dist = fov_px;

    for (const auto& e : enemies) {
        if (!g_aimbot.ignore_vis && !e.spotted) continue;
        Vec3 target = e.bone(g_aimbot.bone_idx);
        Vec2 screen;
        if (!world_to_screen(target, screen, vm, sw, sh)) continue;
        float d = screen_dist(screen, center);
        if (d < best_dist) { best_dist = d; best = &e; }
    }

    if (!best) {
        g_aimbot.sticky_target = 0;
        return;
    }

    // sticky — keep same target if in FOV
    if (g_aimbot.sticky && g_aimbot.sticky_target) {
        bool found = false;
        for (const auto& e : enemies) {
            if (e.pawn != g_aimbot.sticky_target) continue;
            Vec3 t = e.bone(g_aimbot.bone_idx);
            Vec2 s;
            if (world_to_screen(t, s, vm, sw, sh) && screen_dist(s, center) < fov_px) {
                best = &e; found = true;
            }
            break;
        }
        if (!found) g_aimbot.sticky_target = 0;
    }
    g_aimbot.sticky_target = best->pawn;

    Vec3 local_eye;
    auto local_pawn = client::local_pawn();
    auto node = proc::safe_read_ptr(local_pawn + off::fields::m_pGameSceneNode);
    if (node) {
        auto bone_arr = proc::safe_read_ptr(*node + off::fields::m_modelState + off::fields::m_pBoneArray);
        if (bone_arr) local_eye = *reinterpret_cast<Vec3*>(*bone_arr + 6 * 0x20);
    }
    local_eye.z += 64.f;

    Vec3 target = best->bone(g_aimbot.bone_idx);

    // prediction: extrapolate target position based on velocity
    if (g_aimbot.prediction) {
        Vec3 predicted = target + best->velocity * g_aimbot.prediction_scale;
        target = predicted;
    }

    Vec3 desired = angle_to(local_eye, target);
    Vec3 current = client::view_angles();

    Vec3 delta{ desired.x - current.x, desired.y - current.y, 0.f };
    while (delta.y > 180.f)  delta.y -= 360.f;
    while (delta.y < -180.f) delta.y += 360.f;

    Vec3 next = current + delta * g_aimbot.smooth;

    // RCS — subtract aim punch, scaled
    if (g_aimbot.rcs_enabled) {
        auto punch = proc::safe_read<Vec3>(local_pawn + off::fields::m_aimPunchAngle);
        if (punch) {
            next.x -= punch->x * g_aimbot.rcs_pitch_scale;
            next.y -= punch->y * g_aimbot.rcs_yaw_scale;
        }
    }

    normalize_angle(next);
    proc::safe_write<Vec3>(off::client_base + off::dwViewAngles, next);
}

} // namespace features