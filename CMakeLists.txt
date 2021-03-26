cmake_minimum_required(VERSION 3.14)

project(Trilobytes LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MINGW)
    # This allows Catch2 to be built without MinGW having a fit over the size of the object file generated
    add_compile_options("-Wa,-mbig-obj")
endif()

Message("${CMAKE_CXX_COMPILER_ID}")

find_package(QT NAMES Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

include(FetchContent)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
include(json)
include(fmt)
include(catch2)

# Make additional files appear in QtCreator
FILE(GLOB SAVD_GENOMES SavedGenomes/*)
add_custom_target(Other
    SOURCES
    README.md
    ${SAVD_GENOMES}
)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(Utility)

