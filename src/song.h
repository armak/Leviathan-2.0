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

		virtual void play();

		virtual void pause();

		virtual void toggle();

		virtual bool is_playing();

		virtual void seek(long double position);

		virtual long double getTime();

		virtual long double getLength();

	private:
		long double length;
		bool playing;
		IMediaControl * mediaControl;
		IMediaSeeking * mediaSeeking;
		IBasicAudio * audioControl;
	};

    // An implementation of the Song class that just handles the time.
    class NoSong : public Song
    {
    public:
        NoSong() : position(0.0), playing(false), lastTick(0) {}

        ~NoSong() {}

        void play() override {
            if (!playing) {
                playing = true;
                lastTick = GetTickCount64();
            }
        }

        void pause() override {
            if (playing) {
                playing = false;
                position += getElapsedTime();
            }
        }

        void toggle() override {
            return playing ? pause() : play();
        }

        bool is_playing() override {
            return playing;
        }

        void seek(long double position_) override {
            position = position_;
            if (playing) {
                lastTick = GetTickCount64();  // Reset the tick count after seeking
            }
        }

        long double getTime() override {
            if (playing) {
                return position + getElapsedTime();
            }
            return position;
        }

        long double getLength() override {
            return 3600.0; // Arbitrary length (1 hour)
        }

    private:
        long double position;
        bool playing;
        ULONGLONG lastTick;

        long double getElapsedTime() const {
            ULONGLONG currentTick = GetTickCount64();
            return static_cast<long double>(currentTick - lastTick) / 1000.;
        }
    };
}
