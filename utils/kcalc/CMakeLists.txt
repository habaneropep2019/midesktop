include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

# TODO
#add_subdirectory(docs)


set(KCALC_SRCS
	kcalc.cpp kcalc_core.cpp configdlg.cpp fontdlg.cpp
	dlabel.cpp stats.cpp)

set(KCALC_HEADERS
	kcalc.h configdlg.h fontdlg.h dlabel.h)

OSIRIS_WRAP_HEADER(MOC_FILES ${KCALC_HEADERS})

set(KCALC_MOC_FILES
	kcalc.moc configdlg.moc fontdlg.moc dlabel.moc)

add_executable(kcalc ${KCALC_SRCS} ${MOC_FILES})
target_link_libraries(kcalc
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

install(TARGETS kcalc RUNTIME DESTINATION ${KDE1_BINDIR})

#install(FILES directory.trash directory.autostart directory.templates directory.desktop DESTINATION ${KDE1_DATADIR}/kfm)


#install(FILES config/kfmrc config/desktop DESTINATION ${KDE1_CONFDIR})
#install(FILES magic DESTINATION ${KDE1_MIMEDIR})

#install_icon(kfm.xpm "")
#install_icon(link.xpm "")
