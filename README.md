# Leviathan-2.0
The new generation of 4k intro frameworks. Currently very early in development but seems stable and usable enough give for testing.

## Compatibility
Works in all versions of Visual Studio 2015. Probably in 2013 and 2017 as well but not tested.

## Configurations
This section describes the different build configurations available from Visual Studio IDE
### Release
The real deal. No-nonsense max crinklering with minimum extra. Use this for occasional testing and your final releases (the compression WILL take a while).
### Snapshot
Use for general development. Only minimal crinklering but nothing extra included. Useful still for keeping track of relative size changes.
### Debug
By default includes some debug options, especially for OpenGL. Check out debug.h for more.
### Editor
Creates a bigger exe similar to Debug, but with keyboard controls for pausing and seeking around temporally. Reguires a pre-rendered copy of the audio track used (well, not a must but...).

## TODO
* Automatically dump audio in Editor mode if no .wav already exists.
* Support for more softsynths?
* Editor more overlays (non-interactive seekbar, etc.).
* More preprocessor macros for shader passes.
* Incorporate shader_minifier into the "build chain".
