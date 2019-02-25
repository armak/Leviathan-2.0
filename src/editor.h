namespace Leviathan
{
	// forward declaration
	class Song;
}

namespace Leviathan
{
	// simpler wrapper class for the editor functionality
	class Editor
	{
	public:
		Editor();

		void beginFrame(const unsigned long time);

		void endFrame(const unsigned long time);

		void printFrameStatistics();

		double handleEvents(Song* track, double position);

		void updateShaders(int* mainShaderPID, int* postShaderPID, bool force_update = false);

	private:
		int reloadShaderSource(const char* filename);

		bool compileAndDebugShader(const char* shader, const char* filename, bool kill_on_failure = true);

		enum PlayState {Playing, Paused};
		PlayState state;

		unsigned long lastFrameStart;
		unsigned long lastFrameStop;

		static const int shaderErrorBufferLength = 4096;
		static const int windowSize = 10;
		int timeHistory[windowSize] = {};

		bool shaderUpdatePending = false;
		int previousUpdateTime;

		double trackPosition;
		double trackEnd;
	};
}
