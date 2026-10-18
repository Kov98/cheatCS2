// aimbot.h
#pragma once
#include <cstdint>
#include "sdk/entity.h"

namespace features {
    struct AimbotSettings {
        bool  enabled = false;
        bool  ignore_vis = false;
        bool  sticky = true;
        float fov_deg = 5.f;
        float smooth = 0.28f;
        int   bone_idx = 6; // 6=head, 7=neck, 9=chest, 0=pelvis
        bool  rcs_enabled = true;
        float rcs_pitch_scale = 2.0f;
        float rcs_yaw_scale = 2.0f;
        bool  prediction = true;
        float prediction_scale = 0.018f; // seconds
        std::uintptr_t sticky_target = 0;
    };
    extern AimbotSettings g_aimbot;

    void aimbot_tick(const std::vector<Entity>& enemies);
}