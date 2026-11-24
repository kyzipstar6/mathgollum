cmake_minimum_required(VERSION 3.14)
project(smeagol CXX)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.5)
FetchContent_MakeAvailable(raylib)

add_executable(smeagol main.cpp)
target_link_libraries(smeagol raylib)

# Copy the fonts next to the executable (the game loads fonts/JetBrainsMono-*.ttf).

# Your photos: put them in smeagol/step1.png and smeagol/step2.png next to the executable.
