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
int __cdecl main(int argc, char* argv[])
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
	
	PID_QUALIFIER int pid = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))(GL_FRAGMENT_SHADER, 1, &fragment);
	#if TWO_PASS
		PID_QUALIFIER int pi2 = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))(GL_FRAGMENT_SHADER, 1, &post);
	#endif

	#if OPENGL_DEBUG
		shaderDebug(fragment, FAIL_KILL);
		#if TWO_PASS
			shaderDebug(post, FAIL_KILL);
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
		// absolute path always works here
		// relative path works only when not ran from visual studio directly
		song track(L"audio.wav");
		track.play();
		start = timeGetTime();

		const int windowSize = 10;
		int timeHistory[windowSize] = {};
	#endif

	// main loop
	do
	{
		#ifdef EDITOR_CONTROLS
			int frameStart = timeGetTime();
		#endif

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
			int frameTime = timeGetTime() - frameStart;

			// calculate average fps over 'windowSize' of frames
			float fps = 0.0f;
			for (int i = 0; i < windowSize - 1; ++i)
			{
				timeHistory[i] = timeHistory[i + 1];
				fps += 1.0f / static_cast<float>(timeHistory[i]);
			}
			timeHistory[windowSize - 1] = frameTime;
			fps += 1.0f / static_cast<float>(frameTime);
			fps *= 1000.0f / static_cast<float>(windowSize);

			printf("Frame duration: %i ms (running fps average: %f)\r", frameTime, fps);

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
