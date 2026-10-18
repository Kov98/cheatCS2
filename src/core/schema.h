// schema.h
#pragma once
#include <cstdint>
#include <string>

namespace schema {
    bool init();
    // lookup a field offset in a class by name. 0 on failure.
    std::uint32_t find_field(const char* class_name, const char* field_name);
    void dump_class(const char* class_name);
}