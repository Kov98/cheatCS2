#pragma once
#include <string>
#include <cstdint>
#include <array>

template<std::size_t N>
struct EncStr {
    std::array<char, N> data{};
    constexpr EncStr(const char (&s)[N]) {
        for (std::size_t i = 0; i < N; ++i)
            data[i] = static_cast<char>(s[i] ^ (0xA5 + i * 37));
    }
    std::string dec() const {
        std::string out(N - 1, 0);
        for (std::size_t i = 0; i < N - 1; ++i)
            out[i] = static_cast<char>(data[i] ^ (0xA5 + i * 37));
        return out;
    }
};

template<std::size_t N>
struct EncWStr {
    std::array<wchar_t, N> data{};
    constexpr EncWStr(const wchar_t (&s)[N]) {
        for (std::size_t i = 0; i < N; ++i)
            data[i] = static_cast<wchar_t>(s[i] ^ (0x5A3 + i * 137));
    }
    std::wstring dec() const {
        std::wstring out(N - 1, 0);
        for (std::size_t i = 0; i < N - 1; ++i)
            out[i] = static_cast<wchar_t>(data[i] ^ (0x5A3 + i * 137));
        return out;
    }
};

#define ENC(s)  EncStr<sizeof(s)>(s).dec()
#define ENC_W(s) EncWStr<sizeof(s) / sizeof(wchar_t)>(s).dec()