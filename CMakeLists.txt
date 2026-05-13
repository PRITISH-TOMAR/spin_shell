cmake_minimum_required(VERSION 3.13)

project(shell-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard

# Statically link standard libraries to avoid missing DLL issues on Windows
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(shell ${SOURCE_FILES})

# target_link_libraries(shell PRIVATE readline)
