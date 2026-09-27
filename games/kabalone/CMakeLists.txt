include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

# TODO
#add_subdirectory(docs)


set(KABALONE_SRCS AbTop.cpp Board.cpp BoardWidget.cpp Move.cpp Spy.cpp kabalone.cpp)

set(KABALONE_MOC AbTop.moc Board.moc BoardWidget.moc)

OSIRIS_WRAP_MOC(MOC_FILES ${KABALONE_MOC})

add_executable(kabalone ${KABALONE_SRCS} ${MOC_FILES})
target_link_libraries(kabalone
    ${OSIRIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
)

install(TARGETS kabalone RUNTIME DESTINATION ${KDE1_BINDIR})



install(FILES mini-kabalone.xpm DESTINATION ${KDE1_ICONDIR}/mini)

install(FILES large-kabalone.xpm DESTINATION ${KDE1_ICONDIR}/large)

install(FILES kabalone.kdelnk DESTINATION ${KDE1_APPSDIR}/Games)

install_icon(kabalone.xpm "")
