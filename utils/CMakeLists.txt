cmake_minimum_required (VERSION 3.0)

project (kdeutils)

#set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

# additional cmake files
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(KDE1 REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Osiris REQUIRED)

set(KDEUTILS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(BOOTSTRAP OFF)

#include_directories(common)

#add_definitions( -DHAVE_CONFIG_H )

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wno-write-strings -Wno-error=incompatible-pointer-types")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpermissive -std=c++98")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()

add_subdirectory(kwrite)
add_subdirectory(kcalc)
add_subdirectory(knotes)
add_subdirectory(khexdit)
add_subdirectory(ark)
