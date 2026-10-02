// language: C++17, file: gui.cpp, target: Windows x64 / BlueStacks
// ImGui GUI rendered on top of the game via wglSwapBuffers hook.
// INSERT   → toggle GUI window
// DELETE   → toggle chams
// HOME     → toggle ESP
#include <windows.h>
#include <GL/gl.h>
#include <cstring>
#include <mutex>
#include <vector>

#include "imgui.h"
#include "imgui_impl_opengl2.h"
#include "imgui_impl_win32.h"

#include "config.h"
#include "gl_proxy.h"

// ── Win32 message hook so ImGui receives keyboard/mouse input ───────────────
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static HWND       g_hwnd     = nullptr;
static WNDPROC    g_orig_wnd = nullptr;
static bool       g_inited   = false;

static LRESULT CALLBACK wnd_hook(HWND hWnd, UINT msg, WPARAM wP, LPARAM lP) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wP, lP)) return TRUE;
    return CallWindowProcW(g_orig_wnd, hWnd, msg, wP, lP);
}

// ── Hotkey helper ────────────────────────────────────────────────────────────
static bool key_just_down(int vk) {
    static SHORT prev[256] = {};
    SHORT cur = GetAsyncKeyState(vk);
    bool pressed = (cur & 0x8000) && !(prev[vk & 0xFF] & 0x8000);
    prev[vk & 0xFF] = cur;
    return pressed;
}

// ── Save / restore GL state around ImGui rendering ──────────────────────────
struct GlState {
    GLint   blend_src, blend_dst, blend_eq;
    GLint   viewport[4];
    GLint   scissor_box[4];
    GLint   matrix_mode;
    GLboolean blend, scissor, depth_test, cull_face;

    void save() {
        real_glGetIntegerv(GL_BLEND_SRC,  &blend_src);
        real_glGetIntegerv(GL_BLEND_DST,  &blend_dst);
        // GL_BLEND_EQUATION may not be in the header; use numeric constant
        real_glGetIntegerv(0x8009, &blend_eq);
        real_glGetIntegerv(GL_VIEWPORT,   viewport);
        real_glGetIntegerv(GL_SCISSOR_BOX, scissor_box);
        real_glGetIntegerv(GL_MATRIX_MODE, &matrix_mode);
        blend      = real_glIsEnabled ? GL_TRUE : GL_FALSE; // approximate
        depth_test = GL_FALSE;
        cull_face  = GL_FALSE;
    }
    void restore() {
        real_glBlendFunc((GLenum)blend_src, (GLenum)blend_dst);
        real_glBlendEquation((GLenum)blend_eq);
        real_glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        real_glScissor(scissor_box[0], scissor_box[1], scissor_box[2], scissor_box[3]);
        real_glMatrixMode((GLenum)matrix_mode);
        real_glDisable(GL_DEPTH_TEST);
        real_glDisable(GL_CULL_FACE);
        real_glEnable(GL_BLEND);
    }
};

// ── Draw ESP boxes via ImGui DrawList ─────────────────────────────────────
static void draw_esp() {
    if (!g_cfg.esp_enabled) return;

    std::lock_guard<std::mutex> lk(g_cfg.esp_mutex);
    if (g_cfg.esp_entries.empty()) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImU32 col = IM_COL32(
        (int)(g_cfg.esp_r * 255),
        (int)(g_cfg.esp_g * 255),
        (int)(g_cfg.esp_b * 255),
        220
    );
    ImU32 col_line = IM_COL32(
        (int)(g_cfg.esp_r * 255),
        (int)(g_cfg.esp_g * 255),
        (int)(g_cfg.esp_b * 255),
        128
    );

    for (const auto& e : g_cfg.esp_entries) {
        float h = e.sy_bot - e.sy_top;
        if (h < 4.f || h > 2000.f) continue;  // sanity guard
        float w = h * 0.4f;                     // aspect ~0.4 for a player

        float x0 = e.sx_top - w * 0.5f;
        float x1 = e.sx_top + w * 0.5f;
        float y0 = e.sy_top;
        float y1 = e.sy_bot;

        // Corner-tick box (cleaner look than full rect)
        float tk = w * 0.3f; // tick length
        float tv = h * 0.15f;

        // Top-left
        dl->AddLine({x0, y0}, {x0 + tk, y0}, col, 1.5f);
        dl->AddLine({x0, y0}, {x0, y0 + tv}, col, 1.5f);
        // Top-right
        dl->AddLine({x1, y0}, {x1 - tk, y0}, col, 1.5f);
        dl->AddLine({x1, y0}, {x1, y0 + tv}, col, 1.5f);
        // Bottom-left
        dl->AddLine({x0, y1}, {x0 + tk, y1}, col, 1.5f);
        dl->AddLine({x0, y1}, {x0, y1 - tv}, col, 1.5f);
        // Bottom-right
        dl->AddLine({x1, y1}, {x1 - tk, y1}, col, 1.5f);
        dl->AddLine({x1, y1}, {x1, y1 - tv}, col, 1.5f);

        // Optional snapline to screen bottom center
        if (g_cfg.esp_snaplines) {
            ImVec2 disp_sz = ImGui::GetIO().DisplaySize;
            dl->AddLine({disp_sz.x * 0.5f, disp_sz.y},
                        {(x0 + x1) * 0.5f, y1},
                        col_line, 1.f);
        }
    }
}

// ── Main GUI window ──────────────────────────────────────────────────────────
static void draw_gui_window() {
    if (!g_cfg.gui_open) return;

    ImGui::SetNextWindowSize({380, 420}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({20, 20}, ImGuiCond_Once);

    ImGui::Begin("S2 Cheat  |  INSERT to hide", &g_cfg.gui_open,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

    // ── Chams section ─────────────────────────────────────────────────────
    ImGui::SeparatorText("CHAMS");
    ImGui::Checkbox("Enable Chams", &g_cfg.chams_enabled);
    if (g_cfg.chams_enabled) {
        ImGui::Indent(12.f);
        ImGui::Checkbox("Wireframe mode", &g_cfg.chams_wireframe);

        ImGui::Text("Visible color");
        float vis[3] = {g_cfg.chams_vis_r, g_cfg.chams_vis_g, g_cfg.chams_vis_b};
        if (ImGui::ColorEdit3("##vis", vis))
            { g_cfg.chams_vis_r=vis[0]; g_cfg.chams_vis_g=vis[1]; g_cfg.chams_vis_b=vis[2]; }

        ImGui::Text("Hidden (through wall)");
        float hid[3] = {g_cfg.chams_hid_r, g_cfg.chams_hid_g, g_cfg.chams_hid_b};
        if (ImGui::ColorEdit3("##hid", hid))
            { g_cfg.chams_hid_r=hid[0]; g_cfg.chams_hid_g=hid[1]; g_cfg.chams_hid_b=hid[2]; }
        ImGui::Unindent(12.f);
    }

    // ── ESP section ───────────────────────────────────────────────────────
    ImGui::SeparatorText("ESP");
    ImGui::Checkbox("Enable ESP boxes", &g_cfg.esp_enabled);
    if (g_cfg.esp_enabled) {
        ImGui::Indent(12.f);
        ImGui::Checkbox("Snaplines", &g_cfg.esp_snaplines);
        float ec[3] = {g_cfg.esp_r, g_cfg.esp_g, g_cfg.esp_b};
        if (ImGui::ColorEdit3("Box color##esp", ec))
            { g_cfg.esp_r=ec[0]; g_cfg.esp_g=ec[1]; g_cfg.esp_b=ec[2]; }
        ImGui::Unindent(12.f);
    }

    // ── Detection tuning ──────────────────────────────────────────────────
    ImGui::SeparatorText("DETECTION");
    ImGui::InputText("Target uniform", g_cfg.target_shader,
                     sizeof(g_cfg.target_shader));
    ImGui::SliderInt("Min index count", &g_cfg.min_index_count, 100, 5000);
    ImGui::Checkbox("Log all uniforms (OutputDebugString)", &g_cfg.shader_log);

    // ── Hotkeys info ──────────────────────────────────────────────────────
    ImGui::SeparatorText("HOTKEYS");
    ImGui::TextDisabled("INSERT  toggle this window");
    ImGui::TextDisabled("DELETE  toggle chams");
    ImGui::TextDisabled("HOME    toggle ESP");

    // ── Status bar ────────────────────────────────────────────────────────
    ImGui::Separator();
    {
        std::lock_guard<std::mutex> lk(g_cfg.esp_mutex);
        ImGui::TextColored({0.5f,1.f,0.5f,1.f},
            "Players this frame: %d", (int)g_cfg.esp_entries.size());
    }

    ImGui::End();
}

// ════════════════════════════════════════════════════════════════════════════
//  PUBLIC: called every frame from wglSwapBuffers hook
// ════════════════════════════════════════════════════════════════════════════
void gui_frame(HDC hdc)
{
    // ── One-time init (needs an active GL context) ────────────────────────
    if (!g_inited) {
        g_hwnd = WindowFromDC(hdc);
        if (!g_hwnd) return;  // context not ready yet

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr; // no imgui.ini file

        // Style
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding    = 6.f;
        style.FrameRounding     = 4.f;
        style.GrabRounding      = 3.f;
        style.ScrollbarRounding = 4.f;
        style.Colors[ImGuiCol_WindowBg]    = ImVec4(0.08f,0.08f,0.10f,0.90f);
        style.Colors[ImGuiCol_TitleBgActive]= ImVec4(0.15f,0.15f,0.20f,1.f);

        ImGui_ImplWin32_Init(g_hwnd);
        ImGui_ImplOpenGL2_Init();

        // Subclass the window so ImGui gets mouse/keyboard events
        g_orig_wnd = (WNDPROC)SetWindowLongPtrW(
            g_hwnd, GWLP_WNDPROC, (LONG_PTR)wnd_hook);

        g_inited = true;
    }

    // ── Hotkeys (polled each frame) ───────────────────────────────────────
    if (key_just_down(VK_INSERT)) g_cfg.gui_open   = !g_cfg.gui_open;
    if (key_just_down(VK_DELETE)) g_cfg.chams_enabled = !g_cfg.chams_enabled;
    if (key_just_down(VK_HOME))   g_cfg.esp_enabled   = !g_cfg.esp_enabled;

    // ── Save GL state (ImGui will clobber blending/matrices) ─────────────
    GlState gs;
    gs.save();
    real_glDisable(GL_DEPTH_TEST);
    real_glDisable(GL_CULL_FACE);

    // ── ImGui frame ───────────────────────────────────────────────────────
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    draw_esp();
    draw_gui_window();

    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    // ── Restore GL state ──────────────────────────────────────────────────
    gs.restore();
}
