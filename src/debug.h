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

static bool shaderDebug(const char* shader, bool kill_on_failure = true)
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
		if(kill_on_failure)
		{
			ExitProcess(0);
		}
		else
		{
			return false;
		}
	}
	else
	{
		((PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader"))(debugid);
		return true;
	}
}

#define STRINGIFY2(x) #x // Thanks sooda!
#define STRINGIFY(x) STRINGIFY2(x)
#define CHECK_ERRORS() assertGlError(STRINGIFY(__LINE__))

#ifdef EDITOR_CONTROLS
	#include <stdlib.h>
	#include <stdio.h>

	static int start;
	static unsigned long long lastLoad;

	char* updateShader(const char* filename)
	{
		char* shaderString;
		long inputSize;
		FILE* file = fopen(filename, "r");

		fseek(file, 0, SEEK_END);
		inputSize = ftell(file);
		rewind(file);
		shaderString = (char*) malloc(inputSize * (sizeof(char)));
		fread(shaderString, sizeof(char), inputSize, file);
		fclose(file);

		return shaderString;
	}

	void refreshShaders(int& pid, int& pi2)
	{
		if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState('S'))
		{
			if (timeGetTime() - lastLoad > 500) {
				Sleep(250);
				char* newSource = updateShader("../src/shaders/fragment.frag");
				if (!shaderDebug(newSource, false))
				{
					MessageBox(NULL, newSource, "", 0x00000000L);
				}
				pid = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))(GL_FRAGMENT_SHADER, 1, &newSource);

				newSource = updateShader("../src/shaders/post.frag");
				if (!shaderDebug(newSource, false))
				{
					MessageBox(NULL, newSource, "", 0x00000000L);
				}
				pi2 = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))(GL_FRAGMENT_SHADER, 1, &newSource);
			}
			lastLoad = timeGetTime()-start;
			
		}
	}
#endif