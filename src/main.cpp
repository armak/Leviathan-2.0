#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define VC_LEANMEAN
#define VC_EXTRALEAN

// custom build and feature flags
#ifdef DEBUG
	#define OPENGL_DEBUG 1
	#define FULLSCREEN   0
	#define CLEAN_EXIT   0
	#define DESPERATE    0
#else
	#define OPENGL_DEBUG 0
	#define FULLSCREEN   1
	#define CLEAN_EXIT   0
	#define DESPERATE    0
#endif

#define TWO_PASS     1
#define USE_MIPMAPS  1

#include "definitions.h"
#if OPENGL_DEBUG
	#include "debug.h"
#endif

#include "shaders/fragment.inl"
#if TWO_PASS
	#include "shaders/post.inl"
#endif

#ifndef EDITOR_CONTROLS
void entrypoint(void)
#else
#include "song.h"
int CALLBACK WinMain(HINSTANCE prev, HINSTANCE self, LPSTR cmd, int show)
#endif
{
	// initialize window
	#if FULLSCREEN
		ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN);
		ShowCursor(0);
		const HDC hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_POPUP | WS_VISIBLE, 0, 0, XRES, YRES, 0, 0, 0, 0));
	#else
		HDC hDC = GetDC(CreateWindow("static", 0, WS_POPUP | WS_VISIBLE, 0, 0, XRES, YRES, 0, 0, 0, 0));
	#endif	

	// initalize opengl
	SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
	wglMakeCurrent(hDC, wglCreateContext(hDC));
	const int pid = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))(GL_FRAGMENT_SHADER, 1, &fragment);
	#if TWO_PASS
		const int pi2 = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))(GL_FRAGMENT_SHADER, 1, &post);
		// not really needed, as long as tid is some value > 0
		//unsigned int tid;
		//glGenTextures(1, &tid);
	#endif

	#if OPENGL_DEBUG
		shaderDebug(fragment);
		#if TWO_PASS
			shaderDebug(post);
		#endif
	#endif

	// initialize sound
	#ifndef EDITOR_CONTROLS
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)_4klang_render, lpSoundBuffer, 0, 0);
		waveOutOpen(&hWaveOut, WAVE_MAPPER, &WaveFMT, NULL, 0, CALLBACK_NULL);
		waveOutPrepareHeader(hWaveOut, &WaveHDR, sizeof(WaveHDR));
		waveOutWrite(hWaveOut, &WaveHDR, sizeof(WaveHDR));
	#else
		GLuint hud_fbo;
		((PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers"))(1, &hud_fbo);
		((PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer"))(GL_FRAMEBUFFER, hud_fbo);
		CHECK_ERRORS();
		unsigned int hud_tex;
		glGenTextures(1, &hud_tex);
		CHECK_ERRORS();
		glBindTexture(GL_TEXTURE_2D, hud_tex);
		CHECK_ERRORS();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, XRES, YRES, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		CHECK_ERRORS();
		glBindTexture(GL_TEXTURE_2D, 0);
		CHECK_ERRORS();
		((PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D"))(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hud_tex, 0);
		CHECK_ERRORS();
		((PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer"))(GL_FRAMEBUFFER, 0);
		CHECK_ERRORS();

		shaderDebug(hud);
		const int hid = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))(GL_FRAGMENT_SHADER, 1, &hud);

		double position = 0.0;
		song track(L"audio.wav");
		track.play();
	#endif

	// main loop
	do
	{
		#if !(DESPERATE)
			// do minimal message handling so windows doesn't kill your application
			// not always strictly necessary but increases compatibility a lot
			MSG msg;
			PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
		#endif

		// render with the primary shader
		((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(pid);
		#ifndef EDITOR_CONTROLS
			waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));
			// remember to divide your shader time variable with the SAMPLE_RATE (44100 with 4klang)
			((PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i"))(0, 4*44100+MMTime.u.sample);
		#else
			position = track.getTime();
			((PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i"))(0, ((int)((position+4.0)*44100.0)));
		#endif
		glRects(-1, -1, 1, 1);

		#ifdef EDITOR_CONTROLS
			//((PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture"))(GL_TEXTURE0);
			CHECK_ERRORS();
			((PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer"))(GL_FRAMEBUFFER, hud_fbo);
			CHECK_ERRORS();
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			CHECK_ERRORS();
			glBindTexture(GL_TEXTURE_2D, hud_tex);
			CHECK_ERRORS();
			((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(hid);
			CHECK_ERRORS();
			//((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))(0, 4.0+position);
			//CHECK_ERRORS();
			((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))(0, (float)XRES, (float)YRES);
			CHECK_ERRORS();
			glRects(-1, -1, 1, 1);
			CHECK_ERRORS();
			glBindTexture(GL_TEXTURE_2D, 0);
			CHECK_ERRORS();
			((PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer"))(GL_FRAMEBUFFER, 0);
			CHECK_ERRORS();
		#endif

		// render "post process" using the opengl backbuffer
		#if TWO_PASS
			glBindTexture(GL_TEXTURE_2D, 1);
			#if USE_MIPMAPS
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, XRES, YRES, 0);
				((PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap"))(GL_TEXTURE_2D);
			#else
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, XRES, YRES, 0);
			#endif
			((PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture"))(GL_TEXTURE0);
			((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(pi2);
			((PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i"))(0, 0);
			glRects(-1, -1, 1, 1);
		#endif

		SwapBuffers(hDC);

		// pausing and seeking enabled in debug mode
		#ifdef EDITOR_CONTROLS
			double seek = 0.0;
			if(GetAsyncKeyState(VK_DOWN)) track.pause();
			if(GetAsyncKeyState(VK_UP))   track.play();
			if(GetAsyncKeyState(VK_RIGHT) && !GetAsyncKeyState(VK_SHIFT)) seek += 1.0;
			if(GetAsyncKeyState(VK_LEFT)  && !GetAsyncKeyState(VK_SHIFT)) seek -= 1.0;
			if(GetAsyncKeyState(VK_RIGHT) && GetAsyncKeyState(VK_SHIFT))  seek += 0.1;
			if(GetAsyncKeyState(VK_LEFT)  && GetAsyncKeyState(VK_SHIFT))  seek -= 0.1;
			if(position+seek != position)
			{
				position += seek;
				track.seek(position);
			}
		#endif
	} while(!GetAsyncKeyState(VK_ESCAPE) && MMTime.u.sample < MAX_SAMPLES);

	#if CLEAN_EXIT
		ChangeDisplaySettings(0, 0);
		ShowCursor(1);
	#endif
	ExitProcess(0);
}
