include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

# TODO
#add_subdirectory(docs)


set(MIARK_SRCS
	adddlg.cc ar.cc arch.cc extractdlg.cc karchive.cc
                 arkwidget.cc lha.cc main.cc tar.cc zip.cc arkprocess.cc text.c)

set(MIARK_HEADERS
	adddlg.h errors.h lha.h zip.h ar.h extractdlg.h arkwidget.h tar.h arch.h karchive.h arkprocess.h text.h)

OSIRIS_WRAP_HEADER(MOC_FILES ${MIARK_HEADERS})

set(MIARK_MOC_FILES
	adddlg.moc arch.moc extractdlg.moc arkwidget.moc)

add_executable(miark ${MIARK_SRCS} ${MOC_FILES})
target_link_libraries(miark
    ${OSIRIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KHTMLW}
    ${KDE1_JSCRIPT}
    ${KDE1_KFM}
    ${KDE1_KIMGIO}
    ${KDE1_KFILE}
    )

install(TARGETS miark RUNTIME DESTINATION ${KDE1_BINDIR})



install(FILES kwrite.xpm undo.xpm redo.xpm DESTINATION ${KDE1_ICONDIR})

install(FILES mini-ark.xpm DESTINATION ${KDE1_ICONDIR}/mini)

install(FILES large-ark.xpm DESTINATION ${KDE1_ICONDIR}/large)

install(FILES ark.kdelnk DESTINATION ${KDE1_APPSDIR}/Utilities)

install_icon(ark.xpm "")
