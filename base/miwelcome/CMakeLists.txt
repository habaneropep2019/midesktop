include_directories(
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    ${CURSES_INCLUDE_DIRS}
    )

set(MIWELCOME_SRCS main.cpp)

add_executable(miwelcome ${MIWELCOME_SRCS} ${MOC_FILES})
target_link_libraries(miwelcome
    ${OSIRIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS miwelcome RUNTIME DESTINATION ${KDE1_BINDIR})

