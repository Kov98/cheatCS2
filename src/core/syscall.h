// syscall.h
#pragma once
#include <windows.h>
#include <cstdint>

namespace sysc {
    bool init();  // parse ntdll, build stubs
    void* stub(const char* name); // return executable stub for "NtXxx"

    // convenience
    NTSTATUS nt_protect(void* addr, size_t size, DWORD new_prot, DWORD* old_prot);
    NTSTATUS nt_alloc(void** out, size_t size, DWORD protect);
    NTSTATUS nt_free(void* addr);
    NTSTATUS nt_delay(long long* interval);
}