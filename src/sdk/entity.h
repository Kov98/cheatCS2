#pragma once
#include <cstdint>
#include "math.h"
#include "core/offsets.h"
#include "core/process.h"

struct Entity {
    std::uintptr_t pawn{};
    std::uintptr_t node{};
    Vec3           origin{};
    Vec3           head{};
    Vec3           neck{};
    Vec3           chest{};
    Vec3           pelvis{};
    Vec3           velocity{};
    int            health{};
    int            team{};
    bool           spotted{};
    bool           dormant{};
    bool           valid{};
    Vec3           bones[30]{};
    int            bone_count{};

    Vec3 bone(int idx) const { return idx < bone_count ? bones[idx] : Vec3{}; }

    static bool read(std::uintptr_t pawn, Entity& out) {
        auto health = proc::safe_read<int>(pawn + off::fields::m_iHealth);
        if (!health || *health <= 0 || *health > 100) return false;
        out.health = *health;
        out.pawn   = pawn;

        auto node = proc::safe_read_ptr(pawn + off::fields::m_pGameSceneNode);
        if (!node) return false;
        out.node = *node;

        auto team = proc::safe_read<int>(pawn + off::fields::m_iTeamNum);
        if (team) out.team = *team;

        auto dorm = proc::safe_read<bool>(pawn + off::fields::m_bDormant);
        if (dorm) out.dormant = *dorm;

        auto sp = proc::safe_read<std::uint64_t>(pawn + off::fields::m_bSpottedByMask);
        if (sp) out.spotted = *sp != 0;

        auto vel = proc::safe_read<Vec3>(pawn + off::fields::m_vecAbsVelocity);
        if (vel) out.velocity = *vel;

        // bones — read model state then bone array
        auto model_state = out.node + off::fields::m_modelState;
        auto bone_arr = proc::safe_read_ptr(model_state + off::fields::m_pBoneArray);
        if (bone_arr) {
            constexpr int MAX_BONES = 30;
            std::memcpy(out.bones, reinterpret_cast<void*>(*bone_arr), sizeof(Vec3) * MAX_BONES);
            out.bone_count = MAX_BONES;
            out.head   = out.bones[6];
            out.neck   = out.bones[7];
            out.chest  = out.bones[9];
            out.pelvis = out.bones[0];
            out.origin = out.pelvis;
        } else {
            // fallback: origin from scene node
            auto org = proc::safe_read<Vec3>(out.node + 0x80);
            if (org) out.origin = *org;
        }

        out.valid = true;
        return true;
    }
};