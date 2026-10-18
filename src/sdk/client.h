// client.h
#pragma once
#include <cstdint>
#include <vector>
#include "entity.h"

namespace client {
    bool init();

    std::uintptr_t local_pawn();
    std::uintptr_t local_controller();
    int            local_team();

    Mat4 view_matrix();
    Vec3 view_angles();

    // returns all enemy pawns (filtered against local team)
    std::vector<Entity> enemies();
}