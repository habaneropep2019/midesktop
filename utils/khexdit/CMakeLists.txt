include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

# TODO
#add_subdirectory(docs)


set(KHEXDIT_SRCS
	hexdata.cpp hexfile.cpp hexwidget.cpp)

set(KHEXDIT_HEADERS
	hexfile.h hexwidget.h)

OSIRIS_WRAP_HEADER(MOC_FILES ${KHEXDIT_HEADERS})

set(KHEXDIT_MOC_FILES
	hexwidget.moc hexfile.moc)

add_executable(khexdit ${KHEXDIT_SRCS} ${MOC_FILES})
target_link_libraries(khexdit
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

install(TARGETS khexdit RUNTIME DESTINATION ${KDE1_BINDIR})



install(FILES mini-khexdit.xpm DESTINATION ${KDE1_ICONDIR}/mini)

install(FILES large-khexdit.xpm DESTINATION ${KDE1_ICONDIR}/large)

install(FILES khexdit.kdelnk DESTINATION ${KDE1_APPSDIR}/Utilities)

install_icon(khexdit.xpm "")
