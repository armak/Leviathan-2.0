// custom build and feature flags
#ifdef DEBUG
	#define OPENGL_DEBUG        1
	#define FULLSCREEN          0
	#define DESPERATE           0
	#define BREAK_COMPATIBILITY 0
#else
	#define OPENGL_DEBUG        0
	#define FULLSCREEN          1
	#define DESPERATE           0
	#define BREAK_COMPATIBILITY 0
#endif

#define TWO_PASS    1
#define USE_MIPMAPS 1
#define USE_AUDIO   1
#define NO_UNIFORMS 0

#include "definitions.h"
#if OPENGL_DEBUG
	#include "debug.h"
#endif

#include "glext.h"
#include "shaders/fragment.inl"
#if TWO_PASS
	#include "shaders/post.inl"
#endif

#ifndef EDITOR_CONTROLS
void entrypoint(void)
#else
#include "song.h"
int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#endif
{
	// initialize window
	#if FULLSCREEN
		ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN);
		ShowCursor(0);
		const HDC hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_POPUP | WS_VISIBLE, 0, 0, XRES, YRES, 0, 0, 0, 0));
	#else
		#ifdef EDITOR_CONTROLS
			HWND window = CreateWindow("static", 0, WS_POPUP | WS_VISIBLE, 0, 0, XRES, YRES, 0, 0, 0, 0);
			HDC hDC = GetDC(window);
		#else
			HDC hDC = GetDC(CreateWindow("static", 0, WS_POPUP | WS_VISIBLE, 0, 0, XRES, YRES, 0, 0, 0, 0));
		#endif
	#endif	

	// initalize opengl
	SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
	wglMakeCurrent(hDC, wglCreateContext(hDC));
	#ifdef EDITOR_CONTROLS
		int pid = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))(GL_FRAGMENT_SHADER, 1, &fragment);
	#else
		const int pid = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))(GL_FRAGMENT_SHADER, 1, &fragment);
	#endif
	#if TWO_PASS
		#ifdef EDITOR_CONTROLS
			int pi2 = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))(GL_FRAGMENT_SHADER, 1, &post);
		#else
			const int pi2 = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))(GL_FRAGMENT_SHADER, 1, &post);
		#endif
	#endif

	#if OPENGL_DEBUG
		shaderDebug(fragment
			#ifdef EDITOR_CONTROLS
				,false
			#endif
		);
		#if TWO_PASS
			shaderDebug(post
				#ifdef EDITOR_CONTROLS
					,false
				#endif
			);
		#endif
	#endif

	// initialize sound
	#ifndef EDITOR_CONTROLS
		#if USE_AUDIO
			CreateThread(0, 0, (LPTHREAD_START_ROUTINE)_4klang_render, lpSoundBuffer, 0, 0);
			waveOutOpen(&hWaveOut, WAVE_MAPPER, &WaveFMT, NULL, 0, CALLBACK_NULL);
			waveOutPrepareHeader(hWaveOut, &WaveHDR, sizeof(WaveHDR));
			waveOutWrite(hWaveOut, &WaveHDR, sizeof(WaveHDR));
		#endif
	#else
		long double position = 0.0;
		song track(L"audio.wav");
		track.play();
		start = timeGetTime();
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
			// if you don't have an audio system figure some other way to pass time to your shader
			#if USE_AUDIO
				waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));
				// it is possible to upload your vars as vertex color attribute (gl_Color) to save one function import
				#if NO_UNIFORMS
					glColor3ui(MMTime.u.sample, 0, 0);
				#else
					// remember to divide your shader time variable with the SAMPLE_RATE (44100 with 4klang)
					((PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i"))(0, MMTime.u.sample);
				#endif
			#endif
		#else
			refreshShaders(pid, pi2);
			position = track.getTime();
			((PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i"))(0, ((int)(position*44100.0)));
		#endif
		glRects(-1, -1, 1, 1);

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
			if(GetAsyncKeyState(VK_MENU))
			{
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
			}
		#endif
	} while(!GetAsyncKeyState(VK_ESCAPE)
		#if USE_AUDIO
			&& MMTime.u.sample < MAX_SAMPLES
		#endif
	);

	ExitProcess(0);
}
