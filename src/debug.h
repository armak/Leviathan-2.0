// This header contains some useful functions for debugging OpenGL.
// Remember to disable them when building your final releases.

#include <windows.h>
#include <GL/gl.h>
#include "glext.h"

static GLchar* getErrorString(GLenum errorCode)
{
	if (errorCode == GL_NO_ERROR) {
		return (GLchar*) "No error";
	}
	else if (errorCode == GL_INVALID_VALUE) {
		return (GLchar*) "Invalid value";
	}
	else if (errorCode == GL_INVALID_ENUM) {
		return (GLchar*) "Invalid enum";
	}
	else if (errorCode == GL_INVALID_OPERATION) {
		return (GLchar*) "Invalid operation";
	}
	else if (errorCode == GL_STACK_OVERFLOW) {
		return (GLchar*) "Stack overflow";
	}
	else if (errorCode == GL_STACK_UNDERFLOW) {
		return (GLchar*) "Stack underflow";
	}
	else if (errorCode == GL_OUT_OF_MEMORY) {
		return (GLchar*) "Out of memory";
	}
	return (GLchar*) "Unknown";
}

static void assertGlError(const char* error_message)
{
	const GLenum ErrorValue = glGetError();
	if (ErrorValue == GL_NO_ERROR) return;

	const char* APPEND_DETAIL_STRING = ": %s\n";
	const size_t APPEND_LENGTH = strlen(APPEND_DETAIL_STRING) + 1;
	const size_t message_length = strlen(error_message);
	MessageBox(NULL, error_message, getErrorString(ErrorValue), 0x00000000L);
	ExitProcess(0);
}

static void shaderDebug(const char* shader)
{
	// try and compile the shader 
	int result;
	const int debugid = ((PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader"))(GL_FRAGMENT_SHADER);
	((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))(debugid, 1, &shader, 0);
	((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))(debugid);
	
	// get compile result
	((PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv"))(debugid, GL_COMPILE_STATUS, &result);
	if(result == GL_FALSE)
	{	
		// display compile log on failure
		char info[2048];
		((PFNGLGETSHADERINFOLOGPROC) wglGetProcAddress("glGetShaderInfoLog"))(debugid, 2047, NULL, (char*)info);
		MessageBox(NULL, info, "", 0x00000000L);
		ExitProcess(0);
	}
	else
	{
		((PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader"))(debugid);
	}
}

#define STRINGIFY2(x) #x // Thanks sooda!
#define STRINGIFY(x) STRINGIFY2(x)
#define CHECK_ERRORS() assertGlError(STRINGIFY(__LINE__))