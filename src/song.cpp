#include <string>
#include "song.h"
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ComCtl32.lib")
#pragma comment(lib, "winmm.lib")

#pragma comment(lib, "strmbase.lib")

using namespace Leviathan;

Song::Song()
{
}

Song::Song(LPCWSTR path) : playing(false)
{
	__int64 trackLength;
	IGraphBuilder * graph;

	CoInitialize(0);
	CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC, IID_IGraphBuilder, (void**)&graph);

	graph->QueryInterface(IID_IMediaControl, (void**)&mediaControl);
	graph->QueryInterface(IID_IMediaSeeking, (void**)&mediaSeeking);
	graph->QueryInterface(IID_IBasicAudio, (void**)&audioControl);
	audioControl->put_Volume(long(-90000));

	HRESULT hr = graph->RenderFile(path, 0);

	if (hr == S_OK)
		printf("Audio file opened successfully\n");
	else
		printf("Failed to open audio file\n");

	graph->Release();

	mediaSeeking->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
	mediaSeeking->GetDuration(&trackLength);
	length = (long double)(trackLength) / (long double)10000000.0;

	__int64 position = 0;
	mediaSeeking->SetPositions(&position, AM_SEEKING_AbsolutePositioning, &position, AM_SEEKING_NoPositioning);
	pause();
}

Song::~Song()
{
	audioControl->Release();
	mediaControl->Release();
	mediaSeeking->Release();
}

int Song::play()
{
	mediaControl->Run();
	playing = true;
	return 0;
}

int Song::pause()
{
	mediaControl->Stop();
	playing = false;
	return 0;
}

int Song::toggle()
{
	playing = !playing;
	if (playing)
		play();
	else
		pause();
	return 0;
}

bool Song::is_playing()
{
	OAFilterState state;
	mediaControl->GetState(INFINITE, &state);
	__int64 trackLength, position;
	mediaSeeking->GetDuration(&trackLength);
	mediaSeeking->GetCurrentPosition(&position);
	return position<trackLength;
}

int Song::seek(long double position)
{
	if (position>length)
		position = length;
	if (position<.0)
		position = .0;

	__int64 seekPosition = __int64(10000000.0*position);

	if (playing)
		mediaControl->Stop();
	mediaSeeking->SetPositions(&seekPosition, AM_SEEKING_AbsolutePositioning, &seekPosition, AM_SEEKING_NoPositioning);
	if (playing)
		mediaControl->Run();

	return 0;
}

long double Song::getTime()
{
	__int64 position;
	mediaSeeking->GetCurrentPosition(&position);
	return (long double)(position) / (long double)10000000.0;
}

long double Song::getLength()
{
	return length;
}