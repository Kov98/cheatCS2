// triggerbot.cpp
#include "triggerbot.h"
#include "sdk/client.h"
#include "core/process.h"
#include "core/offsets.h"
#include <windows.h>
#include <random>

namespace features {
TriggerbotSettings g_trigger;

static std::mt19937 rng{ std::random_device{}() };

static void fire_click() {
    INPUT in{};
    in.type = INPUT_MOUSE;
    in.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &in, sizeof(in));
    Sleep(6);
    in.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &in, sizeof(in));
}

void triggerbot_tick(const std::vector<Entity>& enemies) {
    if (!g_trigger.enabled) return;
    if (!g_trigger.always_on && !(GetAsyncKeyState(g_trigger.key) & 0x8000)) return;

    auto local = client::local_pawn();
    if (!local) return;

    if (g_trigger.anti_flash) {
        auto flash = proc::safe_read<float>(local + off::fields::m_flFlashDuration);
        if (flash && *flash > 0.f) return;
    }

    auto vm = client::view_matrix();
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    Vec2 center{ sw * 0.5f, sh * 0.5f };

    for (const auto& e : enemies) {
        if (g_trigger.vis_only && !e.spotted) continue;
        Vec3 target = e.bone(g_trigger.hitbox);
        Vec2 s;
        if (!world_to_screen(target, s, vm, sw, sh)) continue;
        if (screen_dist(s, center) < 4.f) {
            std::uniform_int_distribution<int> delay(g_trigger.delay_min_ms, g_trigger.delay_max_ms);
            Sleep(delay(rng));
            fire_click();
            break;
        }
    }
}

} // namespace features