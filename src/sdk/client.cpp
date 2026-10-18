// client.cpp
#include "client.h"
#include "core/offsets.h"
#include "core/process.h"
#include "core/crypt.h"

namespace client {

static std::uintptr_t g_entity_system = 0;

bool init() {
    // entity system is a global pointer in client.dll, resolved via
    // CreateInterface("GameEntitySystem") is also possible; pattern is simpler
    auto sig = ENC("48 8B 0D ? ? ? ? 48 85 C9 74 ? 8B 41");
    auto match = proc::pattern_scan(off::client_base, sig.c_str());
    if (match) {
        auto p = proc::resolve_rip(match, 3, 7);
        auto deref = proc::safe_read_ptr(p);
        if (deref) g_entity_system = *deref;
    }
    return g_entity_system != 0;
}

std::uintptr_t local_pawn() {
    auto p = proc::safe_read_ptr(off::client_base + off::dwLocalPlayerPawn);
    return p ? *p : 0;
}

std::uintptr_t local_controller() {
    auto p = proc::safe_read_ptr(off::client_base + off::dwLocalPlayerController);
    return p ? *p : 0;
}

int local_team() {
    auto pawn = local_pawn();
    if (!pawn) return 0;
    auto t = proc::safe_read<int>(pawn + off::fields::m_iTeamNum);
    return t ? *t : 0;
}

Mat4 view_matrix() {
    Mat4 vm{};
    if (auto m = proc::safe_read<Mat4>(off::client_base + off::dwViewMatrix))
        vm = *m;
    return vm;
}

Vec3 view_angles() {
    Vec3 a{};
    if (auto v = proc::safe_read<Vec3>(off::client_base + off::dwViewAngles))
        a = *v;
    return a;
}

// entity list is a sparse array of identity blocks. walks chunks.
std::vector<Entity> enemies() {
    std::vector<Entity> out;
    auto local = local_pawn();
    if (!local) return out;
    int my_team = local_team();

    auto list_base = proc::safe_read_ptr(off::client_base + off::dwEntityList);
    if (!list_base) return out;

    for (int chunk = 0; chunk < 64; ++chunk) {
        auto chunk_ptr = proc::safe_read_ptr(*list_base + 8 * chunk);
        if (!chunk_ptr) continue;
        for (int idx = 0; idx < 512; ++idx) {
            auto identity = proc::safe_read_ptr(*chunk_ptr + 0x78 * idx);
            if (!identity) continue;
            auto pawn = proc::safe_read_ptr(*identity + 0x10);
            if (!pawn || *pawn == local) continue;

            Entity e;
            if (!Entity::read(*pawn, e)) continue;
            if (e.dormant) continue;
            if (e.team == my_team) continue;
            out.push_back(e);
        }
    }
    return out;
}

} // namespace client