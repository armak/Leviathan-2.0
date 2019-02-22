#include "song.h"

namespace Leviathan
{
	// simpler wrapper class for the editor functionality
	class Editor
	{
	public:
		Editor()
		{
		}

		void beginFrame(const unsigned long time);

		void endFrame(const unsigned long time);

		void printFrameStatistics();

		double handleEvents(Song* track, double position);

	private:
		unsigned long lastFrameStart;
		unsigned long lastFrameStop;

		static const int windowSize = 10;
		int timeHistory[windowSize] = {};
	};
}
