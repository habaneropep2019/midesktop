#=============================================================================
# Copyright 2016 Helio Chissini de Castro <helio@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)
#
# Modifications to work with Osiris toolkit (C) 2025 Alec Bloss <abloss@libranext.com>

set(OSIRIS_FOUND 0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include(OsirisMacros)

if(UNIX)
  find_path(OSIRIS_INCLUDE_DIR qapp.h
    /opt/osiris/include
    /opt/osiris/include/osiris
    ${OSIRIS_INCLUDE_DIR}
    )

  find_library(OSIRIS_LIBRARIES osiris-mt
    /opt/osiris/lib64
    /opt/osiris/lib
    ${OSIRIS_LIBRARY_DIR}
    )

endif()

# handle the QUIETLY and REQUIRED arguments and set MOTIF_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Osiris DEFAULT_MSG OSIRIS_LIBRARIES OSIRIS_INCLUDE_DIR)

mark_as_advanced(
  OSIRIS_INCLUDE_DIR
  OSIRIS_LIBRARIES
)
