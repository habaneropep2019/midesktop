include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )


set(KNOTES_SRCS
	knotes.cpp  configdlg.cpp fontdlg.cpp mail.cpp alarm.cpp
		bwdatetime.cpp spin.cpp timer.cpp savealarm.cpp renamedlg.cpp
		docking.cpp )

set(KNOTES_HEADERS
	knotes.h configdlg.h fontdlg.h mail.h alarm.h
		spin.h renamedlg.h docking.h)

OSIRIS_WRAP_HEADER(MOC_FILES ${KNOTES_HEADERS})

set(KNOTES_MOC_FILES
	knotes.moc configdlg.moc fontdlg.moc mail.moc alarm.moc
	spin.moc renamedlg.moc docking.moc)

add_executable(knotes ${KNOTES_SRCS} ${MOC_FILES})
target_link_libraries(knotes
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

install(TARGETS knotes RUNTIME DESTINATION ${KDE1_BINDIR})

#install(FILES directory.trash directory.autostart directory.templates directory.desktop DESTINATION ${KDE1_DATADIR}/kfm)


install(FILES knoteslogo.xpm knotesclose.xpm DESTINATION ${KDE1_DATADIR}/knotes/pics/)

install(FILES knotes.kdelnk DESTINATION ${KDE1_APPSDIR}/Utilities)

install_icon(knotes.xpm "")
