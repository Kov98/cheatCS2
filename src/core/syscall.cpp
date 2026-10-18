// syscall.cpp
#include "syscall.h"
#include <unordered_map>
#include <string>
#include <vector>
#include "crypt.h"

namespace sysc {

static std::unordered_map<std::string, void*> g_stubs;
static std::uintptr_t g_ntdll = 0;

// trampoline template:
//   mov r10, rcx           4C 8B D1
//   mov eax, <ssn>         B8 xx xx xx xx
//   jmp qword ptr [rip+0]  FF 25 00 00 00 00 <8-byte addr>
static void* make_stub(std::uint32_t ssn, void* target) {
    auto* mem = static_cast<std::uint8_t*>(
        VirtualAlloc(nullptr, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!mem) return nullptr;
    std::size_t i = 0;
    mem[i++] = 0x4C; mem[i++] = 0x8B; mem[i++] = 0xD1;     // mov r10, rcx
    mem[i++] = 0xB8;                                        // mov eax, ssn
    mem[i++] = static_cast<std::uint8_t>(ssn);
    mem[i++] = static_cast<std::uint8_t>(ssn >> 8);
    mem[i++] = static_cast<std::uint8_t>(ssn >> 16);
    mem[i++] = static_cast<std::uint8_t>(ssn >> 24);
    mem[i++] = 0xFF; mem[i++] = 0x25; mem[i++] = 0; mem[i++] = 0; mem[i++] = 0; mem[i++] = 0;
    std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(target);
    std::memcpy(mem + i, &addr, 8);
    return mem;
}

bool init() {
    g_ntdll = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(ENC("ntdll.dll")));
    if (!g_ntdll) return false;

    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(g_ntdll);
    auto* nt  = reinterpret_cast<IMAGE_NT_HEADERS*>(g_ntdll + dos->e_lfanew);
    auto& exp = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    auto* dir = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(g_ntdll + exp.VirtualAddress);
    auto* names = reinterpret_cast<DWORD*>(g_ntdll + dir->AddressOfNames);
    auto* ords  = reinterpret_cast<WORD*>(g_ntdll + dir->AddressOfNameOrdinals);
    auto* funcs = reinterpret_cast<DWORD*>(g_ntdll + dir->AddressOfFunctions);

    for (DWORD i = 0; i < dir->NumberOfNames; ++i) {
        const char* name = reinterpret_cast<const char*>(g_ntdll + names[i]);
        if (name[0] != 'N' || name[1] != 't') continue;
        auto* fn = reinterpret_cast<std::uint8_t*>(g_ntdll + funcs[ords[i]]);
        // syscall pattern: 4C 8B D1 B8 <ssn 4> ... 0F 05
        if (fn[0] != 0x4C || fn[1] != 0x8B || fn[2] != 0xD1 || fn[3] != 0xB8) continue;
        std::uint32_t ssn = *reinterpret_cast<std::uint32_t*>(fn + 4);
        g_stubs[name] = make_stub(ssn, fn);
    }
    return !g_stubs.empty();
}

void* stub(const char* name) {
    auto it = g_stubs.find(name);
    return it == g_stubs.end() ? nullptr : it->second;
}

using NtProtectFn  = NTSTATUS(NTAPI*)(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);
using NtAllocFn    = NTSTATUS(NTAPI*)(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
using NtFreeFn     = NTSTATUS(NTAPI*)(HANDLE, PVOID*, PSIZE_T, ULONG);
using NtDelayFn    = NTSTATUS(NTAPI*)(BOOLEAN, PLARGE_INTEGER);

NTSTATUS nt_protect(void* addr, size_t size, DWORD new_prot, DWORD* old_prot) {
    auto* s = stub(ENC("NtProtectVirtualMemory").c_str());
    if (!s) return -1;
    PVOID base = addr; SIZE_T sz = size;
    return reinterpret_cast<NtProtectFn>(s)(GetCurrentProcess(), &base, &sz, new_prot, old_prot);
}

NTSTATUS nt_alloc(void** out, size_t size, DWORD protect) {
    auto* s = stub(ENC("NtAllocateVirtualMemory").c_str());
    if (!s) return -1;
    PVOID base = nullptr; SIZE_T sz = size;
    NTSTATUS st = reinterpret_cast<NtAllocFn>(s)(GetCurrentProcess(), &base, 0, &sz,
                                                 MEM_COMMIT | MEM_RESERVE, protect);
    if (st == 0) *out = base;
    return st;
}

NTSTATUS nt_free(void* addr) {
    auto* s = stub(ENC("NtFreeVirtualMemory").c_str());
    if (!s) return -1;
    PVOID base = addr; SIZE_T sz = 0;
    return reinterpret_cast<NtFreeFn>(s)(GetCurrentProcess(), &base, &sz, MEM_RELEASE);
}

NTSTATUS nt_delay(long long* interval) {
    auto* s = stub(ENC("NtDelayExecution").c_str());
    if (!s) return -1;
    LARGE_INTEGER li; li.QuadPart = *interval;
    return reinterpret_cast<NtDelayFn>(s)(FALSE, &li);
}

} // namespace sysc