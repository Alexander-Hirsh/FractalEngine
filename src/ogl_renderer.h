#pragma once

#include "glcorearb.h"

// #############################################################################
//                           OpenGL Function Pointers
// #############################################################################
static PFNGLCREATEPROGRAMPROC glCreateProgram_ptr;
static PFNGLDELETETEXTURESPROC glDeleteTextures_ptr;
static PFNGLGENTEXTURESPROC glGenTextures_ptr;
static PFNGLBINDTEXTUREPROC glBindTexture_ptr;
static PFNGLDRAWBUFFERPROC glDrawBuffer_ptr;
static PFNGLDRAWARRAYSPROC glDrawArrays_ptr;
static PFNGLCREATESHADERPROC glCreateShader_ptr;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_ptr;
static PFNGLUNIFORM1FPROC glUniform1f_ptr;
static PFNGLUNIFORM2FVPROC glUniform2fv_ptr;
static PFNGLUNIFORM3FVPROC glUniform3fv_ptr;
static PFNGLUNIFORM1IPROC glUniform1i_ptr;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv_ptr;
static PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor_ptr;
static PFNGLACTIVETEXTUREPROC glActiveTexture_ptr;
static PFNGLBUFFERSUBDATAPROC glBufferSubData_ptr;
static PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced_ptr;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer_ptr;
static PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer_ptr;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus_ptr;
static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers_ptr;
static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D_ptr;
static PFNGLDRAWBUFFERSPROC glDrawBuffers_ptr;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers_ptr;
static PFNGLBLENDFUNCIPROC glBlendFunci_ptr;
static PFNGLBLENDEQUATIONPROC glBlendEquation_ptr;
static PFNGLCLEARBUFFERFVPROC glClearBufferfv_ptr;
static PFNGLSHADERSOURCEPROC glShaderSource_ptr;
static PFNGLCOMPILESHADERPROC glCompileShader_ptr;
static PFNGLGETSHADERIVPROC glGetShaderiv_ptr;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_ptr;
static PFNGLATTACHSHADERPROC glAttachShader_ptr;
static PFNGLLINKPROGRAMPROC glLinkProgram_ptr;
static PFNGLVALIDATEPROGRAMPROC glValidateProgram_ptr;
static PFNGLGETPROGRAMIVPROC glGetProgramiv_ptr;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_ptr;
static PFNGLGENBUFFERSPROC glGenBuffers_ptr;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays_ptr;
static PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation_ptr;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray_ptr;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_ptr;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer_ptr;
static PFNGLBINDBUFFERPROC glBindBuffer_ptr;
static PFNGLBINDBUFFERBASEPROC glBindBufferBase_ptr;
static PFNGLBUFFERDATAPROC glBufferData_ptr;
static PFNGLGETVERTEXATTRIBPOINTERVPROC glGetVertexAttribPointerv_ptr;
static PFNGLUSEPROGRAMPROC glUseProgram_ptr;
static PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays_ptr;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers_ptr;
static PFNGLDELETEPROGRAMPROC glDeleteProgram_ptr;
static PFNGLDETACHSHADERPROC glDetachShader_ptr;
static PFNGLDELETESHADERPROC glDeleteShader_ptr;
static PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced_ptr;
static PFNGLGENERATEMIPMAPPROC glGenerateMipmap_ptr;
static PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback_ptr;
static PFNGLDISPATCHCOMPUTEPROC glDispatchCompute_ptr;
static PFNGLMEMORYBARRIERPROC glMemoryBarrier_ptr;


void loadGlFunctions()
{
  // Load OpenGL Functions from the Operating System / Graphics Card
  glCreateProgram_ptr = (PFNGLCREATEPROGRAMPROC)platformLoadGlFunction("glCreateProgram");
  glDeleteTextures_ptr = (PFNGLDELETETEXTURESPROC)platformLoadGlFunction("glDeleteTextures");
  glGenTextures_ptr = (PFNGLGENTEXTURESPROC)platformLoadGlFunction("glGenTextures");
  glBindTexture_ptr = (PFNGLBINDTEXTUREPROC)platformLoadGlFunction("glBindTexture");
  glDrawArrays_ptr = (PFNGLDRAWARRAYSPROC)platformLoadGlFunction("glDrawArrays");
  glCreateShader_ptr = (PFNGLCREATESHADERPROC) platformLoadGlFunction("glCreateShader");
  glGetUniformLocation_ptr = (PFNGLGETUNIFORMLOCATIONPROC) platformLoadGlFunction("glGetUniformLocation");
  glUniform1f_ptr = (PFNGLUNIFORM1FPROC) platformLoadGlFunction("glUniform1f");
  glUniform2fv_ptr = (PFNGLUNIFORM2FVPROC) platformLoadGlFunction("glUniform2fv");
  glUniform3fv_ptr = (PFNGLUNIFORM3FVPROC) platformLoadGlFunction("glUniform3fv");
  glUniform1i_ptr = (PFNGLUNIFORM1IPROC) platformLoadGlFunction("glUniform1i");
  glUniformMatrix4fv_ptr = (PFNGLUNIFORMMATRIX4FVPROC) platformLoadGlFunction("glUniformMatrix4fv");
  glVertexAttribDivisor_ptr = (PFNGLVERTEXATTRIBDIVISORPROC) platformLoadGlFunction("glVertexAttribDivisor");
  glActiveTexture_ptr = (PFNGLACTIVETEXTUREPROC) platformLoadGlFunction("glActiveTexture");
  glBufferSubData_ptr = (PFNGLBUFFERSUBDATAPROC) platformLoadGlFunction("glBufferSubData");
  glDrawArraysInstanced_ptr = (PFNGLDRAWARRAYSINSTANCEDPROC) platformLoadGlFunction("glDrawArraysInstanced");
  glBindFramebuffer_ptr = (PFNGLBINDFRAMEBUFFERPROC) platformLoadGlFunction("glBindFramebuffer");
    glBlitFramebuffer_ptr = (PFNGLBLITFRAMEBUFFERPROC) platformLoadGlFunction("glBlitFramebuffer");
  glCheckFramebufferStatus_ptr = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) platformLoadGlFunction("glCheckFramebufferStatus");
  glGenFramebuffers_ptr = (PFNGLGENFRAMEBUFFERSPROC) platformLoadGlFunction("glGenFramebuffers");
  glFramebufferTexture2D_ptr = (PFNGLFRAMEBUFFERTEXTURE2DPROC) platformLoadGlFunction("glFramebufferTexture2D");
  glDrawBuffers_ptr = (PFNGLDRAWBUFFERSPROC) platformLoadGlFunction("glDrawBuffers");
  glDeleteFramebuffers_ptr = (PFNGLDELETEFRAMEBUFFERSPROC) platformLoadGlFunction("glDeleteFramebuffers");
  glBlendFunci_ptr = (PFNGLBLENDFUNCIPROC) platformLoadGlFunction("glBlendFunci");
  glBlendEquation_ptr = (PFNGLBLENDEQUATIONPROC) platformLoadGlFunction("glBlendEquation");
  glClearBufferfv_ptr = (PFNGLCLEARBUFFERFVPROC) platformLoadGlFunction("glClearBufferfv");
  glShaderSource_ptr = (PFNGLSHADERSOURCEPROC) platformLoadGlFunction("glShaderSource");
  glCompileShader_ptr = (PFNGLCOMPILESHADERPROC) platformLoadGlFunction("glCompileShader");
  glGetShaderiv_ptr = (PFNGLGETSHADERIVPROC) platformLoadGlFunction("glGetShaderiv");
  glGetShaderInfoLog_ptr = (PFNGLGETSHADERINFOLOGPROC) platformLoadGlFunction("glGetShaderInfoLog");
  glAttachShader_ptr = (PFNGLATTACHSHADERPROC) platformLoadGlFunction("glAttachShader");
  glLinkProgram_ptr = (PFNGLLINKPROGRAMPROC) platformLoadGlFunction("glLinkProgram");
  glValidateProgram_ptr = (PFNGLVALIDATEPROGRAMPROC) platformLoadGlFunction("glValidateProgram");
  glGetProgramiv_ptr = (PFNGLGETPROGRAMIVPROC) platformLoadGlFunction("glGetProgramiv");
  glGetProgramInfoLog_ptr = (PFNGLGETPROGRAMINFOLOGPROC) platformLoadGlFunction("glGetProgramInfoLog");
  glGenBuffers_ptr = (PFNGLGENBUFFERSPROC) platformLoadGlFunction("glGenBuffers");
  glGenVertexArrays_ptr = (PFNGLGENVERTEXARRAYSPROC) platformLoadGlFunction("glGenVertexArrays");
  glGetAttribLocation_ptr = (PFNGLGETATTRIBLOCATIONPROC) platformLoadGlFunction("glGetAttribLocation");
  glBindVertexArray_ptr = (PFNGLBINDVERTEXARRAYPROC) platformLoadGlFunction("glBindVertexArray");
  glEnableVertexAttribArray_ptr = (PFNGLENABLEVERTEXATTRIBARRAYPROC) platformLoadGlFunction("glEnableVertexAttribArray");
  glVertexAttribPointer_ptr = (PFNGLVERTEXATTRIBPOINTERPROC) platformLoadGlFunction("glVertexAttribPointer");
  glBindBuffer_ptr = (PFNGLBINDBUFFERPROC) platformLoadGlFunction("glBindBuffer");
  glBindBufferBase_ptr = (PFNGLBINDBUFFERBASEPROC) platformLoadGlFunction("glBindBufferBase");
  glBufferData_ptr = (PFNGLBUFFERDATAPROC) platformLoadGlFunction("glBufferData");
  glGetVertexAttribPointerv_ptr = (PFNGLGETVERTEXATTRIBPOINTERVPROC) platformLoadGlFunction("glGetVertexAttribPointerv");
  glUseProgram_ptr = (PFNGLUSEPROGRAMPROC) platformLoadGlFunction("glUseProgram");
  glDeleteVertexArrays_ptr = (PFNGLDELETEVERTEXARRAYSPROC) platformLoadGlFunction("glDeleteVertexArrays");
  glDeleteBuffers_ptr = (PFNGLDELETEBUFFERSPROC) platformLoadGlFunction("glDeleteBuffers");
  glDeleteProgram_ptr = (PFNGLDELETEPROGRAMPROC) platformLoadGlFunction("glDeleteProgram");
  glDetachShader_ptr = (PFNGLDETACHSHADERPROC) platformLoadGlFunction("glDetachShader");
  glDeleteShader_ptr = (PFNGLDELETESHADERPROC) platformLoadGlFunction("glDeleteShader");
  glDrawElementsInstanced_ptr = (PFNGLDRAWELEMENTSINSTANCEDPROC) platformLoadGlFunction("glDrawElementsInstanced");
  glGenerateMipmap_ptr = (PFNGLGENERATEMIPMAPPROC) platformLoadGlFunction("glGenerateMipmap");
  glDebugMessageCallback_ptr = (PFNGLDEBUGMESSAGECALLBACKPROC)platformLoadGlFunction("glDebugMessageCallback");
  glDispatchCompute_ptr = (PFNGLDISPATCHCOMPUTEPROC) platformLoadGlFunction("glDispatchCompute");
  glMemoryBarrier_ptr = (PFNGLMEMORYBARRIERPROC) platformLoadGlFunction("glMemoryBarrier");
}

// #############################################################################
//                           OpenGL Function Wrappers
// #############################################################################
GLAPI GLuint APIENTRY glCreateProgram (void)
{
  return glCreateProgram_ptr();
}

GLAPI void APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures)
{
  glDeleteTextures_ptr(n, textures);
}

GLAPI void APIENTRY glGenTextures (GLsizei n, GLuint *textures)
{
  glGenTextures_ptr(n, textures);
}

GLAPI void APIENTRY glBindTexture (GLenum target, GLuint texture)
{
  glBindTexture_ptr(target, texture);
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    glDrawArrays_ptr(mode, first, count);
}

GLuint glCreateShader(GLenum shaderType)
{
    return glCreateShader_ptr(shaderType);
}

GLint glGetUniformLocation(GLuint program, const GLchar* name)
{
    return glGetUniformLocation_ptr(program, name);
}

void glUniform1f(GLint location, GLfloat v0)
{
    glUniform1f_ptr(location, v0);
}

void glUniform2fv(GLint location, GLsizei count, const GLfloat* value)
{
    glUniform2fv_ptr(location, count, value);
}

void glUniform3fv(GLint location, GLsizei count, const GLfloat* value)
{
    glUniform3fv_ptr(location, count, value);
}

void glUniform1i(GLint location, GLint v0)
{
    glUniform1i_ptr(location, v0);
}

void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    glUniformMatrix4fv_ptr(location, count, transpose, value);
}

void glVertexAttribDivisor(GLuint index, GLuint divisor)
{
    glVertexAttribDivisor_ptr(index, divisor);
}

void glActiveTexture(GLenum texture)
{
    glActiveTexture_ptr(texture);
}

void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
    glBufferSubData_ptr(target, offset, size, data);
}

void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount)
{
    glDrawArraysInstanced_ptr(mode, first, count, instanceCount);
}

void glBindFramebuffer(GLenum target, GLuint framebuffer)
{
    glBindFramebuffer_ptr(target, framebuffer);
}

void glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                       GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                       GLbitfield mask, GLenum filter)
{
    glBlitFramebuffer_ptr(srcX0, srcY0, srcX1, srcY1,
                          dstX0, dstY0, dstX1, dstY1, mask, filter);
}

GLenum glCheckFramebufferStatus(GLenum target)
{
    return glCheckFramebufferStatus_ptr(target);
}

void glGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
    glGenFramebuffers_ptr(n, framebuffers);
}

void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    glFramebufferTexture2D_ptr(target, attachment, textarget, texture, level);
}

void glDrawBuffers(GLsizei n, const GLenum* bufs)
{
    glDrawBuffers_ptr(n, bufs);
}

void glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
    glDeleteFramebuffers_ptr(n, framebuffers);
}

void glBlendFunci(GLuint buf, GLenum src, GLenum dst)
{
    glBlendFunci_ptr(buf, src, dst);
}

void glBlendEquation(GLenum mode)
{
    glBlendEquation_ptr(mode);
}

void glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
    glClearBufferfv_ptr(buffer, drawbuffer, value);
}

void glShaderSource(GLuint shader, GLsizei count, const GLchar* const* strings, const GLint* lengths)
{
    glShaderSource_ptr(shader, count, strings, lengths);
}

void glCompileShader(GLuint shader)
{
    glCompileShader_ptr(shader);
}

void glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
    glGetShaderiv_ptr(shader, pname, params);
}

void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    glGetShaderInfoLog_ptr(shader, bufSize, length, infoLog);
}

void glAttachShader(GLuint program, GLuint shader)
{
    glAttachShader_ptr(program, shader);
}

void glLinkProgram(GLuint program)
{
    glLinkProgram_ptr(program);
}

void glValidateProgram(GLuint program)
{
    glValidateProgram_ptr(program);
}

void glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
    glGetProgramiv_ptr(program, pname, params);
}

void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
    glGetProgramInfoLog_ptr(program, bufSize, length, infoLog);
}

void glGenBuffers(GLsizei n, GLuint* buffers)
{
    glGenBuffers_ptr(n, buffers);
}

void glGenVertexArrays(GLsizei n, GLuint* arrays)
{
    glGenVertexArrays_ptr(n, arrays);
}

GLint glGetAttribLocation(GLuint program, const GLchar* name)
{
    return glGetAttribLocation_ptr(program, name);
}

void glBindVertexArray(GLuint array)
{
    glBindVertexArray_ptr(array);
}

void glEnableVertexAttribArray(GLuint index)
{
    glEnableVertexAttribArray_ptr(index);
}

void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
    glVertexAttribPointer_ptr(index, size, type, normalized, stride, pointer);
}

void glBindBuffer(GLenum target, GLuint buffer)
{
    glBindBuffer_ptr(target, buffer);
}

void glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
    glBindBufferBase_ptr(target, index, buffer);
}

void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
    glBufferData_ptr(target, size, data, usage);
}

void glGetVertexAttribPointerv(GLuint index, GLenum pname, void** pointer)
{
    glGetVertexAttribPointerv_ptr(index, pname, pointer);
}

void glUseProgram(GLuint program)
{
    glUseProgram_ptr(program);
}

void glDeleteVertexArrays(GLsizei n, const GLuint* arrays)
{
    glDeleteVertexArrays_ptr(n, arrays);
}

void glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
    glDeleteBuffers_ptr(n, buffers);
}

void glDeleteProgram(GLuint program)
{
    glDeleteProgram_ptr(program);
}

void glDetachShader (GLuint program, GLuint shader)
{
    glDetachShader_ptr(program, shader);
}

void glDeleteShader(GLuint shader)
{
    glDeleteShader_ptr(shader);
}

void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount)
{
    glDrawElementsInstanced_ptr(mode, count, type, indices, instancecount);
}

void glGenerateMipmap(GLenum target)
{
    glGenerateMipmap_ptr(target);
}

void glDebugMessageCallback (GLDEBUGPROC callback, const void *userParam)
{
  glDebugMessageCallback_ptr(callback, userParam);
}

void glDispatchCompute(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ)
{
  glDispatchCompute_ptr(numGroupsX, numGroupsY, numGroupsZ);
}

void glMemoryBarrier(GLbitfield barriers)
{
  glMemoryBarrier_ptr(barriers);
}