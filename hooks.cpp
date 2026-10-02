// language: C++17, file: hooks.cpp, target: Windows x64 DLL (opengl32 proxy)
// Defines the five exported symbols that replace their real counterparts.
// No MinHook needed — we ARE opengl32.dll; these are our own exports.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <cstring>
#include <cstdio>
#include <algorithm>

#include "config.h"
#include "gl_proxy.h"
#include "math.h"
#include "gui.h"

// ════════════════════════════════════════════════════════════════════════════
//  INTERNAL HELPERS
// ════════════════════════════════════════════════════════════════════════════

// Typed call shorthands for GLES2 extension pointers
using pfn_glGetUniformLocation = GLint (APIENTRY*)(GLuint, const GLchar*);
using pfn_glUniformMatrix4fv   = void  (APIENTRY*)(GLint, GLsizei, GLboolean, const GLfloat*);

static inline GLint call_GetUniformLocation(GLuint prog, const GLchar* name) {
    auto fn = (pfn_glGetUniformLocation)real_glGetUniformLocation;
    return fn ? fn(prog, name) : -1;
}

// Get the currently bound GLSL program (OpenGL 2.0 way via glGetIntegerv)
static inline GLint current_program() {
    GLint prog = 0;
    real_glGetIntegerv(0x8B8D /*GL_CURRENT_PROGRAM*/, &prog);
    return prog;
}

// ── GL_CURRENT_PROGRAM constant (not always in <GL/gl.h>) ────────────────
#ifndef GL_CURRENT_PROGRAM
#define GL_CURRENT_PROGRAM 0x8B8D
#endif

// ── Viewport helper (x,y,w,h) ────────────────────────────────────────────
static void refresh_viewport() {
    real_glGetIntegerv(GL_VIEWPORT, g_cfg.viewport);
}

// ════════════════════════════════════════════════════════════════════════════
//  HOOKED: glGetUniformLocation
//  Purpose: optional shader name logging; detects target shader in program.
// ════════════════════════════════════════════════════════════════════════════
extern "C" GLint APIENTRY glGetUniformLocation(GLuint program, const GLchar* name)
{
    if (g_cfg.shader_log && name) {
        char buf[256];
        snprintf(buf, sizeof(buf), "[S2] uniform: %s\n", name);
        OutputDebugStringA(buf);
    }
    return call_GetUniformLocation(program, name);
}

// ════════════════════════════════════════════════════════════════════════════
//  HOOKED: glUniformMatrix4fv
//  Purpose: capture the most-recent 4×4 matrix upload — used as our MVP for
//           WorldToScreen. We capture every single-matrix upload and keep the
//           last one before a qualifying draw call. Unity uploads MVP as
//           unity_MatrixMVP or _ProjMatrix on the same program tick.
// ════════════════════════════════════════════════════════════════════════════
extern "C" void APIENTRY glUniformMatrix4fv(GLint location, GLsizei count,
                                             GLboolean transpose,
                                             const GLfloat* value)
{
    auto fn = (pfn_glUniformMatrix4fv)real_glUniformMatrix4fv;
    if (fn) fn(location, count, transpose, value);

    // Capture any single-mat upload as the current MVP candidate.
    // We will use whichever was last set before a player-mesh draw.
    if (count == 1 && value && g_cfg.esp_enabled) {
        memcpy(g_cfg.mvp, value, 16 * sizeof(float));
        g_cfg.mvp_uniform_loc = location;
        g_cfg.mvp_valid.store(true, std::memory_order_relaxed);
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  HOOKED: glDrawElements
//  Purpose: Chams (color + wireframe wallhack) and ESP box accumulation.
//
//  Detection strategy (from Oifox):
//    1. mode == GL_TRIANGLES (no UI quads)
//    2. count >= min_index_count  (skip small props)
//    3. Current GLSL program has our target uniform (e.g. "_Color")
// ════════════════════════════════════════════════════════════════════════════
extern "C" void APIENTRY glDrawElements(GLenum mode, GLsizei count,
                                         GLenum type, const void* indices)
{
    // ── Always draw normally first ────────────────────────────────────────
    real_glDrawElements(mode, count, type, indices);

    if (!g_cfg.chams_enabled && !g_cfg.esp_enabled) return;
    if (mode != GL_TRIANGLES) return;
    if (count < g_cfg.min_index_count) return;

    // ── Shader detection: does the current program use our target uniform? ─
    GLint prog = current_program();
    if (prog == 0) return;
    GLint uid = call_GetUniformLocation(prog, g_cfg.target_shader);
    if (uid == -1) return;

    // ── Chams ─────────────────────────────────────────────────────────────
    if (g_cfg.chams_enabled) {
        // Save minimal state
        GLint prev_dfunc; real_glGetIntegerv(GL_DEPTH_FUNC, &prev_dfunc);
        GLboolean prev_dmask; real_glGetIntegerv(GL_DEPTH_WRITEMASK, (GLint*)&prev_dmask);
        GLboolean blend_was = real_glIsEnabled ? GL_FALSE : GL_FALSE; // evaluated below

        // --- Hidden-through-wall pass (draws behind geometry) ---
        real_glDepthFunc(GL_GREATER);       // pass where depth test would normally FAIL
        real_glDepthMask(GL_FALSE);
        real_glEnable(GL_BLEND);
        real_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (g_cfg.chams_wireframe) {
            real_glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        real_glColorMask(
            (GLboolean)(g_cfg.chams_hid_r > 0.f),
            (GLboolean)(g_cfg.chams_hid_g > 0.f),
            (GLboolean)(g_cfg.chams_hid_b > 0.f),
            GL_TRUE
        );
        real_glDrawElements(mode, count, type, indices);

        // --- Visible pass (normal depth test) ---
        real_glDepthFunc(GL_LEQUAL);
        if (!g_cfg.chams_wireframe) {
            real_glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        real_glColorMask(
            (GLboolean)(g_cfg.chams_vis_r > 0.f),
            (GLboolean)(g_cfg.chams_vis_g > 0.f),
            (GLboolean)(g_cfg.chams_vis_b > 0.f),
            GL_TRUE
        );
        real_glDrawElements(mode, count, type, indices);

        // Restore state
        real_glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        real_glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        real_glDepthFunc((GLenum)prev_dfunc);
        real_glDepthMask(prev_dmask);
        real_glDisable(GL_BLEND);
    }

    // ── ESP: accumulate a box entry using captured MVP ─────────────────────
    if (g_cfg.esp_enabled && g_cfg.mvp_valid.load(std::memory_order_relaxed)) {
        refresh_viewport();

        Mat4 mvp;
        memcpy(mvp.m, g_cfg.mvp, sizeof(mvp.m));

        // Project player model-space top (head ~+0.9 m) and bottom (feet ~-0.9 m)
        float sx_top, sy_top, sx_bot, sy_bot;
        bool top_ok = world_to_screen(mvp, 0.f,  0.9f, 0.f, g_cfg.viewport, sx_top, sy_top);
        bool bot_ok = world_to_screen(mvp, 0.f, -0.9f, 0.f, g_cfg.viewport, sx_bot, sy_bot);

        if (top_ok && bot_ok) {
            // Determine if visible: depth test passing at screen center means
            // the player is currently not occluded. We approximate by checking
            // whether the chams visible pass drew anything — pragmatically we
            // just flag it based on whether glDepthFunc(LEQUAL) would pass.
            // Without a readback, we mark all as potentially through-wall (false).
            EspEntry e;
            e.sx_top = sx_top; e.sy_top = sy_top;
            e.sx_bot = sx_bot; e.sy_bot = sy_bot;
            e.visible = false; // fine-grained occlusion detection needs readback

            std::lock_guard<std::mutex> lk(g_cfg.esp_mutex);
            g_cfg.esp_entries.push_back(e);
        }
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  HOOKED: wglSwapBuffers
//  Purpose: Render ImGui GUI + ESP overlay at end of every frame.
// ════════════════════════════════════════════════════════════════════════════
extern "C" BOOL WINAPI wglSwapBuffers(HDC hdc)
{
    gui_frame(hdc);     // renders ImGui + ESP boxes; no-op until context ready

    // Clear ESP list for next frame after we've drawn it
    {
        std::lock_guard<std::mutex> lk(g_cfg.esp_mutex);
        g_cfg.esp_entries.clear();
    }
    g_cfg.mvp_valid.store(false, std::memory_order_relaxed);

    return real_wglSwapBuffers(hdc);
}

// ════════════════════════════════════════════════════════════════════════════
//  HOOKED: wglGetProcAddress
//  Purpose: Return our hooked version for functions BlueStacks fetches via
//           wglGetProcAddress rather than the static import table.
// ════════════════════════════════════════════════════════════════════════════
extern "C" PROC WINAPI wglGetProcAddress(LPCSTR name)
{
    if (!name) return nullptr;

    // Return our hooked stubs for the functions we intercept
    if (strcmp(name, "glDrawElements")      == 0) return (PROC)glDrawElements;
    if (strcmp(name, "glGetUniformLocation")== 0) return (PROC)glGetUniformLocation;
    if (strcmp(name, "glUniformMatrix4fv")  == 0) return (PROC)glUniformMatrix4fv;
    if (strcmp(name, "wglSwapBuffers")      == 0) return (PROC)wglSwapBuffers;

    return real_wglGetProcAddress(name);
}

void hooks_init() { /* nothing extra needed — we ARE the DLL */ }
