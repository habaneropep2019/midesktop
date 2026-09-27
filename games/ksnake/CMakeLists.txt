include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

# TODO
#add_subdirectory(docs)


set(KSNAKE_SRCS
	game.cpp trys.cpp rattler.cpp board.cpp
    level.cpp ball.cpp snake.cpp
    basket.cpp startroom.cpp lcdrange.cpp
    score.cpp pixServer.cpp progress.cpp levels.cpp
    view.cpp keys.cpp)

set(KSNAKE_HEADERS
	game.h trys.h rattler.h snake.h
    basket.h startroom.h lcdrange.h score.h
    progress.h view.h keys.h)

OSIRIS_WRAP_HEADER(MOC_FILES ${KSNAKE_HEADERS})

set(KSNAKE_MOC_FILES
	game.moc trys.moc rattler.moc snake.moc
    basket.moc startroom.moc lcdrange.moc score.moc progress.moc levels.moc view.moc keys.moc)

add_executable(ksnake ${KSNAKE_SRCS} ${MOC_FILES})
target_link_libraries(ksnake
    ${OSIRIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS ksnake RUNTIME DESTINATION ${KDE1_BINDIR})



install(FILES mini-ksnake.xpm DESTINATION ${KDE1_ICONDIR}/mini)

install(FILES large-ksnake.xpm DESTINATION ${KDE1_ICONDIR}/large)

install(FILES ksnake.kdelnk DESTINATION ${KDE1_APPSDIR}/Games)

install (DIRECTORY data/levels DESTINATION ${KDE1_DATADIR}/ksnake/)

install (DIRECTORY data/backgrounds DESTINATION ${KDE1_DATADIR}/ksnake/)

install (DIRECTORY data/pixmaps/ DESTINATION ${KDE1_DATADIR}/ksnake/pics FILES_MATCHING PATTERN "*.xpm")

install_icon(ksnake.xpm "")
