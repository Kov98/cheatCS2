// offsets.h
#pragma once
#include <cstdint>
#include <string>

namespace off {
    // client.dll base
    extern std::uintptr_t client_base;

    // global offsets (resolved via pattern)
    extern std::uintptr_t dwEntityList;
    extern std::uintptr_t dwLocalPlayerPawn;
    extern std::uintptr_t dwLocalPlayerController;
    extern std::uintptr_t dwViewMatrix;
    extern std::uintptr_t dwViewAngles;
    extern std::uintptr_t dwGlobalVars;
    extern std::uintptr_t dwGameRules;
    extern std::uintptr_t dwPlantedC4;

    // schema field offsets (resolved at runtime, fallback to dump)
    namespace fields {
        extern std::uint32_t m_iHealth;
        extern std::uint32_t m_iTeamNum;
        extern std::uint32_t m_lifeState;
        extern std::uint32_t m_pGameSceneNode;
        extern std::uint32_t m_bDormant;
        extern std::uint32_t m_iShotsFired;
        extern std::uint32_t m_aimPunchAngle;
        extern std::uint32_t m_bSpottedByMask;
        extern std::uint32_t m_flFlashDuration;
        extern std::uint32_t m_vecAbsVelocity;
        extern std::uint32_t m_vecOrigin;
        extern std::uint32_t m_modelState;
        extern std::uint32_t m_pBoneArray;
        extern std::uint32_t m_pClippingWeapon;
        extern std::uint32_t m_hOwnerEntity;
    }

    bool init(std::uintptr_t client);
    bool all_resolved();
    void log_failures();
}