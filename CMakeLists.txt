cmake_minimum_required(VERSION 3.20)
project(ProofReader)

set(CMAKE_WIN32_EXECUTABLE ON)
set(saucer_webview2_arch "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" CACHE STRING "")

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

if (NOT EXISTS "${CMAKE_SOURCE_DIR}/build/frontend")
    execute_process(COMMAND npm.cmd ci WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/frontend" COMMAND_ERROR_IS_FATAL ANY)
    execute_process(COMMAND npm.cmd run build WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/frontend" COMMAND_ERROR_IS_FATAL ANY)
endif()
saucer_embed("${CMAKE_SOURCE_DIR}/build/frontend" TARGET ProofReader)
