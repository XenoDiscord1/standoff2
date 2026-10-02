#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <vector>
#include <mutex>
#include <atomic>

// ── Per-frame ESP entry accumulated during glDrawElements ─────────────────
struct EspEntry {
    float sx_top, sy_top;   // head (screen)
    float sx_bot, sy_bot;   // feet (screen)
    bool  visible;          // was depth-test passing? (through-wall flag)
};

// ── Global configuration (GUI writes, hooks read) ─────────────────────────
struct Config {
    // --- Feature toggles ---
    bool chams_enabled      = false;
    bool chams_wireframe    = false;  // solid color vs wireframe
    bool esp_enabled        = false;
    bool esp_snaplines      = false;
    bool gui_open           = true;

    // --- Chams colors (visible / hidden-through-wall) ---
    float chams_vis_r = 0.0f, chams_vis_g = 1.0f, chams_vis_b = 0.0f;
    float chams_hid_r = 1.0f, chams_hid_g = 0.0f, chams_hid_b = 0.0f;

    // --- ESP box color ---
    float esp_r = 1.0f, esp_g = 0.85f, esp_b = 0.0f;

    // --- Detection tuning ---
    // Shader uniform name that identifies enemy player mesh.
    // Run with shader_log = true first to find the right name for your build.
    char target_shader[64] = "_Color";
    int  min_index_count   = 900;   // skip small geometry (UI, props)
    bool shader_log        = false; // log every uniform to OutputDebugString

    // --- ESP frame data (filled by glDrawElements, read by wglSwapBuffers) -
    std::vector<EspEntry> esp_entries;
    std::mutex            esp_mutex;

    // --- Captured MVP for W2S -----------------------------------------------
    float mvp[16]           = {};
    GLint mvp_uniform_loc   = -1;
    std::atomic<bool> mvp_valid { false };

    // --- Viewport (updated each frame) ---------------------------------------
    GLint viewport[4]       = {}; // x, y, w, h
};

inline Config g_cfg;
