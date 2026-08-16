cmake_minimum_required (VERSION 3.5)

project (kdebase)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

# additional cmake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(KDE1 REQUIRED)

set(KDEBASE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(BOOTSTRAP OFF)

include_directories(common)

add_definitions( -DHAVE_CONFIG_H )

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wno-write-strings -Wno-error=incompatible-pointer-types")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fpermissive -std=c++98")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(CURSES ncurses)
pkg_check_modules(JPEG jpeg)

add_subdirectory(kcheckpass)
add_subdirectory(kfm)
add_subdirectory(mimetypes)
add_subdirectory(kdehelp)
add_subdirectory(kfontmanager)
add_subdirectory(kfind)
if(CURSES_FOUND)
add_subdirectory(kvt)
endif()
add_subdirectory(kwm)
add_subdirectory(kpanel)
add_subdirectory(mipanel)
add_subdirectory(kwmcom)
add_subdirectory(kwmmodules)
add_subdirectory(krootwm)
#add_subdirectory(kdm)
#add_subdirectory(kscreensaver)
add_subdirectory(kmenuedit)
add_subdirectory(kaudio)
add_subdirectory(kcontrol)
#add_subdirectory(kappfinder)
add_subdirectory(pics)
add_subdirectory(applnk)
add_subdirectory(doc)
add_subdirectory(config)
add_subdirectory(kstart)
add_subdirectory(kikbd)
add_subdirectory(konsole)
#add_subdirectory(po)
add_subdirectory(miwelcome)

# Generate and install system config files

## profile.d - set PATH and KDEDIR

set(PROFILE_TEMPLATE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/configfiles_in/midesktop.sh.in")

configure_file("${PROFILE_TEMPLATE_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/midesktop.sh" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/midesktop.sh" DESTINATION "/etc/profile.d/" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

## XSession .desktop so MiDesktop shows up in display managers

set(XSESSION_TEMPLATE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/configfiles_in/mi.desktop.in")

configure_file("${XSESSION_TEMPLATE_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/mi.desktop" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mi.desktop" DESTINATION "/usr/share/xsessions/" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

## Install compatibility header since we only use SGI STL in modern times and some old applications expect which STL to be defined (i.e. SGI vs HP STL). If for some reason you are still using HP STL, uhh

set(SGI_STL_COMPAT_HEADER_TEMPLATE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/configfiles_in/midesktop_config.h.in")

configure_file("${SGI_STL_COMPAT_HEADER_TEMPLATE_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/midesktop_config.h" @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/midesktop_config.h" DESTINATION "${CMAKE_INSTALL_PREFIX}/include/" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
