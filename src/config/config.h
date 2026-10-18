// config.h
#pragma once
#include <string>

namespace config {
    void save(const std::string& path);
    void load(const std::string& path);
}