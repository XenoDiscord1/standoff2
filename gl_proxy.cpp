// language: C++17, file: gl_proxy.cpp
// Loads the real opengl32.dll from System32 and forwards every exported call.
// Hooks for chams/ESP/GUI live in hooks.cpp and replace select functions.
#include <windows.h>
#include <GL/gl.h>
#include <cstring>
#include "gl_proxy.h"
#include <cstddef>

// Missing types from legacy Windows GL/gl.h (needed for GLES-style stubs)
#ifndef GLchar
typedef char GLchar;
#endif
#ifndef GLsizeiptr
typedef ptrdiff_t GLsizeiptr;
#endif
#ifndef GLintptr
typedef ptrdiff_t GLintptr;
#endif

#pragma warning(push)
#pragma warning(disable: 4273) // inconsistent dll linkage — intentional re-export
#pragma warning(disable: 4005) // macro redefinition

// ── Helper macro: resolve a single symbol from the real DLL ──────────────
#define RESOLVE(name) \
    real_##name = (decltype(real_##name))GetProcAddress(hReal, #name)

#define RESOLVE_EXT(name) \
    real_##name = (void*)real_wglGetProcAddress(#name); \
    if (!real_##name) real_##name = (void*)GetProcAddress(hReal, #name)

static HMODULE hReal = nullptr;

// ── All real-function-pointer definitions ─────────────────────────────────
extern "C" {

BOOL  (WINAPI* real_wglSwapBuffers)(HDC)                            = nullptr;
HGLRC (WINAPI* real_wglCreateContext)(HDC)                          = nullptr;
BOOL  (WINAPI* real_wglMakeCurrent)(HDC, HGLRC)                    = nullptr;
BOOL  (WINAPI* real_wglDeleteContext)(HGLRC)                        = nullptr;
HGLRC (WINAPI* real_wglGetCurrentContext)()                         = nullptr;
HDC   (WINAPI* real_wglGetCurrentDC)()                              = nullptr;
PROC  (WINAPI* real_wglGetProcAddress)(LPCSTR)                      = nullptr;
int   (WINAPI* real_wglChoosePixelFormat)(HDC, const PIXELFORMATDESCRIPTOR*)  = nullptr;
int   (WINAPI* real_wglDescribePixelFormat)(HDC, int, UINT, LPPIXELFORMATDESCRIPTOR) = nullptr;
BOOL  (WINAPI* real_wglSetPixelFormat)(HDC, int, const PIXELFORMATDESCRIPTOR*) = nullptr;
int   (WINAPI* real_wglGetPixelFormat)(HDC)                        = nullptr;
BOOL  (WINAPI* real_wglShareLists)(HGLRC, HGLRC)                   = nullptr;

void   (APIENTRY* real_glBegin)(GLenum)                             = nullptr;
void   (APIENTRY* real_glEnd)()                                     = nullptr;
void   (APIENTRY* real_glVertex2f)(GLfloat, GLfloat)               = nullptr;
void   (APIENTRY* real_glTexCoord2f)(GLfloat, GLfloat)             = nullptr;
void   (APIENTRY* real_glColor4f)(GLfloat,GLfloat,GLfloat,GLfloat) = nullptr;
void   (APIENTRY* real_glColor4ub)(GLubyte,GLubyte,GLubyte,GLubyte)= nullptr;
void   (APIENTRY* real_glPushMatrix)()                              = nullptr;
void   (APIENTRY* real_glPopMatrix)()                               = nullptr;
void   (APIENTRY* real_glPushAttrib)(GLbitfield)                    = nullptr;
void   (APIENTRY* real_glPopAttrib)()                               = nullptr;
void   (APIENTRY* real_glPushClientAttrib)(GLbitfield)              = nullptr;
void   (APIENTRY* real_glPopClientAttrib)()                         = nullptr;
void   (APIENTRY* real_glEnable)(GLenum)                            = nullptr;
void   (APIENTRY* real_glDisable)(GLenum)                           = nullptr;
void   (APIENTRY* real_glEnableClientState)(GLenum)                 = nullptr;
void   (APIENTRY* real_glDisableClientState)(GLenum)                = nullptr;
void   (APIENTRY* real_glBlendFunc)(GLenum, GLenum)                 = nullptr;
void   (APIENTRY* real_glBlendEquation)(GLenum)                     = nullptr;
void   (APIENTRY* real_glScissor)(GLint, GLint, GLsizei, GLsizei)  = nullptr;
void   (APIENTRY* real_glViewport)(GLint, GLint, GLsizei, GLsizei) = nullptr;
void   (APIENTRY* real_glGetIntegerv)(GLenum, GLint*)               = nullptr;
void   (APIENTRY* real_glGetFloatv)(GLenum, GLfloat*)               = nullptr;
void   (APIENTRY* real_glPolygonMode)(GLenum, GLenum)               = nullptr;
void   (APIENTRY* real_glDepthFunc)(GLenum)                         = nullptr;
void   (APIENTRY* real_glDepthMask)(GLboolean)                      = nullptr;
void   (APIENTRY* real_glDepthRange)(GLclampd, GLclampd)            = nullptr;
void   (APIENTRY* real_glColorMask)(GLboolean,GLboolean,GLboolean,GLboolean) = nullptr;
void   (APIENTRY* real_glStencilMask)(GLuint)                       = nullptr;
void   (APIENTRY* real_glClear)(GLbitfield)                         = nullptr;
void   (APIENTRY* real_glClearColor)(GLfloat,GLfloat,GLfloat,GLfloat) = nullptr;
void   (APIENTRY* real_glClearDepth)(GLclampd)                      = nullptr;
void   (APIENTRY* real_glBindTexture)(GLenum, GLuint)               = nullptr;
void   (APIENTRY* real_glGenTextures)(GLsizei, GLuint*)             = nullptr;
void   (APIENTRY* real_glDeleteTextures)(GLsizei, const GLuint*)    = nullptr;
void   (APIENTRY* real_glTexImage2D)(GLenum,GLint,GLint,GLsizei,GLsizei,GLint,GLenum,GLenum,const void*) = nullptr;
void   (APIENTRY* real_glTexSubImage2D)(GLenum,GLint,GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,const void*) = nullptr;
void   (APIENTRY* real_glTexParameteri)(GLenum, GLenum, GLint)      = nullptr;
void   (APIENTRY* real_glTexParameterf)(GLenum, GLenum, GLfloat)    = nullptr;
void   (APIENTRY* real_glPixelStorei)(GLenum, GLint)                = nullptr;
void   (APIENTRY* real_glMatrixMode)(GLenum)                        = nullptr;
void   (APIENTRY* real_glLoadIdentity)()                            = nullptr;
void   (APIENTRY* real_glOrtho)(GLdouble,GLdouble,GLdouble,GLdouble,GLdouble,GLdouble) = nullptr;
void   (APIENTRY* real_glReadPixels)(GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,void*) = nullptr;
void   (APIENTRY* real_glDrawArrays)(GLenum, GLint, GLsizei)        = nullptr;
void   (APIENTRY* real_glDrawElements)(GLenum,GLsizei,GLenum,const void*) = nullptr;
void   (APIENTRY* real_glLineWidth)(GLfloat)                        = nullptr;
void   (APIENTRY* real_glPointSize)(GLfloat)                        = nullptr;
void   (APIENTRY* real_glShadeModel)(GLenum)                        = nullptr;
void   (APIENTRY* real_glAlphaFunc)(GLenum, GLclampf)               = nullptr;
const GLubyte* (APIENTRY* real_glGetString)(GLenum)                 = nullptr;
GLenum (APIENTRY* real_glGetError)()                                = nullptr;
void   (APIENTRY* real_glFinish)()                                  = nullptr;
void   (APIENTRY* real_glFlush)()                                   = nullptr;

// Extension pointers (void* — cast at call site)
void* real_glUseProgram              = nullptr;
void* real_glGetUniformLocation      = nullptr;
void* real_glUniformMatrix4fv        = nullptr;
void* real_glGetActiveUniform        = nullptr;
void* real_glCreateProgram           = nullptr;
void* real_glDeleteProgram           = nullptr;
void* real_glCreateShader            = nullptr;
void* real_glDeleteShader            = nullptr;
void* real_glShaderSource            = nullptr;
void* real_glCompileShader           = nullptr;
void* real_glAttachShader            = nullptr;
void* real_glLinkProgram             = nullptr;
void* real_glGetProgramiv            = nullptr;
void* real_glGetShaderiv             = nullptr;
void* real_glEnableVertexAttribArray = nullptr;
void* real_glDisableVertexAttribArray= nullptr;
void* real_glVertexAttribPointer     = nullptr;
void* real_glGenBuffers              = nullptr;
void* real_glDeleteBuffers           = nullptr;
void* real_glBindBuffer              = nullptr;
void* real_glBufferData              = nullptr;
void* real_glActiveTexture           = nullptr;
void* real_glBlendFuncSeparate       = nullptr;
void* real_glBlendEquationSeparate   = nullptr;
void* real_glGenerateMipmap          = nullptr;
void* real_glGenFramebuffers         = nullptr;
void* real_glDeleteFramebuffers      = nullptr;
void* real_glBindFramebuffer         = nullptr;
void* real_glFramebufferTexture2D    = nullptr;
void* real_glGenRenderbuffers        = nullptr;
void* real_glDeleteRenderbuffers     = nullptr;
void* real_glBindRenderbuffer        = nullptr;
void* real_glRenderbufferStorage     = nullptr;
void* real_glFramebufferRenderbuffer = nullptr;
void* real_glCheckFramebufferStatus  = nullptr;
void* real_glGetAttribLocation       = nullptr;
void* real_glBindAttribLocation      = nullptr;
void* real_glGetProgramInfoLog       = nullptr;
void* real_glGetShaderInfoLog        = nullptr;
void* real_glUniform1i               = nullptr;
void* real_glUniform2i               = nullptr;
void* real_glUniform3i               = nullptr;
void* real_glUniform4i               = nullptr;
void* real_glUniform1f               = nullptr;
void* real_glUniform1fv              = nullptr;
void* real_glUniform2f               = nullptr;
void* real_glUniform2fv              = nullptr;
void* real_glUniform3f               = nullptr;
void* real_glUniform3fv              = nullptr;
void* real_glUniform4f               = nullptr;
void* real_glUniform4fv              = nullptr;
void* real_glUniform1iv              = nullptr;
void* real_glUniform2iv              = nullptr;
void* real_glUniform3iv              = nullptr;
void* real_glUniform4iv              = nullptr;
void* real_glUniformMatrix2fv        = nullptr;
void* real_glUniformMatrix3fv        = nullptr;
void* real_glDrawRangeElements       = nullptr;
void* real_glStencilFuncSeparate     = nullptr;
void* real_glStencilOpSeparate       = nullptr;
void* real_glStencilMaskSeparate     = nullptr;
void* real_glDepthRangef             = nullptr;
void* real_glValidateProgram         = nullptr;
void* real_glIsEnabled               = nullptr;

} // extern "C"

// ── Init: loads real DLL, fills every pointer above ──────────────────────
bool proxy_init()
{
    if (hReal) return true;

    wchar_t sys_path[MAX_PATH];
    GetSystemDirectoryW(sys_path, MAX_PATH);
    wcscat_s(sys_path, L"\\opengl32.dll");

    hReal = LoadLibraryW(sys_path);
    if (!hReal) return false;

    // --- WGL ---
    RESOLVE(wglSwapBuffers);
    RESOLVE(wglCreateContext);
    RESOLVE(wglMakeCurrent);
    RESOLVE(wglDeleteContext);
    RESOLVE(wglGetCurrentContext);
    RESOLVE(wglGetCurrentDC);
    RESOLVE(wglGetProcAddress);
    RESOLVE(wglChoosePixelFormat);
    RESOLVE(wglDescribePixelFormat);
    RESOLVE(wglSetPixelFormat);
    RESOLVE(wglGetPixelFormat);
    RESOLVE(wglShareLists);

    // --- Core GL ---
    RESOLVE(glBegin);
    RESOLVE(glEnd);
    RESOLVE(glVertex2f);
    RESOLVE(glTexCoord2f);
    RESOLVE(glColor4f);
    RESOLVE(glColor4ub);
    RESOLVE(glPushMatrix);
    RESOLVE(glPopMatrix);
    RESOLVE(glPushAttrib);
    RESOLVE(glPopAttrib);
    RESOLVE(glPushClientAttrib);
    RESOLVE(glPopClientAttrib);
    RESOLVE(glEnable);
    RESOLVE(glDisable);
    RESOLVE(glEnableClientState);
    RESOLVE(glDisableClientState);
    RESOLVE(glBlendFunc);
    RESOLVE(glBlendEquation);
    RESOLVE(glScissor);
    RESOLVE(glViewport);
    RESOLVE(glGetIntegerv);
    RESOLVE(glGetFloatv);
    RESOLVE(glPolygonMode);
    RESOLVE(glDepthFunc);
    RESOLVE(glDepthMask);
    RESOLVE(glDepthRange);
    RESOLVE(glColorMask);
    RESOLVE(glStencilMask);
    RESOLVE(glClear);
    RESOLVE(glClearColor);
    RESOLVE(glClearDepth);
    RESOLVE(glBindTexture);
    RESOLVE(glGenTextures);
    RESOLVE(glDeleteTextures);
    RESOLVE(glTexImage2D);
    RESOLVE(glTexSubImage2D);
    RESOLVE(glTexParameteri);
    RESOLVE(glTexParameterf);
    RESOLVE(glPixelStorei);
    RESOLVE(glMatrixMode);
    RESOLVE(glLoadIdentity);
    RESOLVE(glOrtho);
    RESOLVE(glReadPixels);
    RESOLVE(glDrawArrays);
    RESOLVE(glDrawElements);
    RESOLVE(glLineWidth);
    RESOLVE(glPointSize);
    RESOLVE(glShadeModel);
    RESOLVE(glAlphaFunc);
    RESOLVE(glGetString);
    RESOLVE(glGetError);
    RESOLVE(glFinish);
    RESOLVE(glFlush);

    // --- Extensions (GLES2 pipeline, resolved via wglGetProcAddress) ---
    // A temporary context may not be active yet here, so we try both paths.
    // The hk_wglMakeCurrent hook will re-resolve any that are still null.
#define TRY_EXT(name) \
    real_##name = (void*)real_wglGetProcAddress(#name); \
    if (!real_##name) real_##name = (void*)GetProcAddress(hReal, #name)

    TRY_EXT(glUseProgram);
    TRY_EXT(glGetUniformLocation);
    TRY_EXT(glUniformMatrix4fv);
    TRY_EXT(glGetActiveUniform);
    TRY_EXT(glCreateProgram);
    TRY_EXT(glDeleteProgram);
    TRY_EXT(glCreateShader);
    TRY_EXT(glDeleteShader);
    TRY_EXT(glShaderSource);
    TRY_EXT(glCompileShader);
    TRY_EXT(glAttachShader);
    TRY_EXT(glLinkProgram);
    TRY_EXT(glGetProgramiv);
    TRY_EXT(glGetShaderiv);
    TRY_EXT(glEnableVertexAttribArray);
    TRY_EXT(glDisableVertexAttribArray);
    TRY_EXT(glVertexAttribPointer);
    TRY_EXT(glGenBuffers);
    TRY_EXT(glDeleteBuffers);
    TRY_EXT(glBindBuffer);
    TRY_EXT(glBufferData);
    TRY_EXT(glActiveTexture);
    TRY_EXT(glBlendFuncSeparate);
    TRY_EXT(glBlendEquationSeparate);
    TRY_EXT(glGenerateMipmap);
    TRY_EXT(glGenFramebuffers);
    TRY_EXT(glDeleteFramebuffers);
    TRY_EXT(glBindFramebuffer);
    TRY_EXT(glFramebufferTexture2D);
    TRY_EXT(glGenRenderbuffers);
    TRY_EXT(glDeleteRenderbuffers);
    TRY_EXT(glBindRenderbuffer);
    TRY_EXT(glRenderbufferStorage);
    TRY_EXT(glFramebufferRenderbuffer);
    TRY_EXT(glCheckFramebufferStatus);
    TRY_EXT(glGetAttribLocation);
    TRY_EXT(glBindAttribLocation);
    TRY_EXT(glGetProgramInfoLog);
    TRY_EXT(glGetShaderInfoLog);
    TRY_EXT(glUniform1i);
    TRY_EXT(glUniform1f);
    TRY_EXT(glUniform1fv);
    TRY_EXT(glUniform2f);
    TRY_EXT(glUniform2fv);
    TRY_EXT(glUniform3f);
    TRY_EXT(glUniform3fv);
    TRY_EXT(glUniform4f);
    TRY_EXT(glUniform4fv);
    TRY_EXT(glUniform1iv);
    TRY_EXT(glUniform2iv);
    TRY_EXT(glUniform3iv);
    TRY_EXT(glUniform4iv);
    TRY_EXT(glUniformMatrix2fv);
    TRY_EXT(glUniformMatrix3fv);
    TRY_EXT(glDrawRangeElements);
    TRY_EXT(glStencilFuncSeparate);
    TRY_EXT(glStencilOpSeparate);
    TRY_EXT(glStencilMaskSeparate);
    TRY_EXT(glDepthRangef);
    TRY_EXT(glValidateProgram);
#undef TRY_EXT

    return true;
}

// ════════════════════════════════════════════════════════════════════════════
//   FORWARDING STUBS — every exported symbol that IS NOT hooked in hooks.cpp
//   just calls the real function. The hooked symbols (glDrawElements,
//   glGetUniformLocation, glUniformMatrix4fv, wglSwapBuffers, wglGetProcAddress)
//   are defined in hooks.cpp and omitted here.
// ════════════════════════════════════════════════════════════════════════════

// ── WGL ─────────────────────────────────────────────────────────────────────
extern "C" {

HGLRC WINAPI wglCreateContext(HDC hdc)
    { return real_wglCreateContext(hdc); }

BOOL WINAPI wglMakeCurrent(HDC hdc, HGLRC hrc) {
    BOOL r = real_wglMakeCurrent(hdc, hrc);
    // Re-resolve extension pointers now that a context is active.
    if (r && hrc && real_wglGetProcAddress) {
#define RE(name) if (!real_##name) real_##name = (void*)real_wglGetProcAddress(#name)
        RE(glUseProgram); RE(glGetUniformLocation); RE(glUniformMatrix4fv);
        RE(glCreateProgram); RE(glDeleteProgram); RE(glCreateShader);
        RE(glDeleteShader); RE(glShaderSource); RE(glCompileShader);
        RE(glAttachShader); RE(glLinkProgram); RE(glGetProgramiv);
        RE(glGetShaderiv); RE(glEnableVertexAttribArray);
        RE(glDisableVertexAttribArray); RE(glVertexAttribPointer);
        RE(glGenBuffers); RE(glDeleteBuffers); RE(glBindBuffer); RE(glBufferData);
        RE(glActiveTexture); RE(glBlendFuncSeparate); RE(glBlendEquationSeparate);
        RE(glGenerateMipmap); RE(glGenFramebuffers); RE(glDeleteFramebuffers);
        RE(glBindFramebuffer); RE(glFramebufferTexture2D); RE(glGenRenderbuffers);
        RE(glDeleteRenderbuffers); RE(glBindRenderbuffer); RE(glRenderbufferStorage);
        RE(glFramebufferRenderbuffer); RE(glCheckFramebufferStatus);
        RE(glGetAttribLocation); RE(glBindAttribLocation);
        RE(glGetProgramInfoLog); RE(glGetShaderInfoLog);
        RE(glUniform1i); RE(glUniform1f); RE(glUniform1fv);
        RE(glUniform2f); RE(glUniform2fv); RE(glUniform3f); RE(glUniform3fv);
        RE(glUniform4f); RE(glUniform4fv); RE(glUniform1iv); RE(glUniform2iv);
        RE(glUniform3iv); RE(glUniform4iv); RE(glUniformMatrix2fv);
        RE(glUniformMatrix3fv); RE(glDrawRangeElements); RE(glStencilFuncSeparate);
        RE(glStencilOpSeparate); RE(glStencilMaskSeparate); RE(glDepthRangef);
        RE(glValidateProgram); RE(glGetActiveUniform);
#undef RE
    }
    return r;
}

BOOL  WINAPI wglDeleteContext(HGLRC h)         { return real_wglDeleteContext(h); }
HGLRC WINAPI wglGetCurrentContext()            { return real_wglGetCurrentContext(); }
HDC   WINAPI wglGetCurrentDC()                 { return real_wglGetCurrentDC(); }
int   WINAPI wglChoosePixelFormat(HDC h,const PIXELFORMATDESCRIPTOR*p) { return real_wglChoosePixelFormat(h,p); }
int   WINAPI wglDescribePixelFormat(HDC h,int i,UINT s,LPPIXELFORMATDESCRIPTOR p) { return real_wglDescribePixelFormat(h,i,s,p); }
BOOL  WINAPI wglSetPixelFormat(HDC h,int i,const PIXELFORMATDESCRIPTOR*p) { return real_wglSetPixelFormat(h,i,p); }
int   WINAPI wglGetPixelFormat(HDC h)          { return real_wglGetPixelFormat(h); }
BOOL  WINAPI wglShareLists(HGLRC a,HGLRC b)   { return real_wglShareLists(a,b); }

// ── Rarely-used WGL (exact signatures from wingdi.h) ─────────────────────
BOOL  WINAPI wglCopyContext(HGLRC s, HGLRC d, UINT m)
{
    (void)s; (void)d; (void)m;
    return FALSE;
}

HGLRC WINAPI wglCreateLayerContext(HDC h, int l)
{
    (void)h; (void)l;
    return nullptr;
}

BOOL  WINAPI wglDescribeLayerPlane(HDC hdc, int iLayerPlane, int iPixelFormat,
                                   UINT nBytes, LPLAYERPLANEDESCRIPTOR plpd)
{
    (void)hdc; (void)iLayerPlane; (void)iPixelFormat; (void)nBytes; (void)plpd;
    return FALSE;
}

int   WINAPI wglGetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart,
                                       int cEntries, COLORREF* pcr)
{
    (void)hdc; (void)iLayerPlane; (void)iStart; (void)cEntries; (void)pcr;
    return 0;
}

BOOL  WINAPI wglRealizeLayerPalette(HDC hdc, int iLayerPlane, BOOL bRealize)
{
    (void)hdc; (void)iLayerPlane; (void)bRealize;
    return FALSE;
}

int   WINAPI wglSetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart,
                                       int cEntries, CONST COLORREF* pcr)
{
    (void)hdc; (void)iLayerPlane; (void)iStart; (void)cEntries; (void)pcr;
    return 0;
}

BOOL  WINAPI wglSwapLayerBuffers(HDC hdc, UINT fuPlanes)
{
    (void)fuPlanes;
    return SwapBuffers(hdc);
}

BOOL  WINAPI wglUseFontBitmapsA(HDC hdc, DWORD first, DWORD count, DWORD listBase)
{
    (void)hdc; (void)first; (void)count; (void)listBase;
    return FALSE;
}

BOOL  WINAPI wglUseFontBitmapsW(HDC hdc, DWORD first, DWORD count, DWORD listBase)
{
    (void)hdc; (void)first; (void)count; (void)listBase;
    return FALSE;
}

BOOL  WINAPI wglUseFontOutlinesA(HDC hdc, DWORD first, DWORD count, DWORD listBase,
                                 FLOAT deviation, FLOAT extrusion, int format,
                                 LPGLYPHMETRICSFLOAT lpgmf)
{
    (void)hdc; (void)first; (void)count; (void)listBase;
    (void)deviation; (void)extrusion; (void)format; (void)lpgmf;
    return FALSE;
}

BOOL  WINAPI wglUseFontOutlinesW(HDC hdc, DWORD first, DWORD count, DWORD listBase,
                                 FLOAT deviation, FLOAT extrusion, int format,
                                 LPGLYPHMETRICSFLOAT lpgmf)
{
    (void)hdc; (void)first; (void)count; (void)listBase;
    (void)deviation; (void)extrusion; (void)format; (void)lpgmf;
    return FALSE;
}

// ── Core GL forwards ─────────────────────────────────────────────────────
#define FWD0(r,n)              r APIENTRY n() { if (real_##n) real_##n(); }
#define FWD1(r,n,T1,a1)        r APIENTRY n(T1 a1) { if (real_##n) real_##n(a1); }
#define FWD2(r,n,T1,a1,T2,a2)  r APIENTRY n(T1 a1, T2 a2) { if (real_##n) real_##n(a1,a2); }

FWD0(void,   glEnd)
FWD0(void,   glLoadIdentity)
FWD0(void,   glFinish)
FWD0(void,   glFlush)
FWD0(GLenum, glGetError)

FWD1(void, glBegin,              GLenum, m)
FWD1(void, glEnable,             GLenum, c)
FWD1(void, glDisable,            GLenum, c)
FWD1(void, glClear,              GLbitfield, b)
FWD1(void, glBlendEquation,      GLenum, m)
FWD1(void, glDepthMask,          GLboolean, f)
FWD2(void, glDepthRange,         GLclampd, n, GLclampd, f)
FWD1(void, glDepthFunc,          GLenum, fn)
FWD1(void, glStencilMask,        GLuint, m)
FWD1(void, glLineWidth,          GLfloat, w)
FWD1(void, glPointSize,          GLfloat, s)
FWD1(void, glShadeModel,         GLenum, m)
FWD0(void, glPushMatrix)
FWD0(void, glPopMatrix)
FWD1(void, glMatrixMode,         GLenum, m)
FWD1(void, glEnableClientState,  GLenum, a)
FWD1(void, glDisableClientState, GLenum, a)

void APIENTRY glVertex2f(GLfloat x, GLfloat y)               { real_glVertex2f(x,y); }
void APIENTRY glTexCoord2f(GLfloat s, GLfloat t)             { real_glTexCoord2f(s,t); }
void APIENTRY glColor4f(GLfloat r,GLfloat g,GLfloat b,GLfloat a) { real_glColor4f(r,g,b,a); }
void APIENTRY glColor4ub(GLubyte r,GLubyte g,GLubyte b,GLubyte a){ real_glColor4ub(r,g,b,a); }
void APIENTRY glPushAttrib(GLbitfield m)                      { real_glPushAttrib(m); }
void APIENTRY glPopAttrib()                                   { real_glPopAttrib(); }
void APIENTRY glPushClientAttrib(GLbitfield m)                { real_glPushClientAttrib(m); }
void APIENTRY glPopClientAttrib()                             { real_glPopClientAttrib(); }
void APIENTRY glBlendFunc(GLenum s, GLenum d)                 { real_glBlendFunc(s,d); }
void APIENTRY glScissor(GLint x,GLint y,GLsizei w,GLsizei h)  { real_glScissor(x,y,w,h); }
void APIENTRY glViewport(GLint x,GLint y,GLsizei w,GLsizei h) { real_glViewport(x,y,w,h); }
void APIENTRY glGetIntegerv(GLenum p,GLint* v)                { real_glGetIntegerv(p,v); }
void APIENTRY glGetFloatv(GLenum p,GLfloat* v)                { real_glGetFloatv(p,v); }
void APIENTRY glPolygonMode(GLenum f,GLenum m)                { real_glPolygonMode(f,m); }
void APIENTRY glColorMask(GLboolean r,GLboolean g,GLboolean b,GLboolean a) { real_glColorMask(r,g,b,a); }
void APIENTRY glClearColor(GLfloat r,GLfloat g,GLfloat b,GLfloat a) { real_glClearColor(r,g,b,a); }
void APIENTRY glClearDepth(GLclampd d)                        { real_glClearDepth(d); }
void APIENTRY glBindTexture(GLenum t,GLuint id)               { real_glBindTexture(t,id); }
void APIENTRY glGenTextures(GLsizei n,GLuint* t)              { real_glGenTextures(n,t); }
void APIENTRY glDeleteTextures(GLsizei n,const GLuint* t)     { real_glDeleteTextures(n,t); }
void APIENTRY glTexParameteri(GLenum t,GLenum p,GLint v)      { real_glTexParameteri(t,p,v); }
void APIENTRY glTexParameterf(GLenum t,GLenum p,GLfloat v)    { real_glTexParameterf(t,p,v); }
void APIENTRY glPixelStorei(GLenum p,GLint v)                 { real_glPixelStorei(p,v); }
void APIENTRY glOrtho(GLdouble l,GLdouble r,GLdouble b,GLdouble t,GLdouble n,GLdouble f)
    { real_glOrtho(l,r,b,t,n,f); }
void APIENTRY glTexImage2D(GLenum target,GLint level,GLint internalfmt,
    GLsizei w,GLsizei h,GLint border,GLenum fmt,GLenum type,const void* data)
    { real_glTexImage2D(target,level,internalfmt,w,h,border,fmt,type,data); }
void APIENTRY glTexSubImage2D(GLenum target,GLint level,GLint xo,GLint yo,
    GLsizei w,GLsizei h,GLenum fmt,GLenum type,const void* data)
    { real_glTexSubImage2D(target,level,xo,yo,w,h,fmt,type,data); }
void APIENTRY glReadPixels(GLint x,GLint y,GLsizei w,GLsizei h,GLenum fmt,GLenum type,void* data)
    { real_glReadPixels(x,y,w,h,fmt,type,data); }
void APIENTRY glDrawArrays(GLenum m,GLint f,GLsizei c)        { real_glDrawArrays(m,f,c); }
void APIENTRY glAlphaFunc(GLenum f,GLclampf r)                { real_glAlphaFunc(f,r); }
const GLubyte* APIENTRY glGetString(GLenum n)                 { return real_glGetString(n); }

// ── Stubs for GL1.x functions used by legacy code / ImGui ─────────────────
// (colour/vertex variants — we just need them to link; BlueStacks won't call most)
#define STUB(name, ...) void APIENTRY name(__VA_ARGS__) {}
STUB(glAccum,       GLenum o, GLfloat v)
STUB(glBitmap,      GLsizei w,GLsizei h,GLfloat xo,GLfloat yo,GLfloat xm,GLfloat ym,const GLubyte* b)
STUB(glCallList,    GLuint l)
STUB(glCallLists,   GLsizei n,GLenum t,const void* l)
STUB(glClearAccum,  GLfloat r,GLfloat g,GLfloat b,GLfloat a)
STUB(glClearIndex,  GLfloat c)
STUB(glClipPlane,   GLenum p,const GLdouble* eq)
STUB(glCopyPixels,  GLint x,GLint y,GLsizei w,GLsizei h,GLenum t)
STUB(glCopyTexImage1D, GLenum t,GLint l,GLenum i,GLint x,GLint y,GLsizei w,GLint b)
STUB(glCopyTexImage2D, GLenum t,GLint l,GLenum i,GLint x,GLint y,GLsizei w,GLsizei h,GLint b)
STUB(glCopyTexSubImage1D, GLenum t,GLint l,GLint x,GLint y,GLint xs,GLsizei w)
STUB(glCopyTexSubImage2D, GLenum t,GLint l,GLint x,GLint y,GLint xs,GLint ys,GLsizei w,GLsizei h)
STUB(glCullFace,    GLenum m)
STUB(glEdgeFlag,    GLboolean f)
STUB(glEdgeFlagPointer, GLsizei s,const void* p)
STUB(glEdgeFlagv,   const GLboolean* f)
STUB(glEvalCoord1d, GLdouble u)
STUB(glEvalCoord1dv,const GLdouble* u)
STUB(glEvalCoord1f, GLfloat u)
STUB(glEvalCoord1fv,const GLfloat* u)
STUB(glEvalCoord2d, GLdouble u,GLdouble v)
STUB(glEvalCoord2dv,const GLdouble* u)
STUB(glEvalCoord2f, GLfloat u,GLfloat v)
STUB(glEvalCoord2fv,const GLfloat* u)
STUB(glEvalMesh1,   GLenum m,GLint i1,GLint i2)
STUB(glEvalMesh2,   GLenum m,GLint i1,GLint i2,GLint j1,GLint j2)
STUB(glEvalPoint1,  GLint i)
STUB(glEvalPoint2,  GLint i,GLint j)
STUB(glFeedbackBuffer, GLsizei s,GLenum t,GLfloat* b)
STUB(glFogf,        GLenum p,GLfloat v)
STUB(glFogfv,       GLenum p,const GLfloat* v)
STUB(glFogi,        GLenum p,GLint v)
STUB(glFogiv,       GLenum p,const GLint* v)
STUB(glFrontFace,   GLenum m)
STUB(glFrustum,     GLdouble l,GLdouble r,GLdouble b,GLdouble t,GLdouble n,GLdouble f)
STUB(glIndexMask,   GLuint m)
STUB(glIndexPointer,GLenum t,GLsizei s,const void* p)
STUB(glIndexd,      GLdouble c)
STUB(glIndexdv,     const GLdouble* c)
STUB(glIndexf,      GLfloat c)
STUB(glIndexfv,     const GLfloat* c)
STUB(glIndexi,      GLint c)
STUB(glIndexiv,     const GLint* c)
STUB(glIndexs,      GLshort c)
STUB(glIndexsv,     const GLshort* c)
STUB(glIndexub,     GLubyte c)
STUB(glIndexubv,    const GLubyte* c)
STUB(glInitNames)
STUB(glInterleavedArrays, GLenum f,GLsizei s,const void* p)
STUB(glLightModelf, GLenum p,GLfloat v)
STUB(glLightModelfv,GLenum p,const GLfloat* v)
STUB(glLightModeli, GLenum p,GLint v)
STUB(glLightModeliv,GLenum p,const GLint* v)
STUB(glLightf,      GLenum l,GLenum p,GLfloat v)
STUB(glLightfv,     GLenum l,GLenum p,const GLfloat* v)
STUB(glLighti,      GLenum l,GLenum p,GLint v)
STUB(glLightiv,     GLenum l,GLenum p,const GLint* v)
STUB(glLineStipple, GLint f,GLushort p)
STUB(glListBase,    GLuint b)
STUB(glLoadMatrixd, const GLdouble* m)
STUB(glLoadMatrixf, const GLfloat* m)
STUB(glLoadName,    GLuint n)
STUB(glLogicOp,     GLenum o)
STUB(glMap1d,       GLenum t,GLdouble u1,GLdouble u2,GLint s,GLint o,const GLdouble* p)
STUB(glMap1f,       GLenum t,GLfloat u1,GLfloat u2,GLint s,GLint o,const GLfloat* p)
STUB(glMap2d,       GLenum t,GLdouble u1,GLdouble u2,GLint us,GLint uo,GLdouble v1,GLdouble v2,GLint vs,GLint vo,const GLdouble* p)
STUB(glMap2f,       GLenum t,GLfloat u1,GLfloat u2,GLint us,GLint uo,GLfloat v1,GLfloat v2,GLint vs,GLint vo,const GLfloat* p)
STUB(glMapGrid1d,   GLint un,GLdouble u1,GLdouble u2)
STUB(glMapGrid1f,   GLint un,GLfloat u1,GLfloat u2)
STUB(glMapGrid2d,   GLint un,GLdouble u1,GLdouble u2,GLint vn,GLdouble v1,GLdouble v2)
STUB(glMapGrid2f,   GLint un,GLfloat u1,GLfloat u2,GLint vn,GLfloat v1,GLfloat v2)
STUB(glMaterialf,   GLenum f,GLenum p,GLfloat v)
STUB(glMaterialfv,  GLenum f,GLenum p,const GLfloat* v)
STUB(glMateriali,   GLenum f,GLenum p,GLint v)
STUB(glMaterialiv,  GLenum f,GLenum p,const GLint* v)
STUB(glMultMatrixd, const GLdouble* m)
STUB(glMultMatrixf, const GLfloat* m)
STUB(glNewList,     GLuint l,GLenum m)
STUB(glNormal3b,    GLbyte x,GLbyte y,GLbyte z)
STUB(glNormal3bv,   const GLbyte* v)
STUB(glNormal3d,    GLdouble x,GLdouble y,GLdouble z)
STUB(glNormal3dv,   const GLdouble* v)
STUB(glNormal3f,    GLfloat x,GLfloat y,GLfloat z)
STUB(glNormal3fv,   const GLfloat* v)
STUB(glNormal3i,    GLint x,GLint y,GLint z)
STUB(glNormal3iv,   const GLint* v)
STUB(glNormal3s,    GLshort x,GLshort y,GLshort z)
STUB(glNormal3sv,   const GLshort* v)
STUB(glNormalPointer,GLenum t,GLsizei s,const void* p)
STUB(glPassThrough, GLfloat t)
STUB(glPixelMapfv,  GLenum m,GLsizei ms,const GLfloat* v)
STUB(glPixelMapuiv, GLenum m,GLsizei ms,const GLuint* v)
STUB(glPixelMapusv, GLenum m,GLsizei ms,const GLushort* v)
STUB(glPixelTransferf,GLenum p,GLfloat v)
STUB(glPixelTransferi,GLenum p,GLint v)
STUB(glPixelZoom,   GLfloat x,GLfloat y)
STUB(glPolygonStipple,const GLubyte* m)
STUB(glPopName)
STUB(glPrioritizeTextures,GLsizei n,const GLuint* t,const GLclampf* p)
STUB(glPushName,    GLuint n)
STUB(glRasterPos2d, GLdouble x,GLdouble y)
STUB(glRasterPos2dv,const GLdouble* v)
STUB(glRasterPos2f, GLfloat x,GLfloat y)
STUB(glRasterPos2fv,const GLfloat* v)
STUB(glRasterPos2i, GLint x,GLint y)
STUB(glRasterPos2iv,const GLint* v)
STUB(glRasterPos2s, GLshort x,GLshort y)
STUB(glRasterPos2sv,const GLshort* v)
STUB(glRasterPos3d, GLdouble x,GLdouble y,GLdouble z)
STUB(glRasterPos3dv,const GLdouble* v)
STUB(glRasterPos3f, GLfloat x,GLfloat y,GLfloat z)
STUB(glRasterPos3fv,const GLfloat* v)
STUB(glRasterPos3i, GLint x,GLint y,GLint z)
STUB(glRasterPos3iv,const GLint* v)
STUB(glRasterPos3s, GLshort x,GLshort y,GLshort z)
STUB(glRasterPos3sv,const GLshort* v)
STUB(glRasterPos4d, GLdouble x,GLdouble y,GLdouble z,GLdouble w)
STUB(glRasterPos4dv,const GLdouble* v)
STUB(glRasterPos4f, GLfloat x,GLfloat y,GLfloat z,GLfloat w)
STUB(glRasterPos4fv,const GLfloat* v)
STUB(glRasterPos4i, GLint x,GLint y,GLint z,GLint w)
STUB(glRasterPos4iv,const GLint* v)
STUB(glRasterPos4s, GLshort x,GLshort y,GLshort z,GLshort w)
STUB(glRasterPos4sv,const GLshort* v)
STUB(glReadBuffer,  GLenum m)
STUB(glRectd,       GLdouble x1,GLdouble y1,GLdouble x2,GLdouble y2)
STUB(glRectdv,      const GLdouble* v1,const GLdouble* v2)
STUB(glRectf,       GLfloat x1,GLfloat y1,GLfloat x2,GLfloat y2)
STUB(glRectfv,      const GLfloat* v1,const GLfloat* v2)
STUB(glRecti,       GLint x1,GLint y1,GLint x2,GLint y2)
STUB(glRectiv,      const GLint* v1,const GLint* v2)
STUB(glRects,       GLshort x1,GLshort y1,GLshort x2,GLshort y2)
STUB(glRectsv,      const GLshort* v1,const GLshort* v2)
STUB(glRotated,     GLdouble a,GLdouble x,GLdouble y,GLdouble z)
STUB(glRotatef,     GLfloat a,GLfloat x,GLfloat y,GLfloat z)
STUB(glScaled,      GLdouble x,GLdouble y,GLdouble z)
STUB(glScalef,      GLfloat x,GLfloat y,GLfloat z)
STUB(glSelectBuffer,GLsizei s,GLuint* b)
STUB(glStencilFunc, GLenum f,GLint r,GLuint m)
STUB(glStencilOp,   GLenum f,GLenum sf,GLenum df)
STUB(glTexCoordPointer,GLint s,GLenum t,GLsizei st,const void* p)
STUB(glTexEnvf,     GLenum t,GLenum p,GLfloat v)
STUB(glTexEnvfv,    GLenum t,GLenum p,const GLfloat* v)
STUB(glTexEnvi,     GLenum t,GLenum p,GLint v)
STUB(glTexEnviv,    GLenum t,GLenum p,const GLint* v)
STUB(glTexGend,     GLenum c,GLenum p,GLdouble v)
STUB(glTexGendv,    GLenum c,GLenum p,const GLdouble* v)
STUB(glTexGenf,     GLenum c,GLenum p,GLfloat v)
STUB(glTexGenfv,    GLenum c,GLenum p,const GLfloat* v)
STUB(glTexGeni,     GLenum c,GLenum p,GLint v)
STUB(glTexGeniv,    GLenum c,GLenum p,const GLint* v)
STUB(glTexImage1D,  GLenum t,GLint l,GLint i,GLsizei w,GLint b,GLenum f,GLenum ty,const void* d)
STUB(glTexSubImage1D,GLenum t,GLint l,GLint x,GLsizei w,GLenum f,GLenum ty,const void* d)
STUB(glTranslated,  GLdouble x,GLdouble y,GLdouble z)
STUB(glTranslatef,  GLfloat x,GLfloat y,GLfloat z)
STUB(glVertexPointer,GLint s,GLenum t,GLsizei st,const void* p)
STUB(glColorPointer, GLint s,GLenum t,GLsizei st,const void* p)
STUB(glDrawBuffer,   GLenum m)
STUB(glPolygonOffset,GLfloat f,GLfloat u)

// Colour variants (all forward; BlueStacks game won't use these but ImGui might)
#define C3(T,s) void APIENTRY glColor3##s(T r,T g,T b) { real_glColor4f((float)r,(float)g,(float)b,1.f); }
#define C3V(T,s) void APIENTRY glColor3##s##v(const T* v) { real_glColor4f((float)v[0],(float)v[1],(float)v[2],1.f); }
C3(GLbyte,b)  C3(GLdouble,d)  C3(GLfloat,f)  C3(GLint,i)
C3(GLshort,s) C3(GLubyte,ub)  C3(GLuint,ui)  C3(GLushort,us)
C3V(GLbyte,b) C3V(GLdouble,d) C3V(GLfloat,f) C3V(GLint,i)
C3V(GLshort,s) C3V(GLubyte,ub) C3V(GLuint,ui) C3V(GLushort,us)

// Vertex variants
#define V2(T,s) void APIENTRY glVertex2##s(T x,T y)         { real_glVertex2f((float)x,(float)y); }
#define V2V(T,s) void APIENTRY glVertex2##s##v(const T* v)  { real_glVertex2f((float)v[0],(float)v[1]); }
#define V3(T,s) void APIENTRY glVertex3##s(T x,T y,T z)     { real_glVertex2f((float)x,(float)y); }
#define V3V(T,s) void APIENTRY glVertex3##s##v(const T* v)  { real_glVertex2f((float)v[0],(float)v[1]); }
#define V4(T,s) void APIENTRY glVertex4##s(T x,T y,T z,T w) { real_glVertex2f((float)x,(float)y); }
#define V4V(T,s) void APIENTRY glVertex4##s##v(const T* v)  { real_glVertex2f((float)v[0],(float)v[1]); }
V2(GLdouble,d) /* V2(GLfloat,f) already defined above as glVertex2f */ V2(GLint,i) V2(GLshort,s)
V2V(GLdouble,d) V2V(GLfloat,f) V2V(GLint,i) V2V(GLshort,s)
V3(GLdouble,d) V3(GLfloat,f) V3(GLint,i) V3(GLshort,s)
V3V(GLdouble,d) V3V(GLfloat,f) V3V(GLint,i) V3V(GLshort,s)
V4(GLdouble,d) V4(GLfloat,f) V4(GLint,i) V4(GLshort,s)
V4V(GLdouble,d) V4V(GLfloat,f) V4V(GLint,i) V4V(GLshort,s)

// TexCoord variants (stub — not needed by game, only legacy calls)
STUB(glTexCoord1d, GLdouble s) STUB(glTexCoord1dv, const GLdouble* v)
STUB(glTexCoord1f, GLfloat s)  STUB(glTexCoord1fv, const GLfloat* v)
STUB(glTexCoord1i, GLint s)    STUB(glTexCoord1iv, const GLint* v)
STUB(glTexCoord1s, GLshort s)  STUB(glTexCoord1sv, const GLshort* v)
STUB(glTexCoord2d, GLdouble s,GLdouble t)
STUB(glTexCoord2dv,const GLdouble* v)
STUB(glTexCoord2i, GLint s,GLint t)    STUB(glTexCoord2iv, const GLint* v)
STUB(glTexCoord2s, GLshort s,GLshort t) STUB(glTexCoord2sv, const GLshort* v)
STUB(glTexCoord3d, GLdouble s,GLdouble t,GLdouble r)
STUB(glTexCoord3dv,const GLdouble* v)
STUB(glTexCoord3f, GLfloat s,GLfloat t,GLfloat r)
STUB(glTexCoord3fv,const GLfloat* v)
STUB(glTexCoord3i, GLint s,GLint t,GLint r)    STUB(glTexCoord3iv, const GLint* v)
STUB(glTexCoord3s, GLshort s,GLshort t,GLshort r) STUB(glTexCoord3sv,const GLshort* v)
STUB(glTexCoord4d, GLdouble s,GLdouble t,GLdouble r,GLdouble q)
STUB(glTexCoord4dv,const GLdouble* v)
STUB(glTexCoord4f, GLfloat s,GLfloat t,GLfloat r,GLfloat q)
STUB(glTexCoord4fv,const GLfloat* v)
STUB(glTexCoord4i, GLint s,GLint t,GLint r,GLint q) STUB(glTexCoord4iv, const GLint* v)
STUB(glTexCoord4s, GLshort s,GLshort t,GLshort r,GLshort q) STUB(glTexCoord4sv,const GLshort* v)

// ── GLES2 extension forwarding stubs (pointer cast + call) ─────────────────
// glGetUniformLocation, glDrawElements, glUniformMatrix4fv, wglSwapBuffers,
// and wglGetProcAddress are defined in hooks.cpp — NOT here.

#define EXT_CALL(T,name,...) \
    typedef T (APIENTRY* pfn_##name)(__VA_ARGS__); \
    T APIENTRY name(__VA_ARGS__)

EXT_CALL(void, glUseProgram, GLuint p)
    { if(real_glUseProgram) ((pfn_glUseProgram)real_glUseProgram)(p); }

EXT_CALL(GLuint, glCreateProgram)
    { return real_glCreateProgram ? ((GLuint(APIENTRY*)())real_glCreateProgram)() : 0; }

EXT_CALL(void, glDeleteProgram, GLuint p)
    { if(real_glDeleteProgram) ((void(APIENTRY*)(GLuint))real_glDeleteProgram)(p); }

EXT_CALL(GLuint, glCreateShader, GLenum t)
    { return real_glCreateShader ? ((GLuint(APIENTRY*)(GLenum))real_glCreateShader)(t) : 0; }

EXT_CALL(void, glDeleteShader, GLuint s)
    { if(real_glDeleteShader) ((void(APIENTRY*)(GLuint))real_glDeleteShader)(s); }

EXT_CALL(void, glShaderSource, GLuint s,GLsizei c,const GLchar*const* src,const GLint* len)
    { if(real_glShaderSource) ((void(APIENTRY*)(GLuint,GLsizei,const GLchar*const*,const GLint*))real_glShaderSource)(s,c,src,len); }

EXT_CALL(void, glCompileShader, GLuint s)
    { if(real_glCompileShader) ((void(APIENTRY*)(GLuint))real_glCompileShader)(s); }

EXT_CALL(void, glAttachShader, GLuint p,GLuint s)
    { if(real_glAttachShader) ((void(APIENTRY*)(GLuint,GLuint))real_glAttachShader)(p,s); }

EXT_CALL(void, glDetachShader, GLuint p,GLuint s)
    { if(real_glAttachShader) ((void(APIENTRY*)(GLuint,GLuint))real_glAttachShader)(p,s); }

EXT_CALL(void, glLinkProgram, GLuint p)
    { if(real_glLinkProgram) ((void(APIENTRY*)(GLuint))real_glLinkProgram)(p); }

EXT_CALL(void, glGetProgramiv, GLuint p,GLenum pname,GLint* params)
    { if(real_glGetProgramiv) ((void(APIENTRY*)(GLuint,GLenum,GLint*))real_glGetProgramiv)(p,pname,params); }

EXT_CALL(void, glGetShaderiv, GLuint s,GLenum pname,GLint* params)
    { if(real_glGetShaderiv) ((void(APIENTRY*)(GLuint,GLenum,GLint*))real_glGetShaderiv)(s,pname,params); }

EXT_CALL(void, glGetProgramInfoLog, GLuint p,GLsizei max,GLsizei* len,GLchar* log_)
    { if(real_glGetProgramInfoLog) ((void(APIENTRY*)(GLuint,GLsizei,GLsizei*,GLchar*))real_glGetProgramInfoLog)(p,max,len,log_); }

EXT_CALL(void, glGetShaderInfoLog, GLuint s,GLsizei max,GLsizei* len,GLchar* log_)
    { if(real_glGetShaderInfoLog) ((void(APIENTRY*)(GLuint,GLsizei,GLsizei*,GLchar*))real_glGetShaderInfoLog)(s,max,len,log_); }

EXT_CALL(GLint, glGetAttribLocation, GLuint p,const GLchar* n)
    { return real_glGetAttribLocation ? ((GLint(APIENTRY*)(GLuint,const GLchar*))real_glGetAttribLocation)(p,n) : -1; }

EXT_CALL(void, glBindAttribLocation, GLuint p,GLuint i,const GLchar* n)
    { if(real_glBindAttribLocation) ((void(APIENTRY*)(GLuint,GLuint,const GLchar*))real_glBindAttribLocation)(p,i,n); }

EXT_CALL(void, glEnableVertexAttribArray, GLuint i)
    { if(real_glEnableVertexAttribArray) ((void(APIENTRY*)(GLuint))real_glEnableVertexAttribArray)(i); }

EXT_CALL(void, glDisableVertexAttribArray, GLuint i)
    { if(real_glDisableVertexAttribArray) ((void(APIENTRY*)(GLuint))real_glDisableVertexAttribArray)(i); }

EXT_CALL(void, glVertexAttribPointer, GLuint i,GLint sz,GLenum t,GLboolean n,GLsizei s,const void* p)
    { if(real_glVertexAttribPointer) ((void(APIENTRY*)(GLuint,GLint,GLenum,GLboolean,GLsizei,const void*))real_glVertexAttribPointer)(i,sz,t,n,s,p); }

EXT_CALL(void, glVertexAttrib1f, GLuint i,GLfloat x)
    { if(real_glVertexAttribPointer) ((void(APIENTRY*)(GLuint,GLfloat))real_glEnableVertexAttribArray)(i); } // harmless

EXT_CALL(void, glVertexAttrib2f, GLuint i,GLfloat x,GLfloat y)  {}
EXT_CALL(void, glVertexAttrib3f, GLuint i,GLfloat x,GLfloat y,GLfloat z) {}
EXT_CALL(void, glVertexAttrib4f, GLuint i,GLfloat x,GLfloat y,GLfloat z,GLfloat w) {}

#define GEN(name) EXT_CALL(void,name,GLsizei n,GLuint* b) \
    { if(real_##name) ((void(APIENTRY*)(GLsizei,GLuint*))real_##name)(n,b); }
GEN(glGenBuffers) GEN(glGenFramebuffers) GEN(glGenRenderbuffers)

#define DEL(name) EXT_CALL(void,name,GLsizei n,const GLuint* b) \
    { if(real_##name) ((void(APIENTRY*)(GLsizei,const GLuint*))real_##name)(n,b); }
DEL(glDeleteBuffers) DEL(glDeleteFramebuffers) DEL(glDeleteRenderbuffers)

EXT_CALL(void, glBindBuffer, GLenum t,GLuint b)
    { if(real_glBindBuffer) ((void(APIENTRY*)(GLenum,GLuint))real_glBindBuffer)(t,b); }

EXT_CALL(void, glBufferData, GLenum t,GLsizeiptr s,const void* d,GLenum u)
    { if(real_glBufferData) ((void(APIENTRY*)(GLenum,GLsizeiptr,const void*,GLenum))real_glBufferData)(t,s,d,u); }

EXT_CALL(void, glBufferSubData, GLenum t,GLintptr o,GLsizeiptr s,const void* d)
    { if(real_glBufferData) ((void(APIENTRY*)(GLenum,GLintptr,GLsizeiptr,const void*))real_glBufferData)(t,o,s,d); }

EXT_CALL(void, glActiveTexture, GLenum t)
    { if(real_glActiveTexture) ((void(APIENTRY*)(GLenum))real_glActiveTexture)(t); }

EXT_CALL(void, glGenerateMipmap, GLenum t)
    { if(real_glGenerateMipmap) ((void(APIENTRY*)(GLenum))real_glGenerateMipmap)(t); }

EXT_CALL(void, glBlendFuncSeparate, GLenum sRGB,GLenum dRGB,GLenum sA,GLenum dA)
    { if(real_glBlendFuncSeparate) ((void(APIENTRY*)(GLenum,GLenum,GLenum,GLenum))real_glBlendFuncSeparate)(sRGB,dRGB,sA,dA); }

EXT_CALL(void, glBlendEquationSeparate, GLenum mRGB,GLenum mA)
    { if(real_glBlendEquationSeparate) ((void(APIENTRY*)(GLenum,GLenum))real_glBlendEquationSeparate)(mRGB,mA); }

EXT_CALL(void, glBlendColor, GLfloat r,GLfloat g,GLfloat b,GLfloat a)
    { /* rarely needed */ }

EXT_CALL(void, glBindFramebuffer, GLenum t,GLuint fb)
    { if(real_glBindFramebuffer) ((void(APIENTRY*)(GLenum,GLuint))real_glBindFramebuffer)(t,fb); }

EXT_CALL(void, glFramebufferTexture2D, GLenum t,GLenum att,GLenum tt,GLuint tex,GLint lev)
    { if(real_glFramebufferTexture2D) ((void(APIENTRY*)(GLenum,GLenum,GLenum,GLuint,GLint))real_glFramebufferTexture2D)(t,att,tt,tex,lev); }

EXT_CALL(void, glBindRenderbuffer, GLenum t,GLuint rb)
    { if(real_glBindRenderbuffer) ((void(APIENTRY*)(GLenum,GLuint))real_glBindRenderbuffer)(t,rb); }

EXT_CALL(void, glRenderbufferStorage, GLenum t,GLenum i,GLsizei w,GLsizei h)
    { if(real_glRenderbufferStorage) ((void(APIENTRY*)(GLenum,GLenum,GLsizei,GLsizei))real_glRenderbufferStorage)(t,i,w,h); }

EXT_CALL(void, glFramebufferRenderbuffer, GLenum t,GLenum att,GLenum rbt,GLuint rb)
    { if(real_glFramebufferRenderbuffer) ((void(APIENTRY*)(GLenum,GLenum,GLenum,GLuint))real_glFramebufferRenderbuffer)(t,att,rbt,rb); }

EXT_CALL(GLenum, glCheckFramebufferStatus, GLenum t)
    { return real_glCheckFramebufferStatus ? ((GLenum(APIENTRY*)(GLenum))real_glCheckFramebufferStatus)(t) : 0; }

// Uniform stubs
#define U1(s,T) EXT_CALL(void,glUniform1##s,GLint l,T v) \
    { if(real_glUniform1##s) ((void(APIENTRY*)(GLint,T))real_glUniform1##s)(l,v); }
#define U2(s,T) EXT_CALL(void,glUniform2##s,GLint l,T x,T y) \
    { if(real_glUniform2##s) ((void(APIENTRY*)(GLint,T,T))real_glUniform2##s)(l,x,y); }
#define U3(s,T) EXT_CALL(void,glUniform3##s,GLint l,T x,T y,T z) \
    { if(real_glUniform3##s) ((void(APIENTRY*)(GLint,T,T,T))real_glUniform3##s)(l,x,y,z); }
#define U4(s,T) EXT_CALL(void,glUniform4##s,GLint l,T x,T y,T z,T w) \
    { if(real_glUniform4##s) ((void(APIENTRY*)(GLint,T,T,T,T))real_glUniform4##s)(l,x,y,z,w); }
U1(f,GLfloat) U2(f,GLfloat) U3(f,GLfloat) U4(f,GLfloat)
U1(i,GLint)   U2(i,GLint)   U3(i,GLint)   U4(i,GLint)

#define UV(s,T) EXT_CALL(void,glUniform##s##v,GLint l,GLsizei c,const T* v) \
    { if(real_glUniform##s##v) ((void(APIENTRY*)(GLint,GLsizei,const T*))real_glUniform##s##v)(l,c,v); }
UV(1f,GLfloat) UV(2f,GLfloat) UV(3f,GLfloat) UV(4f,GLfloat)
UV(1i,GLint)   UV(2i,GLint)   UV(3i,GLint)   UV(4i,GLint)

EXT_CALL(void, glUniformMatrix2fv, GLint l,GLsizei c,GLboolean t,const GLfloat* v)
    { if(real_glUniformMatrix2fv) ((void(APIENTRY*)(GLint,GLsizei,GLboolean,const GLfloat*))real_glUniformMatrix2fv)(l,c,t,v); }
EXT_CALL(void, glUniformMatrix3fv, GLint l,GLsizei c,GLboolean t,const GLfloat* v)
    { if(real_glUniformMatrix3fv) ((void(APIENTRY*)(GLint,GLsizei,GLboolean,const GLfloat*))real_glUniformMatrix3fv)(l,c,t,v); }

EXT_CALL(void, glDrawRangeElements, GLenum m,GLuint s,GLuint e,GLsizei c,GLenum t,const void* i)
    { if(real_glDrawRangeElements) ((void(APIENTRY*)(GLenum,GLuint,GLuint,GLsizei,GLenum,const void*))real_glDrawRangeElements)(m,s,e,c,t,i); }

EXT_CALL(void, glStencilFuncSeparate, GLenum f,GLenum func,GLint r,GLuint m)
    { if(real_glStencilFuncSeparate) ((void(APIENTRY*)(GLenum,GLenum,GLint,GLuint))real_glStencilFuncSeparate)(f,func,r,m); }

EXT_CALL(void, glStencilOpSeparate, GLenum f,GLenum sf,GLenum df,GLenum dp)
    { if(real_glStencilOpSeparate) ((void(APIENTRY*)(GLenum,GLenum,GLenum,GLenum))real_glStencilOpSeparate)(f,sf,df,dp); }

EXT_CALL(void, glStencilMaskSeparate, GLenum f,GLuint m)
    { if(real_glStencilMaskSeparate) ((void(APIENTRY*)(GLenum,GLuint))real_glStencilMaskSeparate)(f,m); }

EXT_CALL(void, glDepthRangef, GLfloat n,GLfloat f)
    { if(real_glDepthRangef) ((void(APIENTRY*)(GLfloat,GLfloat))real_glDepthRangef)(n,f); }

EXT_CALL(void, glValidateProgram, GLuint p)
    { if(real_glValidateProgram) ((void(APIENTRY*)(GLuint))real_glValidateProgram)(p); }

EXT_CALL(void, glReleaseShaderCompiler) {}

EXT_CALL(void, glGetActiveAttrib, GLuint p,GLuint i,GLsizei b,GLsizei* l,GLint* sz,GLenum* t,GLchar* n)
    { if(real_glGetActiveUniform) ((void(APIENTRY*)(GLuint,GLuint,GLsizei,GLsizei*,GLint*,GLenum*,GLchar*))real_glGetActiveUniform)(p,i,b,l,sz,t,n); }

EXT_CALL(void, glGetActiveUniform, GLuint p,GLuint i,GLsizei b,GLsizei* l,GLint* sz,GLenum* t,GLchar* n)
    { if(real_glGetActiveUniform) ((void(APIENTRY*)(GLuint,GLuint,GLsizei,GLsizei*,GLint*,GLenum*,GLchar*))real_glGetActiveUniform)(p,i,b,l,sz,t,n); }

EXT_CALL(void, glGetAttachedShaders, GLuint p,GLsizei max,GLsizei* c,GLuint* s)
    { /* rarely called */ }

EXT_CALL(void, glGetShaderPrecisionFormat, GLenum st,GLenum pt,GLint* r,GLint* p)
    { if(r){r[0]=0;r[1]=0;} if(p)*p=0; }

EXT_CALL(void, glGetShaderSource, GLuint s,GLsizei b,GLsizei* l,GLchar* src)
    { if(l)*l=0; }

EXT_CALL(void, glGetUniformfv, GLuint p,GLint l,GLfloat* params) {}
EXT_CALL(void, glGetUniformiv, GLuint p,GLint l,GLint* params) {}
EXT_CALL(void, glGetVertexAttribfv, GLuint i,GLenum p,GLfloat* params) {}
EXT_CALL(void, glGetVertexAttribiv, GLuint i,GLenum p,GLint* params) {}
EXT_CALL(void, glGetVertexAttribPointerv, GLuint i,GLenum p,void** ptr) {}
EXT_CALL(void, glGetFramebufferAttachmentParameteriv, GLenum t,GLenum a,GLenum p,GLint* params) {}
EXT_CALL(void, glGetRenderbufferParameteriv, GLenum t,GLenum p,GLint* params) {}
EXT_CALL(void, glSampleCoverage, GLfloat v,GLboolean i) {}

EXT_CALL(GLboolean, glIsBuffer, GLuint b)        { return GL_FALSE; }
EXT_CALL(GLboolean, glIsFramebuffer, GLuint f)   { return GL_FALSE; }
EXT_CALL(GLboolean, glIsProgram, GLuint p)        { return GL_FALSE; }
EXT_CALL(GLboolean, glIsRenderbuffer, GLuint r)   { return GL_FALSE; }
EXT_CALL(GLboolean, glIsShader, GLuint s)         { return GL_FALSE; }
EXT_CALL(GLboolean, glIsEnabled, GLenum c)
    { return real_glIsEnabled ? ((GLboolean(APIENTRY*)(GLenum))GetProcAddress(hReal,"glIsEnabled"))(c) : GL_FALSE; }
EXT_CALL(GLboolean, glIsTexture, GLuint t)        { return GL_FALSE; }
EXT_CALL(GLboolean, glIsList, GLuint l)           { return GL_FALSE; }

GLuint APIENTRY glGenLists(GLsizei r) { return 0; }
void   APIENTRY glDeleteLists(GLuint l,GLsizei r) {}
void   APIENTRY glEndList() {}
GLint  APIENTRY glRenderMode(GLenum m) { return 0; }
void   APIENTRY glGetBooleanv(GLenum p,GLboolean* v)  { if(v)*v=GL_FALSE; }
void   APIENTRY glGetClipPlane(GLenum p,GLdouble* eq) {}
void   APIENTRY glGetDoublev(GLenum p,GLdouble* v)    { if(v)*v=0.0; }
void   APIENTRY glGetLightfv(GLenum l,GLenum p,GLfloat* v) {}
void   APIENTRY glGetLightiv(GLenum l,GLenum p,GLint* v)   {}
void   APIENTRY glGetMapdv(GLenum t,GLenum q,GLdouble* v)  {}
void   APIENTRY glGetMapfv(GLenum t,GLenum q,GLfloat* v)   {}
void   APIENTRY glGetMapiv(GLenum t,GLenum q,GLint* v)     {}
void   APIENTRY glGetMaterialfv(GLenum f,GLenum p,GLfloat* v) {}
void   APIENTRY glGetMaterialiv(GLenum f,GLenum p,GLint* v)   {}
void   APIENTRY glGetPixelMapfv(GLenum m,GLfloat* v)       {}
void   APIENTRY glGetPixelMapuiv(GLenum m,GLuint* v)       {}
void   APIENTRY glGetPixelMapusv(GLenum m,GLushort* v)     {}
void   APIENTRY glGetPointerv(GLenum p,void** v)           { if(v)*v=nullptr; }
void   APIENTRY glGetPolygonStipple(GLubyte* m)            {}
void   APIENTRY glGetTexEnvfv(GLenum t,GLenum p,GLfloat* v) {}
void   APIENTRY glGetTexEnviv(GLenum t,GLenum p,GLint* v)   {}
void   APIENTRY glGetTexGendv(GLenum c,GLenum p,GLdouble* v) {}
void   APIENTRY glGetTexGenfv(GLenum c,GLenum p,GLfloat* v)  {}
void   APIENTRY glGetTexGeniv(GLenum c,GLenum p,GLint* v)    {}
void   APIENTRY glGetTexImage(GLenum t,GLint l,GLenum f,GLenum ty,void* d) {}
void   APIENTRY glGetTexLevelParameterfv(GLenum t,GLint l,GLenum p,GLfloat* v) {}
void   APIENTRY glGetTexLevelParameteriv(GLenum t,GLint l,GLenum p,GLint* v)   {}
void   APIENTRY glGetTexParameterfv(GLenum t,GLenum p,GLfloat* v) {}
void   APIENTRY glGetTexParameteriv(GLenum t,GLenum p,GLint* v)   {}

} // extern "C"

#pragma warning(pop)
