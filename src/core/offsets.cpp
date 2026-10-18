// offsets.cpp
#include "offsets.h"
#include "process.h"
#include "crypt.h"
#include <cstdio>
#include <vector>

namespace off {

std::uintptr_t client_base = 0;
std::uintptr_t dwEntityList = 0;
std::uintptr_t dwLocalPlayerPawn = 0;
std::uintptr_t dwLocalPlayerController = 0;
std::uintptr_t dwViewMatrix = 0;
std::uintptr_t dwViewAngles = 0;
std::uintptr_t dwGlobalVars = 0;
std::uintptr_t dwGameRules = 0;
std::uintptr_t dwPlantedC4 = 0;

namespace fields {
std::uint32_t m_iHealth = 0;
std::uint32_t m_iTeamNum = 0;
std::uint32_t m_lifeState = 0;
std::uint32_t m_pGameSceneNode = 0;
std::uint32_t m_bDormant = 0;
std::uint32_t m_iShotsFired = 0;
std::uint32_t m_aimPunchAngle = 0;
std::uint32_t m_bSpottedByMask = 0;
std::uint32_t m_flFlashDuration = 0;
std::uint32_t m_vecAbsVelocity = 0;
std::uint32_t m_vecOrigin = 0;
std::uint32_t m_modelState = 0;
std::uint32_t m_pBoneArray = 0;
std::uint32_t m_pClippingWeapon = 0;
std::uint32_t m_hOwnerEntity = 0;
}

struct ResolveEntry {
    const char* name;
    std::uintptr_t* dst;
    const char* sig;             // pattern
    int          rip_offset;     // position of rel32 in the match (or -1 for direct deref)
    int          instr_len;      // instruction total length for rip calc
    std::uintptr_t fallback;     // dump value if sig fails
};

static std::vector<ResolveEntry> g_entries;
static bool g_initialized = false;

static void register_entries() {
    // NOTE: these signatures are placeholders. Replace from a2x/cs2-dumper
    // for the current build. Fallbacks below are 2026-10-06 values.
    g_entries = {
        { "dwEntityList",
          &dwEntityList,
          "48 8B 0D ? ? ? ? 48 89 4C 24 ? 4C 8D 44 24",
          3, 7, 0x2717828 },
        { "dwLocalPlayerPawn",
          &dwLocalPlayerPawn,
          "48 8B 05 ? ? ? ? 41 88 87 ? ? ? ? 48 8B 05",
          3, 7, 0x2562808 },
        { "dwViewMatrix",
          &dwViewMatrix,
          "48 8D 0D ? ? ? ? 48 C1 E0 06 48 03 C1",
          3, 7, 0x256A0A0 },
        { "dwViewAngles",
          &dwViewAngles,
          "48 8B 0D ? ? ? ? 48 8B 01 48 8B 40 40 FF 10",
          3, 7, 0x257A2E8 },
        { "dwLocalPlayerController",
          &dwLocalPlayerController,
          "48 8B 05 ? ? ? ? 48 8B 88 ? ? ? ? 48 8D 04 ? 48 8B 00",
          3, 7, 0x256A418 },
    };
}

bool init(std::uintptr_t client) {
    client_base = client;
    if (!g_initialized) { register_entries(); g_initialized = true; }

    for (auto& e : g_entries) {
        std::uintptr_t match = proc::pattern_scan(client, e.sig);
        if (match) {
            std::uintptr_t resolved = proc::resolve_rip(match, e.rip_offset, e.instr_len);
            if (resolved) {
                *e.dst = resolved - client_base; // store as RVA
                continue;
            }
        }
        *e.dst = e.fallback;
    }
    return all_resolved();
}

bool all_resolved() {
    for (auto& e : g_entries) if (*e.dst == 0) return false;
    return true;
}

void log_failures() {
    for (auto& e : g_entries) {
        if (*e.dst == 0) {
            char buf[128];
            snprintf(buf, sizeof(buf), "[cs2ext] offset %s not resolved\n", e.name);
            OutputDebugStringA(buf);
        }
    }
}

} // namespace off