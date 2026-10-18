// process.cpp
#include "process.h"
#include <cstring>
#include <cctype>

namespace proc {

std::uintptr_t module_base(std::uintptr_t module_handle) {
#if defined(_M_X64)
    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(module_handle);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return 0;
    auto* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(module_handle + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return 0;
    return module_handle;
#else
    return 0;
#endif
}

std::uintptr_t module_base(const std::wstring& name) {
    auto* peb = reinterpret_cast<PEB*>(__readgsqword(0x60));
    auto* ldr = peb->Ldr;
    if (!ldr) return 0;
    for (auto* e = ldr->InLoadOrderModuleList.Flink;
         e != &ldr->InLoadOrderModuleList; e = e->Flink) {
        auto* entry = CONTAINING_RECORD(e, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        if (entry->BaseDllName.Buffer &&
            _wcsicmp(entry->BaseDllName.Buffer, name.c_str()) == 0) {
            return reinterpret_cast<std::uintptr_t>(entry->DllBase);
        }
    }
    return 0;
}

template<typename T>
std::optional<T> safe_read(std::uintptr_t addr) {
    if (!addr) return std::nullopt;
    __try {
        return *reinterpret_cast<T*>(addr);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return std::nullopt;
    }
}

template std::optional<std::uint8_t>  safe_read<std::uint8_t>(std::uintptr_t);
template std::optional<std::uint16_t> safe_read<std::uint16_t>(std::uintptr_t);
template std::optional<std::uint32_t> safe_read<std::uint32_t>(std::uintptr_t);
template std::optional<std::uint64_t> safe_read<std::uint64_t>(std::uintptr_t);
template std::optional<std::int32_t>  safe_read<std::int32_t>(std::uintptr_t);
template std::optional<float>         safe_read<float>(std::uintptr_t);

std::optional<std::uintptr_t> safe_read_ptr(std::uintptr_t addr) {
    return safe_read<std::uintptr_t>(addr);
}

template<typename T>
bool safe_write(std::uintptr_t addr, T val) {
    if (!addr) return false;
    __try {
        *reinterpret_cast<T*>(addr) = val;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

template bool safe_write<std::uintptr_t>(std::uintptr_t, std::uintptr_t);
template bool safe_write<std::int32_t>(std::uintptr_t, std::int32_t);
template bool safe_write<float>(std::uintptr_t, float);
template bool safe_write<std::uint8_t>(std::uintptr_t, std::uint8_t);

std::uintptr_t pattern_scan(std::uintptr_t module_base, const char* pattern) {
    if (!module_base) return 0;
    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(module_base);
    auto* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(module_base + dos->e_lfanew);

    std::vector<int> pat_bytes;
    std::vector<bool> wild;
    {
        const char* p = pattern;
        while (*p) {
            if (*p == '?') {
                pat_bytes.push_back(0); wild.push_back(true);
                while (*p == '?') ++p;
                while (*p == ' ') ++p;
            } else if (std::isxdigit((unsigned char)*p)) {
                int b = 0;
                for (int i = 0; i < 2 && std::isxdigit((unsigned char)*p); ++i, ++p) {
                    char c = *p;
                    int v = (c >= '0' && c <= '9') ? c - '0'
                          : (c >= 'a' && c <= 'f') ? c - 'a' + 10
                          : c - 'A' + 10;
                    b = (b << 4) | v;
                }
                pat_bytes.push_back(b); wild.push_back(false);
                while (*p == ' ') ++p;
            } else ++p;
        }
    }
    if (pat_bytes.empty()) return 0;

    auto* sec = IMAGE_FIRST_SECTION(nt);
    for (int i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec) {
        if (!(sec->Characteristics & IMAGE_SCN_MEM_EXECUTE)) continue;
        auto* start = reinterpret_cast<std::uint8_t*>(module_base + sec->VirtualAddress);
        std::size_t size = sec->Misc.VirtualSize;
        for (std::size_t j = 0; j + pat_bytes.size() <= size; ++j) {
            bool ok = true;
            for (std::size_t k = 0; k < pat_bytes.size(); ++k) {
                if (!wild[k] && start[j + k] != (std::uint8_t)pat_bytes[k]) { ok = false; break; }
            }
            if (ok) return reinterpret_cast<std::uintptr_t>(start + j);
        }
    }
    return 0;
}

std::uintptr_t resolve_rip(std::uintptr_t instr, int rel_offset, int instr_len) {
    auto rel_opt = safe_read<std::int32_t>(instr + rel_offset);
    if (!rel_opt) return 0;
    return instr + instr_len + static_cast<std::int32_t>(*rel_opt);
}

} // namespace proc