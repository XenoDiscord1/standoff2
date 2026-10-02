#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>

// ── Initialise proxy: loads real opengl32, resolves all pointers ──────────
bool proxy_init();

// ── Real function pointers — definitions live in gl_proxy.cpp ────────────
// We expose them here so hooks.cpp can forward calls to the real functions.

extern "C" {

// WGL
extern BOOL  (WINAPI* real_wglSwapBuffers)(HDC);
extern HGLRC (WINAPI* real_wglCreateContext)(HDC);
extern BOOL  (WINAPI* real_wglMakeCurrent)(HDC, HGLRC);
extern BOOL  (WINAPI* real_wglDeleteContext)(HGLRC);
extern HGLRC (WINAPI* real_wglGetCurrentContext)();
extern HDC   (WINAPI* real_wglGetCurrentDC)();
extern PROC  (WINAPI* real_wglGetProcAddress)(LPCSTR);
extern int   (WINAPI* real_wglChoosePixelFormat)(HDC, const PIXELFORMATDESCRIPTOR*);
extern int   (WINAPI* real_wglDescribePixelFormat)(HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);
extern BOOL  (WINAPI* real_wglSetPixelFormat)(HDC, int, const PIXELFORMATDESCRIPTOR*);
extern int   (WINAPI* real_wglGetPixelFormat)(HDC);
extern BOOL  (WINAPI* real_wglShareLists)(HGLRC, HGLRC);

// Core GL — used by ImGui OpenGL2 backend and our hook code
extern void   (APIENTRY* real_glBegin)(GLenum);
extern void   (APIENTRY* real_glEnd)();
extern void   (APIENTRY* real_glVertex2f)(GLfloat, GLfloat);
extern void   (APIENTRY* real_glTexCoord2f)(GLfloat, GLfloat);
extern void   (APIENTRY* real_glColor4f)(GLfloat, GLfloat, GLfloat, GLfloat);
extern void   (APIENTRY* real_glColor4ub)(GLubyte, GLubyte, GLubyte, GLubyte);
extern void   (APIENTRY* real_glPushMatrix)();
extern void   (APIENTRY* real_glPopMatrix)();
extern void   (APIENTRY* real_glPushAttrib)(GLbitfield);
extern void   (APIENTRY* real_glPopAttrib)();
extern void   (APIENTRY* real_glPushClientAttrib)(GLbitfield);
extern void   (APIENTRY* real_glPopClientAttrib)();
extern void   (APIENTRY* real_glEnable)(GLenum);
extern void   (APIENTRY* real_glDisable)(GLenum);
extern void   (APIENTRY* real_glEnableClientState)(GLenum);
extern void   (APIENTRY* real_glDisableClientState)(GLenum);
extern void   (APIENTRY* real_glBlendFunc)(GLenum, GLenum);
extern void   (APIENTRY* real_glBlendEquation)(GLenum);
extern void   (APIENTRY* real_glScissor)(GLint, GLint, GLsizei, GLsizei);
extern void   (APIENTRY* real_glViewport)(GLint, GLint, GLsizei, GLsizei);
extern void   (APIENTRY* real_glGetIntegerv)(GLenum, GLint*);
extern void   (APIENTRY* real_glGetFloatv)(GLenum, GLfloat*);
extern void   (APIENTRY* real_glPolygonMode)(GLenum, GLenum);
extern void   (APIENTRY* real_glDepthFunc)(GLenum);
extern void   (APIENTRY* real_glDepthMask)(GLboolean);
extern void   (APIENTRY* real_glDepthRange)(GLclampd, GLclampd);
extern void   (APIENTRY* real_glColorMask)(GLboolean, GLboolean, GLboolean, GLboolean);
extern void   (APIENTRY* real_glStencilMask)(GLuint);
extern void   (APIENTRY* real_glClear)(GLbitfield);
extern void   (APIENTRY* real_glClearColor)(GLfloat, GLfloat, GLfloat, GLfloat);
extern void   (APIENTRY* real_glClearDepth)(GLclampd);
extern void   (APIENTRY* real_glBindTexture)(GLenum, GLuint);
extern void   (APIENTRY* real_glGenTextures)(GLsizei, GLuint*);
extern void   (APIENTRY* real_glDeleteTextures)(GLsizei, const GLuint*);
extern void   (APIENTRY* real_glTexImage2D)(GLenum,GLint,GLint,GLsizei,GLsizei,GLint,GLenum,GLenum,const void*);
extern void   (APIENTRY* real_glTexSubImage2D)(GLenum,GLint,GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,const void*);
extern void   (APIENTRY* real_glTexParameteri)(GLenum, GLenum, GLint);
extern void   (APIENTRY* real_glTexParameterf)(GLenum, GLenum, GLfloat);
extern void   (APIENTRY* real_glPixelStorei)(GLenum, GLint);
extern void   (APIENTRY* real_glMatrixMode)(GLenum);
extern void   (APIENTRY* real_glLoadIdentity)();
extern void   (APIENTRY* real_glOrtho)(GLdouble,GLdouble,GLdouble,GLdouble,GLdouble,GLdouble);
extern void   (APIENTRY* real_glReadPixels)(GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,void*);
extern void   (APIENTRY* real_glDrawArrays)(GLenum, GLint, GLsizei);
extern void   (APIENTRY* real_glDrawElements)(GLenum, GLsizei, GLenum, const void*);
extern void   (APIENTRY* real_glLineWidth)(GLfloat);
extern void   (APIENTRY* real_glPointSize)(GLfloat);
extern void   (APIENTRY* real_glShadeModel)(GLenum);
extern void   (APIENTRY* real_glAlphaFunc)(GLenum, GLclampf);
extern const GLubyte* (APIENTRY* real_glGetString)(GLenum);
extern GLenum (APIENTRY* real_glGetError)();
extern void   (APIENTRY* real_glFinish)();
extern void   (APIENTRY* real_glFlush)();

// GLES2 / shader pipeline (resolved via wglGetProcAddress at runtime)
// We store these as plain void* and cast at call site.
extern void* real_glUseProgram;
extern void* real_glGetUniformLocation;
extern void* real_glUniformMatrix4fv;
extern void* real_glGetActiveUniform;
extern void* real_glCreateProgram;
extern void* real_glDeleteProgram;
extern void* real_glCreateShader;
extern void* real_glDeleteShader;
extern void* real_glShaderSource;
extern void* real_glCompileShader;
extern void* real_glAttachShader;
extern void* real_glLinkProgram;
extern void* real_glGetProgramiv;
extern void* real_glGetShaderiv;
extern void* real_glEnableVertexAttribArray;
extern void* real_glDisableVertexAttribArray;
extern void* real_glVertexAttribPointer;
extern void* real_glGenBuffers;
extern void* real_glDeleteBuffers;
extern void* real_glBindBuffer;
extern void* real_glBufferData;
extern void* real_glActiveTexture;
extern void* real_glBlendFuncSeparate;
extern void* real_glBlendEquationSeparate;
extern void* real_glGenerateMipmap;
extern void* real_glGenFramebuffers;
extern void* real_glDeleteFramebuffers;
extern void* real_glBindFramebuffer;
extern void* real_glFramebufferTexture2D;
extern void* real_glGenRenderbuffers;
extern void* real_glDeleteRenderbuffers;
extern void* real_glBindRenderbuffer;
extern void* real_glRenderbufferStorage;
extern void* real_glFramebufferRenderbuffer;
extern void* real_glCheckFramebufferStatus;
extern void* real_glGetAttribLocation;
extern void* real_glBindAttribLocation;
extern void* real_glGetProgramInfoLog;
extern void* real_glGetShaderInfoLog;
extern void* real_glUniform1i;
extern void* real_glUniform1f;
extern void* real_glUniform1fv;
extern void* real_glUniform2f;
extern void* real_glUniform2fv;
extern void* real_glUniform3f;
extern void* real_glUniform3fv;
extern void* real_glUniform4f;
extern void* real_glUniform4fv;
extern void* real_glUniform1iv;
extern void* real_glUniform2iv;
extern void* real_glUniform3iv;
extern void* real_glUniform4iv;
extern void* real_glUniformMatrix2fv;
extern void* real_glUniformMatrix3fv;
extern void* real_glDrawRangeElements;
extern void* real_glStencilFuncSeparate;
extern void* real_glStencilOpSeparate;
extern void* real_glStencilMaskSeparate;
extern void* real_glDepthRangef;
extern void* real_glValidateProgram;

} // extern "C"
