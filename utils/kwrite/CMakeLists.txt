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

set(KWRITE_HEADERS
	highlight.h kwdoc.h kwview.h kwdialog.h toplevel.h kmimemagic.h)

OSIRIS_WRAP_HEADER(MOC_FILES ${KWRITE_HEADERS})

set(KWRITE_MOC_FILES
	highlight.moc kwdoc.moc kwview.moc kwdialog.moc toplevel.moc kmimemagic.moc)

add_executable(kwrite ${KWRITE_SRCS} ${MOC_FILES})
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
