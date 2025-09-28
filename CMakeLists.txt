cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(LIBRARY_TYPE STATIC)

project(sepscr C)
add_library(${PROJECT_NAME} ${LIBRARY_TYPE}
    src/frontend/asm.c

    src/backend/unit.c
    src/backend/state.c
    src/backend/value.c

    src/backend/instructions/instruction.c
    src/backend/instructions/standard.c
    src/backend/instructions/numbers.c
    src/backend/instructions/heap.c
)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
if (MSVC)
    set(COMPILE_OPTIONS /W4 /WX /permissive- /sdl /wd4068)
else()
    set(COMPILE_OPTIONS
        -Wall -Wextra -Werror

        -Wpedantic -Wconversion -Wuninitialized
        -Wsign-conversion -Wnull-dereference
    )
endif()
target_compile_options(${PROJECT_NAME} PUBLIC ${COMPILE_OPTIONS})

if (WIN32)
    if (BUILD_SHARED_LIBS)
        set(CMAKE_SHARED_LIBRARY_PREFIX "")
        target_compile_definitions(${PROJECT_NAME} PUBLIC LIB_DYNAMIC)
        target_compile_definitions(${PROJECT_NAME} PRIVATE LIB_EXPORTS)
    endif()
endif()

# Fetch Dependencies
include(FetchContent)
FetchContent_Declare(
        sf-std
        GIT_REPOSITORY https://github.com/septek/sf-std.git
        GIT_TAG master
)
FetchContent_MakeAvailable(sf-std)
target_link_libraries(${PROJECT_NAME} PUBLIC sf-std)

# CTest
enable_testing()
file(GLOB TEST_SRCS tests/*.c)
foreach(TEST_SRC ${TEST_SRCS})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SRC})
    target_link_libraries(${TEST_NAME} PRIVATE ${PROJECT_NAME})
    target_compile_options(${TEST_NAME} PUBLIC ${COMPILE_OPTIONS})
    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_tests_properties(${TEST_NAME} PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()