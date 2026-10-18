// schema.cpp
#include "schema.h"
#include "process.h"
#include "offsets.h"
#include "crypt.h"
#include <cstdio>
#include <cstring>

namespace schema {

// --- interface layout (Source 2, common to CS2) ----------------------------
struct CUtlVector {
    int count;
    int _pad;
    void** data;
};

struct CSchemaType {
    void* vtable;
    const char* name;
    void* module;      // CSchemaTypeScope*
    std::uint32_t size;
};

struct CSchemaClassField {
    const char* name;
    CSchemaType* type;
    std::uint32_t offset;
    std::int32_t  metadata_size;
    void* metadata;
};

struct CSchemaClass {
    void* vtable;
    const char* name;
    const char* module_name;
    std::uint32_t size;
    std::uint16_t num_fields;
    std::uint16_t _pad;
    CSchemaClassField** fields;
};

struct CSchemaTypeScope {
    void* vtable;
    const char* name;
    CUtlVector classes;     // array of CSchemaClass*
    CUtlVector enums;
    CUtlVector types;
};

struct CSchemaSystem {
    void* vtable;
    char _pad[0x188];
    CSchemaTypeScope** scopes;
    std::uint32_t scope_count;
};

static CSchemaSystem* g_system = nullptr;

// find the schema system via pattern in client.dll. This one is stable across
// many builds (interface ptr stored in a global). Update sig if patch breaks it.
static CSchemaSystem* locate() {
    auto sig = ENC("48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 48 8B 01 FF 50 08");
    auto match = proc::pattern_scan(off::client_base, sig.c_str());
    if (!match) return nullptr;
    auto ptr = proc::resolve_rip(match, 3, 7);
    if (!ptr) return nullptr;
    return reinterpret_cast<CSchemaSystem*>(ptr);
}

static CSchemaTypeScope* find_scope(CSchemaSystem* sys, const char* module) {
    if (!sys || !sys->scopes) return nullptr;
    for (std::uint32_t i = 0; i < sys->scope_count; ++i) {
        auto* sc = sys->scopes[i];
        if (sc && sc->name && _stricmp(sc->name, module) == 0) return sc;
    }
    return nullptr;
}

static CSchemaClass* find_class(CSchemaTypeScope* scope, const char* name) {
    if (!scope || !scope->classes.data) return nullptr;
    for (int i = 0; i < scope->classes.count; ++i) {
        auto* c = reinterpret_cast<CSchemaClass*>(scope->classes.data[i]);
        if (c && c->name && strcmp(c->name, name) == 0) return c;
    }
    return nullptr;
}

bool init() {
    g_system = locate();
    if (!g_system) return false;

    auto client_str = ENC("client.dll");
    auto* scope = find_scope(g_system, client_str.c_str());
    if (!scope) return false;

    // resolve pawn fields
    auto* pawn = find_class(scope, ENC("C_CSPlayerPawn").c_str());
    if (pawn) {
        struct M { const char* name; std::uint32_t* dst; std::uint32_t fallback; };
        M fields[] = {
            {"m_iHealth",         &off::fields::m_iHealth,         0x344},
            {"m_iTeamNum",        &off::fields::m_iTeamNum,        0x3E3},
            {"m_lifeState",       &off::fields::m_lifeState,       0x338},
            {"m_pGameSceneNode",  &off::fields::m_pGameSceneNode,  0x328},
            {"m_bDormant",        &off::fields::m_bDormant,        0xED},
            {"m_iShotsFired",     &off::fields::m_iShotsFired,     0x1C44},
            {"m_aimPunchAngle",   &off::fields::m_aimPunchAngle,   0x1C58},
            {"m_bSpottedByMask",  &off::fields::m_bSpottedByMask,  0x1C60},
            {"m_flFlashDuration", &off::fields::m_flFlashDuration, 0x1C70},
            {"m_vecAbsVelocity",  &off::fields::m_vecAbsVelocity,  0x3E8},
            {"m_pClippingWeapon", &off::fields::m_pClippingWeapon, 0x12A8},
        };
        for (auto& f : fields) {
            bool found = false;
            for (int i = 0; i < pawn->num_fields; ++i) {
                auto* field = pawn->fields[i];
                if (field && field->name && strcmp(field->name, f.name) == 0) {
                    *f.dst = field->offset; found = true; break;
                }
            }
            if (!found) *f.dst = f.fallback;
        }
    }

    // resolve node fields (m_modelState is inside CGameSceneNode, not pawn)
    auto* node = find_class(scope, ENC("CGameSceneNode").c_str());
    if (node) {
        for (int i = 0; i < node->num_fields; ++i) {
            auto* field = node->fields[i];
            if (field && field->name && strcmp(field->name, "m_modelState") == 0) {
                off::fields::m_modelState = field->offset; break;
            }
        }
    }
    if (!off::fields::m_modelState) off::fields::m_modelState = 0x170;

    // m_pBoneArray lives inside CModelState (nested) — usually at +0x80 from
    // the embedded CModelState. Resolve CModelState class:
    auto* ms = find_class(scope, ENC("CModelState").c_str());
    if (ms) {
        for (int i = 0; i < ms->num_fields; ++i) {
            auto* field = ms->fields[i];
            if (field && field->name && strcmp(field->name, "m_pBoneArray") == 0) {
                off::fields::m_pBoneArray = field->offset; break;
            }
        }
    }
    if (!off::fields::m_pBoneArray) off::fields::m_pBoneArray = 0x80;

    return true;
}

std::uint32_t find_field(const char* class_name, const char* field_name) {
    if (!g_system) return 0;
    auto client_str = ENC("client.dll");
    auto* scope = find_scope(g_system, client_str.c_str());
    if (!scope) return 0;
    auto* cls = find_class(scope, class_name);
    if (!cls) return 0;
    for (int i = 0; i < cls->num_fields; ++i) {
        auto* f = cls->fields[i];
        if (f && f->name && strcmp(f->name, field_name) == 0) return f->offset;
    }
    return 0;
}

void dump_class(const char* class_name) {
    if (!g_system) return;
    auto client_str = ENC("client.dll");
    auto* scope = find_scope(g_system, client_str.c_str());
    if (!scope) return;
    auto* cls = find_class(scope, class_name);
    if (!cls) return;
    char buf[256];
    snprintf(buf, sizeof(buf), "== %s (%u bytes, %u fields) ==\n",
             cls->name, cls->size, cls->num_fields);
    OutputDebugStringA(buf);
    for (int i = 0; i < cls->num_fields; ++i) {
        auto* f = cls->fields[i];
        if (!f) continue;
        snprintf(buf, sizeof(buf), "  +0x%X  %s\n", f->offset, f->name);
        OutputDebugStringA(buf);
    }
}

} // namespace schema