# Leviathan 2.0
Leviathan is a simple "framework" for developing small (around 1-8 kilobyte) [Demoscene](https://en.wikipedia.org/wiki/Demoscene) productions. It is primarily developed for creating the [Prismbeings](https://www.pouet.net/groups.php?which=12646) 4k Intros. "Intro" is parlance for a size-wise small Demo.

## "Features"
* Kept as simple as possible, made for productivity.
* No external dependencies, instantly ready for development.
* Readymade configurations for different use cases.
* Automated shader minification upon compilation.
* Simple unintrusive editor mode with seeking and hot reloading.
* Easy to customize for your needs.

## Compatibility
Current version intended to be used with Visual Studio 2017 (any version). Make sure you have a version with the latest Windows SDK installed (at least version 10.0.17134.0), or use version 8.1. With some extra configuration 2015 and 2013 should work as well. Leviathan is is very Windows-specific and isn't really sensibly portable anywhere else.

## Configurations
This section describes the different build configurations available from Visual Studio IDE
### Heavy Release
Uses heavier Crinkler settings to squeeze out maximum compression (note though that the /VERYSLOW flag could sometimes backfire and produce a larger executable). Also doesn't do shader minification since it assumes the user is performing hand minifications to the .inl source, which would be overwritten by the Shader Minifier.
### Release
Generally recommended for producing a final executable to be released. Uses moderate Crinkler settings.
### Snapshot
Use for general development. Only minimal crinklering but nothing extra included. Useful still for keeping track of relative size changes. This configuration overwrites Release configuration binaries, but doesn't generate and overwrite crinkler report.
### Debug
Deprecated, might work but currently not really useful and not updated. Editor covers everything in this configuration.
### Editor
Creates a bigger exe similar to Debug, but with keyboard controls for pausing and seeking around temporally. Requires a pre-rendered copy of the audio track used (well, not a must but...). Overwrites Debug configuration binaries.

## Build Flags
This section describes the preprocessor definitions available for various features and size optimizations.
### OPENGL_DEBUG
Checks for shader compilation errors on initialization. Also enables to use the CHECK_ERRORS macro on debug.h. You want to leave this disabled in the final release obviously.
### FULLSCREEN
Changes the display mode to fullscreen instead of a static window. You want to use this in your final release but probably not while developing and debugging. Disabling this saves around 20 bytes.
### DESPERATE
Enabling this disables message handling, which isn't strictly necessary but makes running the intro much more reliable and compatible. Enabling this saves around 20 bytes.
### BREAK_COMPATIBILITY
Enabling this uses a pixel format descriptor that uses parameters that are mostly zeroes. This improves the compressability at the cost of violating the API specifications. You might be able to run your intro currently, but might break in the future or on other peoples' configurations right now. Enabling this saves around 5 bytes.
### POST_PASS
Enables using the OpenGL backbuffer as a framebuffer and texture to perform simple post processing or other functionality.
### USE_MIPMAPS
Generates mipmaps for the backbuffer texture.
### USE_AUDIO
Disabling this doesn't include or init 4klang at all.
### NO_UNIFORMS
Enables using the gl_Color vertex attribute to pass variables to the shader instead of the usual uniform uploading. This saves one function import and around 10 bytes.

## Contributing
Fork your own and submit a pull request, ideas always welcome. Please don't add any additional dependencies unless it's a single-file-header library or something similar, and non-GPL licensed. Inclusion of CMake or other such tools is also not considered.

## TODO
* Automatically render and dump audio in Editor mode if no .wav already exists.
* Support for more softsynths?
* Editor mode overlays (non-interactive seekbar, etc.).
* More functionality to add further render passes easily.
* Continue Structuring the code better so not everything is inlined in the main loop.
* Write out a version in assembly to use in the Release configuration.

## Acknowledgements
* Rimina for initial motivation and OpenGL debug functions.
* LJ for giving suggestions for some nice hacks.
* Fizzer for help with implementing some of said hacks.
* iq for the original 4k intro framework.
* Mentor and Blueberry for Crinkler.
* LLB for Shader Minifier. This fork is used in the framework: https://github.com/armak/Shader_Minifier.
* Numerous people for various resources and information: auld, ps, cce, msqrt, ferris, yzi, las to name a few.
