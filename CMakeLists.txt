cmake_minimum_required(VERSION 3.20)
project(miditoqwerty)

find_package(SDL2 REQUIRED)
include_directories(SDL2Test ${SDL2_INCLUDE_DIRS})

# add_subdirectory(fmt)

add_executable(miditoqwerty
    src/main.cpp
)
target_include_directories(miditoqwerty PRIVATE src imgui imgui/backends)
# target_link_libraries(miditoqwerty fmt)
