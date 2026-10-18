# cs2ext — internal CS2 cheat

## Arquitetura

Opção B: internal DLL, manual-mapped, Present hook (MinHook) + ImGui.
Direto acesso à memória — nenhum ReadProcessMemory no IAT (assinatura clássica de external).
Hook no Present, vtable-driven. Módulo escondido pelo mapper (não usa LoadLibrary).

## Build

Pré-requisitos:
- Visual Studio 2022 (MSVC x64, C++20)
- CMake ≥ 3.20
- git

```bash
git clone https://github.com/ocornut/imgui external/imgui
git clone https://github.com/TsudaKage/MHook external/minhook
# coloque nlohmann/json.hpp em external/json/
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
