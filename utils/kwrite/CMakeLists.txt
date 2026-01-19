include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

# TODO
#add_subdirectory(docs)


set(KWRITE_SRCS
	highlight.cpp kwdoc.cpp kwview.cpp
	kwdialog.cpp toplevel.cpp kmimemagic.cpp)

set(KWRITE_MOC
	kfmgui.h kfmview.h kbind.h
	kfmdlg.h bookmark.h kfmprops.h kfmserver_ipc.h
	kfmserver.h kioserver.h kioserver_ipc.h
	root.h kfmman.h kiojob.h
	htmlcache.h kfmtree.h
	krenamewin.h passworddialog.h kURLcompletion.h kfmw.h
	kfmjob.h kfmexec.h kfinder.h
	open-with.h finddlg.h kcookiewin.h popup.h)

OSIRIS_WRAP_HEADER(MOC_FILES ${KWRITE_MOC})

add_executable(kwrite ${KWRITE_SRCS} ${KWRITE_FILES})
target_link_libraries(kwrite
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

install(TARGETS kwrite RUNTIME DESTINATION ${KDE1_BINDIR})

#install(FILES directory.trash directory.autostart directory.templates directory.desktop DESTINATION ${KDE1_DATADIR}/kfm)


#install(FILES config/kfmrc config/desktop DESTINATION ${KDE1_CONFDIR})
#install(FILES magic DESTINATION ${KDE1_MIMEDIR})

#install_icon(kfm.xpm "")
#install_icon(link.xpm "")
