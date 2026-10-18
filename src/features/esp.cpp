// esp.cpp
#include "esp.h"
#include "sdk/client.h"
#include "core/offsets.h"
#include "core/process.h"
#include <cstdio>

namespace features {
EspSettings g_esp;

void esp_draw(const std::vector<Entity>& enemies, ImDrawList* dl) {
    if (!g_esp.enabled) return;

    auto vm = client::view_matrix();
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    auto local = client::local_pawn();
    Vec3 local_pos{};
    if (auto node = proc::safe_read_ptr(local + off::fields::m_pGameSceneNode)) {
        if (auto ba = proc::safe_read_ptr(*node + off::fields::m_modelState + off::fields::m_pBoneArray)) {
            local_pos = *reinterpret_cast<Vec3*>(*ba);
        }
    }

    for (const auto& e : enemies) {
        if (g_esp.vis_only && !e.spotted) continue;
        float dist_m = (e.origin - local_pos).length() / 100.f;
        if (dist_m > g_esp.max_dist_m) continue;

        Vec2 hs, os, neck_s;
        if (!world_to_screen(e.head, hs, vm, sw, sh)) continue;
        if (!world_to_screen(e.pelvis, os, vm, sw, sh)) continue;
        if (!world_to_screen(e.neck, neck_s, vm, sw, sh)) continue;

        float h = os.y - hs.y;
        if (h < 6.f || h > 1400.f) continue;
        float w = h * 0.45f;
        ImU32 col = e.spotted ? g_esp.color_vis : g_esp.color_hidden;

        if (g_esp.boxes) {
            dl->AddRect(ImVec2(hs.x - w * 0.5f, hs.y), ImVec2(hs.x + w * 0.5f, os.y),
                        col, 0.f, 0, 1.2f);
        }

        if (g_esp.skeleton && e.bone_count >= 8) {
            // simple head-neck-pelvis chain; full bone list extends here
            struct Pair { int a, b; };
            Pair pairs[] = { {6,7}, {7,9}, {9,0} };
            for (auto& p : pairs) {
                Vec2 a, b;
                if (!world_to_screen(e.bone(p.a), a, vm, sw, sh)) continue;
                if (!world_to_screen(e.bone(p.b), b, vm, sw, sh)) continue;
                dl->AddLine(ImVec2(a.x, a.y), ImVec2(b.x, b.y), col, 1.f);
            }
        }

        if (g_esp.health) {
            float pct = e.health / 100.f;
            float bar_w = 3.f;
            float bar_x = hs.x - w * 0.5f - 6.f;
            dl->AddRectFilled(ImVec2(bar_x, hs.y), ImVec2(bar_x + bar_w, os.y),
                              g_esp.color_bg);
            float fg_top = os.y - h * pct;
            dl->AddRectFilled(ImVec2(bar_x, fg_top), ImVec2(bar_x + bar_w, os.y),
                              g_esp.color_health);
        }

        if (g_esp.distance) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.0fm", dist_m);
            dl->AddText(ImVec2(hs.x - w * 0.5f, os.y + 2), col, buf);
        }

        if (g_esp.names) {
            auto ctrl = proc::safe_read_ptr(e.pawn + off::fields::m_hOwnerEntity);
            (void)ctrl; // name resolution requires controller offset; add later
        }

        if (g_esp.weapon) {
            if (auto w = proc::safe_read_ptr(e.pawn + off::fields::m_pClippingWeapon)) {
                auto wid = proc::safe_read<std::uint16_t>(*w + 0x1C48);
                if (wid) {
                    char buf[16];
                    snprintf(buf, sizeof(buf), "id:%u", *wid);
                    dl->AddText(ImVec2(hs.x + w * 0.5f + 2.f, hs.y), col, buf);
                }
            }
        }
    }
}

} // namespace features