cmake_minimum_required (VERSION 3.0)

project (kdelibs)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

if (NOT CMAKE_BUILD_TYPE)
    #        set(CMAKE_BUILD_TYPE Release)
endif()

# additional cmake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(TIFF REQUIRED)
find_package(X11 REQUIRED)
find_package(Osiris REQUIRED)
find_package(Gettext REQUIRED)

include(CMakePackageConfigHelpers)
include(KDE1Macros)
include(KDE1InstallDirs)

set(KDELIBS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(common)

add_definitions( -DHAVE_CONFIG_H )

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wno-write-strings")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpermissive -std=c++98")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()

# Generate ldconfig file
include(GNUInstallDirs)

if(CMAKE_SYSTEM_NAME MATCHES "Linux" AND (NOT DEFINED LIB_SUFFIX) AND (NOT CMAKE_CROSSCOMPILING) AND (CMAKE_SIZEOF_VOID_P EQUAL "8") AND (CMAKE_INSTALL_PREFIX MATCHES "/opt/kde1" OR (CMAKE_INSTALL_PREFIX MATCHES "/opt/midesktop")))
    SET(LIB_SUFFIX "64")
endif()

set(LDCONF_TEMPLATE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/midesktop.conf.in")

configure_file("${LDCONF_TEMPLATE_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/midesktop.conf" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/midesktop.conf" DESTINATION "/etc/ld.so.conf.d/")

#

install(FILES
    cmake/FindKDE1.cmake
    cmake/FindOsiris.cmake
    cmake/KDE1Macros.cmake
    cmake/KDE1InstallDirs.cmake
    cmake/KDE1Macros.cmake
    cmake/OsirisMacros.cmake
    DESTINATION ${CMAKE_ROOT}/Modules)

add_subdirectory(doc)
add_subdirectory(kdecore)
add_subdirectory(kdeui)
add_subdirectory(kfile)
add_subdirectory(kfmlib)
add_subdirectory(jscript)
add_subdirectory(kimgio)
add_subdirectory(khtmlw)
add_subdirectory(mediatool)
add_subdirectory(kspell)
add_subdirectory(kab)
add_subdirectory(toolbar)
add_subdirectory(po)
