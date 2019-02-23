#pragma once

#pragma warning(disable:995)
#include <dshow.h>
#pragma warning(default:995)

namespace Leviathan
{
	class Song
	{
	public:
		Song();

		Song(LPCWSTR path);

		~Song();

		int play();

		int pause();

		int toggle();

		bool is_playing();

		int seek(long double position);

		long double getTime();

		long double getLength();

	private:
		long double length;
		bool playing;
		IMediaControl * mediaControl;
		IMediaSeeking * mediaSeeking;
		IBasicAudio * audioControl;
	};
}
