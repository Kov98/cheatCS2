// triggerbot.h
#pragma once
#include "sdk/entity.h"

namespace features {
    struct TriggerbotSettings {
        bool  enabled = false;
        bool  always_on = true;
        int   key = VK_XBUTTON1;      // mouse 4 if not always_on
        int   delay_min_ms = 20;
        int   delay_max_ms = 80;
        bool  vis_only = true;
        bool  anti_flash = true;
        int   hitbox = 6;
    };
    extern TriggerbotSettings g_trigger;
    void triggerbot_tick(const std::vector<Entity>& enemies);
}