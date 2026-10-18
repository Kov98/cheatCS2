// process.h
#pragma once
#include <windows.h>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace proc {
    // module base from PEB LoaderData walk
    std::uintptr_t module_base(std::uintptr_t module_handle);
    std::uintptr_t module_base(const std::wstring& name);

    // safe reads via SEH — return nullopt on fault
    template<typename T>
    std::optional<T> safe_read(std::uintptr_t addr);
    std::optional<std::uintptr_t> safe_read_ptr(std::uintptr_t addr);

    template<typename T>
    bool safe_write(std::uintptr_t addr, T val);

    // pattern scan a module's .text. format: "48 8B 05 ? ? ? ?"
    std::uintptr_t pattern_scan(std::uintptr_t module_base, const char* pattern);
    // resolve rip-relative: addr points to instruction start; rel_offset is position in instruction; instr_len total
    std::uintptr_t resolve_rip(std::uintptr_t instr_addr, int rel_offset, int instr_len);
}