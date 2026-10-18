// config.cpp
#include "config.h"
#include <fstream>
#include "json.hpp"
#include "features/aimbot.h"
#include "features/triggerbot.h"
#include "features/esp.h"
#include "features/radar.h"

using nlohmann::json;

namespace config {

void save(const std::string& path) {
    json j;
    {
        auto& a = features::g_aimbot;
        j["aimbot"] = {
            {"enabled", a.enabled}, {"ignore_vis", a.ignore_vis},
            {"sticky", a.sticky}, {"fov_deg", a.fov_deg},
            {"smooth", a.smooth}, {"bone_idx", a.bone_idx},
            {"rcs_enabled", a.rcs_enabled},
            {"rcs_pitch_scale", a.rcs_pitch_scale},
            {"rcs_yaw_scale", a.rcs_yaw_scale},
            {"prediction", a.prediction},
            {"prediction_scale", a.prediction_scale},
        };
    }
    {
        auto& t = features::g_trigger;
        j["triggerbot"] = {
            {"enabled", t.enabled}, {"always_on", t.always_on},
            {"key", t.key}, {"delay_min_ms", t.delay_min_ms},
            {"delay_max_ms", t.delay_max_ms}, {"vis_only", t.vis_only},
            {"anti_flash", t.anti_flash}, {"hitbox", t.hitbox},
        };
    }
    {
        auto& e = features::g_esp;
        j["esp"] = {
            {"enabled", e.enabled}, {"boxes", e.boxes},
            {"skeleton", e.skeleton}, {"health", e.health},
            {"names", e.names}, {"weapon", e.weapon},
            {"distance", e.distance}, {"vis_only", e.vis_only},
            {"max_dist_m", e.max_dist_m},
        };
    }
    {
        auto& r = features::g_radar;
        j["radar"] = {
            {"enabled", r.enabled}, {"range_m", r.range_m},
            {"size", r.size},
        };
    }
    std::ofstream f(path);
    if (f) f << j.dump(2);
}

void load(const std::string& path) {
    std::ifstream f(path);
    if (!f) return;
    json j;
    try { f >> j; } catch (...) { return; }

    if (j.contains("aimbot")) {
        auto& s = j["aimbot"];
        auto& a = features::g_aimbot;
        a.enabled = s.value("enabled", a.enabled);
        a.ignore_vis = s.value("ignore_vis", a.ignore_vis);
        a.sticky = s.value("sticky", a.sticky);
        a.fov_deg = s.value("fov_deg", a.fov_deg);
        a.smooth = s.value("smooth", a.smooth);
        a.bone_idx = s.value("bone_idx", a.bone_idx);
        a.rcs_enabled = s.value("rcs_enabled", a.rcs_enabled);
        a.rcs_pitch_scale = s.value("rcs_pitch_scale", a.rcs_pitch_scale);
        a.rcs_yaw_scale = s.value("rcs_yaw_scale", a.rcs_yaw_scale);
        a.prediction = s.value("prediction", a.prediction);
        a.prediction_scale = s.value("prediction_scale", a.prediction_scale);
    }
    if (j.contains("triggerbot")) {
        auto& s = j["triggerbot"];
        auto& t = features::g_trigger;
        t.enabled = s.value("enabled", t.enabled);
        t.always_on = s.value("always_on", t.always_on);
        t.key = s.value("key", t.key);
        t.delay_min_ms = s.value("delay_min_ms", t.delay_min_ms);
        t.delay_max_ms = s.value("delay_max_ms", t.delay_max_ms);
        t.vis_only = s.value("vis_only", t.vis_only);
        t.anti_flash = s.value("anti_flash", t.anti_flash);
        t.hitbox = s.value("hitbox", t.hitbox);
    }
    if (j.contains("esp")) {
        auto& s = j["esp"];
        auto& e = features::g_esp;
        e.enabled = s.value("enabled", e.enabled);
        e.boxes = s.value("boxes", e.boxes);
        e.skeleton = s.value("skeleton", e.skeleton);
        e.health = s.value("health", e.health);
        e.names = s.value("names", e.names);
        e.weapon = s.value("weapon", e.weapon);
        e.distance = s.value("distance", e.distance);
        e.vis_only = s.value("vis_only", e.vis_only);
        e.max_dist_m = s.value("max_dist_m", e.max_dist_m);
    }
    if (j.contains("radar")) {
        auto& s = j["radar"];
        auto& r = features::g_radar;
        r.enabled = s.value("enabled", r.enabled);
        r.range_m = s.value("range_m", r.range_m);
        r.size = s.value("size", r.size);
    }
}

} // namespace config