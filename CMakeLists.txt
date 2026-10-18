cmake_minimum_required(VERSION 3.20)
project(cs2ext LANGUAGES CXX ASM_MASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT MSVC)
    message(FATAL_ERROR "MSVC 2022 required")
endif()

# --- deps ---
set(IMGUI_DIR  "${CMAKE_SOURCE_DIR}/external/imgui")
set(MINHOOK_DIR "${CMAKE_SOURCE_DIR}/external/minhook")

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
)
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CMAKE_SOURCE_DIR}/external/json
)

add_subdirectory(${MINHOOK_DIR} minhook_build)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

add_library(cs2ext SHARED ${SRC_FILES})
target_include_directories(cs2ext PRIVATE
    src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${MINHOOK_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/json
)
target_link_libraries(cs2ext PRIVATE
    imgui
    minhook
    d3d11 dxgi d2d1 dwrite d3dcompiler
    user32 gdi32 ole32
)
target_compile_options(cs2ext PRIVATE /O2 /Oi /Gy /GL /MT /EHsc /W3 /permissive- /Zc:inline /std:c++20)
target_link_options(cs2ext PRIVATE /LTCG)

set_target_properties(cs2ext PROPERTIES
    OUTPUT_NAME "cs2ext"
    PREFIX ""
    SUFFIX ".dll"
)