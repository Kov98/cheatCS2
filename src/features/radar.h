// radar.h
#pragma once
#include <imgui.h>
#include "sdk/entity.h"

namespace features {
    struct RadarSettings {
        bool   enabled = false;
        float  range_m = 1500.f;
        float  size = 180.f;
        ImVec2 pos = ImVec2(20, 20);
        ImU32  color_enemy = IM_COL32(255, 60, 60, 220);
        ImU32  color_local = IM_COL32(60, 200, 255, 255);
        ImU32  color_bg    = IM_COL32(0, 0, 0, 180);
    };
    extern RadarSettings g_radar;
    void radar_draw(const std::vector<Entity>& enemies, ImDrawList* dl);
}