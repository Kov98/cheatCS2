// radar.cpp
#include "radar.h"
#include "sdk/client.h"
#include "core/process.h"
#include "core/offsets.h"
#include <cmath>

namespace features {
RadarSettings g_radar;

void radar_draw(const std::vector<Entity>& enemies, ImDrawList* dl) {
    if (!g_radar.enabled) return;

    ImVec2 center = ImVec2(g_radar.pos.x + g_radar.size * 0.5f,
                           g_radar.pos.y + g_radar.size * 0.5f);
    float r = g_radar.size * 0.5f;

    dl->AddCircleFilled(center, r, g_radar.color_bg, 48);
    dl->AddCircle(center, r, IM_COL32(120, 120, 120, 180), 48, 1.f);

    auto local = client::local_pawn();
    Vec3 lp{};
    Vec3 angles = client::view_angles();
    if (auto node = proc::safe_read_ptr(local + off::fields::m_pGameSceneNode)) {
        if (auto ba = proc::safe_read_ptr(*node + off::fields::m_modelState + off::fields::m_pBoneArray)) {
            lp = *reinterpret_cast<Vec3*>(*ba);
        }
    }

    // local: always center, drawn facing up (rotate map by -yaw)
    dl->AddTriangleFilled(
        ImVec2(center.x, center.y - 6),
        ImVec2(center.x - 4, center.y + 4),
        ImVec2(center.x + 4, center.y + 4),
        g_radar.color_local);

    float yaw_rad = angles.y * 3.14159265f / 180.f;
    float cos_y = std::cosf(-yaw_rad), sin_y = std::sinf(-yaw_rad);

    for (const auto& e : enemies) {
        float dx = e.origin.x - lp.x;
        float dy = e.origin.y - lp.y;
        float dist_m = std::sqrt(dx*dx + dy*dy) / 100.f;
        if (dist_m > g_radar.range_m) continue;

        // world -> radar: scale by range, rotate by -yaw
        float rx = (dx / (g_radar.range_m * 100.f)) * r;
        float ry = (dy / (g_radar.range_m * 100.f)) * r;
        float fx =  rx * cos_y - ry * sin_y;
        float fy =  rx * sin_y + ry * cos_y;

        ImVec2 pt(center.x + fx, center.y + fy);
        dl->AddCircleFilled(pt, 3.f, g_radar.color_enemy, 12);
    }
}

} // namespace features