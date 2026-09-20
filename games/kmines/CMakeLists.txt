include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

# TODO
#add_subdirectory(docs)


set(KMINES_SRCS
    dialogs.cpp field.cpp status.cpp main.cpp
)

set(KMINES_MOC
    dialogs.moc field.moc status.moc main.moc
)

OSIRIS_WRAP_MOC(MOC_FILES ${KMINES_MOC})

set(KHEXDIT_MOC_FILES
	hexwidget.moc hexfile.moc)

add_executable(kmines ${KMINES_SRCS} ${MOC_FILES})
target_link_libraries(kmines
    ${OSIRIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
)

install(TARGETS kmines RUNTIME DESTINATION ${KDE1_BINDIR})



install(FILES data/mini-kmines.xpm DESTINATION ${KDE1_ICONDIR}/mini)

install(FILES data/large-kmines.xpm DESTINATION ${KDE1_ICONDIR}/large)

install(FILES data/kmines.kdelnk DESTINATION ${KDE1_APPSDIR}/Games)

install_icon(kmines.xpm "")
