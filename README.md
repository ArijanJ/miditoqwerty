# MIDI to Qwerty
(MIDI input to Qwerty output) translator for Virtual Piano, with additional features for supported platforms (sustain, 88-keys, velocity).

### Having trouble?  Visit [the wiki](https://github.com/ArijanJ/miditoqwerty/wiki/Troubleshooting).

![image_4](https://user-images.githubusercontent.com/56356662/182448262-1aaf1803-e401-4e77-9706-b7f6f4bfa4b1.png)

Libraries used:
[The wonderful imgui](https://github.com/ocornut/imgui)
[shric/midi on GitHub](https://github.com/shric/midi)
[PortMidi](https://github.com/PortMidi/portmidi)

Themes inspired by:
[Monkeytype (check it out!)](https://github.com/monkeytypegame/monkeytype)

# Building

## Prerequisites

To build this project, you will need:
- the `imgui` submodule, as it is;
- the `gl3w` submodule, as it is;
- to compile `PortMidi` with CMake;
- a release of [SDL2](https://www.libsdl.org/)

### CMake

| CMake field| Description|  
| ----------- | ----------- |  
| CMAKE_CONFIGURATION_TYPES| Already set (Release/Debug/...)|  
| CMAKE_INSTALL_PREFIX| Same as your working directory|
| PORTMIDI_INCLUDE_DIR| portmidi/pm_common/|
| PORTTIME_INCLUDE_DIR| portmidi/porttime/|
| PORTMIDI_LIBRARY| portmidi.lib from your build|
| SDL2_DIR| SDL2-x.x.xx/|

Build as Release to avoid some `imgui` asserts.
Remember, you still need the DLLs, /themes/ etc. from the release.
