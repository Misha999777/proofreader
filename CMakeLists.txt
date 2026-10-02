cmake_minimum_required(VERSION 3.20)
project(ProofReader)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WIN32_EXECUTABLE ON)

include(FetchContent)
FetchContent_Declare(
    saucer
    GIT_REPOSITORY https://github.com/saucer/saucer.git
    GIT_TAG        v8.0.5
)
FetchContent_MakeAvailable(saucer)

file(GLOB_RECURSE SOURCES 
    "src/*.cpp" 
    "src/*.hpp"
    "src/*.rc"
)
add_executable(ProofReader ${SOURCES})
target_include_directories(ProofReader PRIVATE src)

target_link_libraries(ProofReader PRIVATE saucer::saucer saucer::embedded saucer::private)
target_link_libraries(ProofReader PRIVATE dwmapi comctl32)

saucer_embed("${CMAKE_SOURCE_DIR}/build/frontend" TARGET ProofReader)
