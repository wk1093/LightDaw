# LightDAW

LightDAW is a lightweight digital audio workstation (DAW) for Windows, macOS, and Linux. It is designed to be simple and easy to use, while still providing the essential features of a DAW.

As of right now it is VERY early in development, and is not yet usable. However, I am working on it actively and hope to have a working version soon.

## Features

 - Custom simplistic plugin API (todo)
 - Purely software based audio processing
 - Cross-platform support
 - Uses portaudio for audio I/O
 - All audio is mixed in 32-bit floating point before being sent to the audio device
 - Uses a custom file format for saving projects
 - Only uses a small amount of external libraries (check the `CMakeLists.txt` for more info)

## WIP

 - Plugin API
 - Stereo audio support (currently only mono)
 - Piano roll
 - Mixer (channel strip with effects, volume, pan, etc.)
 - Audio Recording
 - Audio Effects
 - Automation
 - VST3 support
 - CLAP support
 - VST2 support
 - MIDI devices
 - Audio file import/export
 - Audio-based instrument (sampler)
 - Cleanup main function
 - Separate headers into their own source and header pairs
 - Comments
 - Documentation
 - Testing