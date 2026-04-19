#
# This file is included by FindOsiris.cmake, don't include it directly.

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
# Copyright 2016 Helio Chissini de Castro <helio@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
#
# Modifications to work with Osiris toolkit (C) 2025 Alec Bloss <abloss@libranext.com>

######################################
#
#       Macros for building Osiris files
#
######################################


macro (OSIRIS_EXTRACT_OPTIONS _osiris_files _osiris_options _osiris_target)
  set(${_osiris_files})
  set(${_osiris_options})
  set(_OSIRIS_DOING_OPTIONS FALSE)
  set(_OSIRIS_DOING_TARGET FALSE)
  foreach(_currentArg ${ARGN})
    if ("x${_currentArg}" STREQUAL "xOPTIONS")
      set(_OSIRIS_DOING_OPTIONS TRUE)
    elseif ("x${_currentArg}" STREQUAL "xTARGET")
      set(_OSIRIS_DOING_TARGET TRUE)
    else ()
      if(_OSIRIS_DOING_TARGET)
        set(${_osiris_target} "${_currentArg}")
      elseif(_OSIRIS_DOING_OPTIONS)
        list(APPEND ${_osiris_options} "${_currentArg}")
      else()
        list(APPEND ${_osiris_files} "${_currentArg}")
      endif()
    endif ()
  endforeach()
endmacro ()

macro (OSIRIS_WRAP_HEADER outfiles )
  OSIRIS_EXTRACT_OPTIONS(ui_files ui_options ui_target ${ARGN})

  foreach (it ${ui_files})
    get_filename_component(outfile ${it} NAME_WE)
    set(infile ${CMAKE_CURRENT_SOURCE_DIR}/${outfile}.h)
    set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}.moc)
    add_custom_command(OUTPUT ${outfile}
      COMMAND /opt/osiris/bin/moc
      ARGS ${infile} -o ${outfile}
      MAIN_DEPENDENCY ${infile} VERBATIM)
  list(APPEND ${outfiles} ${outfile})
  endforeach ()

endmacro ()

macro (OSIRIS_WRAP_MOC outfiles )
  OSIRIS_EXTRACT_OPTIONS(ui_files ui_options ui_target ${ARGN})

  foreach (it ${ui_files})
    get_filename_component(filename ${it} NAME_WE)
    set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${filename}.moc)
    set(infile ${CMAKE_CURRENT_SOURCE_DIR}/${filename}.h)
    add_custom_command(OUTPUT ${outfile}
      COMMAND /opt/osiris/bin/moc
      ARGS ${infile} -o ${outfile}
      MAIN_DEPENDENCY ${infile} VERBATIM)
    list(APPEND ${outfiles} ${outfile})
  endforeach ()
  list(APPEND ${outfiles} ${outfile})
  #add_custom_target(moc_ALL ALL DEPENDS ${outfiles})
endmacro ()

macro (OSIRIS_WRAP_CPP outfiles )
  OSIRIS_EXTRACT_OPTIONS(ui_files ui_options ui_target ${ARGN})

  foreach (it ${ui_files})
    get_filename_component(filename ${it} NAME_WE)
    set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${filename}.moc.cpp)
    set(infile ${CMAKE_CURRENT_SOURCE_DIR}/${filename}.h)
    add_custom_command(OUTPUT ${outfile}
      COMMAND /opt/osiris/bin/moc
      ARGS ${infile} -o ${outfile}
      MAIN_DEPENDENCY ${infile} VERBATIM)
    list(APPEND ${outfiles} ${outfile})
  endforeach ()
  list(APPEND ${outfiles} ${outfile})
  #add_custom_target(moc_ALL ALL DEPENDS ${outfiles})
endmacro ()

