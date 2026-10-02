cmake_minimum_required(VERSION 3.20)
project(S2Cheat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static runtime — no MSVCRT dependency in final DLL
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# ── ImGui via FetchContent ──────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.90.9
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

# ── Our opengl32 proxy DLL ─────────────────────────────────────────────────
add_library(opengl32 SHARED
    src/dllmain.cpp
    src/gl_proxy.cpp
    src/hooks.cpp
    src/gui.cpp

    # ImGui core
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp

    # ImGui backends
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp

    opengl32.def
)

target_include_directories(opengl32 PRIVATE
    src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# Suppress imgui_impl_opengl2's own #pragma comment(lib,"opengl32.lib").
# We ARE opengl32 — we resolve gl* internally via our own stubs.
target_compile_definitions(opengl32 PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
    IMGUI_IMPL_OPENGL_LOADER_CUSTOM   # prevents imgui from pulling in opengl32.lib
)

# Do NOT link opengl32.lib — we are opengl32.dll.
# gl* symbols used by ImGui resolve to our own stubs within this same DLL.
target_link_options(opengl32 PRIVATE
    /NODEFAULTLIB:opengl32.lib
    /NODEFAULTLIB:opengl32    # handles both variants
)

target_link_libraries(opengl32 PRIVATE
    gdi32
    user32
    dwmapi
)

set_target_properties(opengl32 PROPERTIES
    OUTPUT_NAME "opengl32"
    PREFIX      ""
    SUFFIX      ".dll"
)
