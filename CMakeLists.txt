cmake_minimum_required(VERSION 3.20)
project(miditoqwerty)
set(CMAKE_CXX_STANDARD 17)
list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

find_package(SDL2 REQUIRED)

find_package(PortMidi REQUIRED)

# Generate gl3w headers and sources
execute_process(
COMMAND cmake .
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gl3w
)

execute_process(
COMMAND cmake --build .
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gl3w
)

set(gl3w_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gl3w/include)

add_library(gl3w ${CMAKE_CURRENT_SOURCE_DIR}/gl3w/src/gl3w.c)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIMGUI_IMPL_OPENGL_LOADER_GL3W -Wall")

include_directories(
    ${gl3w_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${PORTMIDI_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/imgui/backends
	${CMAKE_SOURCE_DIR}/src
    )

add_library (imgui 
    imgui/backends/imgui_impl_sdl.cpp imgui/backends/imgui_impl_opengl3.cpp imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp imgui/imgui_tables.cpp)

add_executable(miditoqwerty
    WIN32
	src/main.cpp
	src/Audio.cpp src/inpututils.cpp src/Log.cpp src/Midi.cpp src/Piano.cpp src/settings.cpp src/themes.cpp src/util.cpp imgui/backends/imgui_impl_sdl.cpp
	src/Midi.h src/themes.h src/util.h src/settings.h src/porttime.h src/resource.h src/inpututils.h src/Piano.h src/Qwerty.h src/Log.h src/Audio.h
	${CMAKE_CURRENT_SOURCE_DIR}/res/resource.rc
)
target_include_directories(miditoqwerty PRIVATE src imgui imgui/backends)
# target_link_libraries(miditoqwerty fmt)

# For g++ < 9
#target_link_libraries(miditoqwerty ${PORTMIDI_LIBRARY} ${SDL2_DIR}/lib/x64/SDL2.lib imgui stdc++fs gl3w ${CMAKE_DL_LIBS})

# For MSVC
target_link_libraries(miditoqwerty ${PORTMIDI_LIBRARY} ${SDL2_DIR}/lib/x64/SDL2.lib imgui gl3w ${CMAKE_DL_LIBS})
