#include "editor.h"
#include "stdio.h"
#include "windows.h"

using namespace Leviathan;

void Editor::beginFrame(const unsigned long time)
{
	lastFrameStart = time;
}

void Editor::endFrame(const unsigned long time)
{
	lastFrameStop = time;
}

void Editor::printFrameStatistics()
{
	const int frameTime = lastFrameStop - lastFrameStart;

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
}

double Editor::handleEvents(Leviathan::Song* track, double position)
{
	if (GetAsyncKeyState(VK_MENU))
	{
		double seek = 0.0;
		if (GetAsyncKeyState(VK_DOWN)) track->pause();
		if (GetAsyncKeyState(VK_UP))   track->play();
		if (GetAsyncKeyState(VK_RIGHT) && !GetAsyncKeyState(VK_SHIFT)) seek += 1.0;
		if (GetAsyncKeyState(VK_LEFT) && !GetAsyncKeyState(VK_SHIFT)) seek -= 1.0;
		if (GetAsyncKeyState(VK_RIGHT) && GetAsyncKeyState(VK_SHIFT))  seek += 0.1;
		if (GetAsyncKeyState(VK_LEFT) && GetAsyncKeyState(VK_SHIFT))  seek -= 0.1;
		if (position + seek != position)
		{
			position += seek;
			track->seek(position);
		}
	}

	return position;
}
