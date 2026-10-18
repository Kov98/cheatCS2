// menu.cpp
#include "menu.h"
#include <imgui.h>
#include "features/aimbot.h"
#include "features/triggerbot.h"
#include "features/esp.h"
#include "features/radar.h"

namespace ui {

bool g_menu_open = false;

void menu_draw() {
    if (!g_menu_open) return;
    if (!ImGui::Begin("cs2ext", &g_menu_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End(); return;
    }

    if (ImGui::BeginTabBar("##tabs")) {
        if (ImGui::BeginTabItem("Aimbot")) {
            auto& a = features::g_aimbot;
            ImGui::Checkbox("Enabled", &a.enabled);
            ImGui::Checkbox("Ignore visibility", &a.ignore_vis);
            ImGui::Checkbox("Sticky target", &a.sticky);
            ImGui::SliderFloat("FOV (deg)", &a.fov_deg, 0.5f, 30.f, "%.1f");
            ImGui::SliderFloat("Smooth", &a.smooth, 0.f, 0.95f, "%.2f");
            const char* bones[] = { "Head", "Neck", "Chest", "Pelvis" };
            int bone_sel = (a.bone_idx == 6) ? 0 : (a.bone_idx == 7) ? 1 : (a.bone_idx == 9) ? 2 : 3;
            if (ImGui::Combo("Bone", &bone_sel, bones, 4)) {
                int ids[] = {6, 7, 9, 0};
                a.bone_idx = ids[bone_sel];
            }
            ImGui::Checkbox("RCS", &a.rcs_enabled);
            ImGui::SliderFloat("RCS pitch", &a.rcs_pitch_scale, 0.f, 4.f, "%.2f");
            ImGui::SliderFloat("RCS yaw",   &a.rcs_yaw_scale,   0.f, 4.f, "%.2f");
            ImGui::Checkbox("Prediction", &a.prediction);
            ImGui::SliderFloat("Prediction (s)", &a.prediction_scale, 0.f, 0.1f, "%.3f");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Triggerbot")) {
            auto& t = features::g_trigger;
            ImGui::Checkbox("Enabled", &t.enabled);
            ImGui::Checkbox("Always on", &t.always_on);
            ImGui::Checkbox("Visible only", &t.vis_only);
            ImGui::Checkbox("Anti-flash", &t.anti_flash);
            ImGui::SliderInt("Delay min (ms)", &t.delay_min_ms, 0, 200);
            ImGui::SliderInt("Delay max (ms)", &t.delay_max_ms, 0, 200);
            ImGui::SliderInt("Hitbox index", &t.hitbox, 0, 12);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Visuals")) {
            auto& e = features::g_esp;
            auto& r = features::g_radar;
            ImGui::Checkbox("ESP enabled", &e.enabled);
            ImGui::Checkbox("Boxes", &e.boxes);
            ImGui::Checkbox("Skeleton", &e.skeleton);
            ImGui::Checkbox("Health", &e.health);
            ImGui::Checkbox("Distance", &e.distance);
            ImGui::Checkbox("Weapon", &e.weapon);
            ImGui::Checkbox("Visible only", &e.vis_only);
            ImGui::SliderFloat("Max dist (m)", &e.max_dist_m, 100.f, 5000.f, "%.0f");
            ImGui::Separator();
            ImGui::Checkbox("Radar enabled", &r.enabled);
            ImGui::SliderFloat("Radar range", &r.range_m, 200.f, 4000.f, "%.0f");
            ImGui::SliderFloat("Radar size", &r.size, 80.f, 320.f, "%.0f");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Config")) {
            if (ImGui::Button("Save")) config::save("cs2ext.cfg");
            ImGui::SameLine();
            if (ImGui::Button("Load")) config::load("cs2ext.cfg");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

} // namespace ui