include_directories(
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(KBGNWM_SRCS main.cpp kbgndwm.cpp bg.cpp)

set(KBGNWM_MOC kbgndwm.moc bg.moc)

add_executable(kbgndwm ${KBGNWM_SRCS} ${MOC_FILES})
target_link_libraries(kbgndwm
    ${OSIRIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KIMGIO}
    )

install(TARGETS kbgndwm RUNTIME DESTINATION ${KDE1_BINDIR})
