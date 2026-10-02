#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Call once per frame from wglSwapBuffers hook.
// Initialises ImGui on first call (needs active GL context).
void gui_frame(HDC hdc);
