// esp.h
#pragma once
#include <imgui.h>
#include "sdk/entity.h"

namespace features {
    struct EspSettings {
        bool  enabled = false;
        bool  boxes = true;
        bool  skeleton = true;
        bool  health = true;
        bool  names = true;
        bool  weapon = true;
        bool  distance = true;
        bool  vis_only = false;
        float max_dist_m = 3000.f;
        ImU32 color_vis   = IM_COL32(0, 255, 0, 220);
        ImU32 color_hidden = IM_COL32(255, 200, 0, 200);
        ImU32 color_health = IM_COL32(0, 255, 0, 255);
        ImU32 color_bg     = IM_COL32(0, 0, 0, 180);
    };
    extern EspSettings g_esp;
    void esp_draw(const std::vector<Entity>& enemies, ImDrawList* dl);
}