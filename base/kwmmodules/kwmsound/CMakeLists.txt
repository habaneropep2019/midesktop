include_directories(
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(KWMSOUND_SRCS kwmsound.C)

set(KWMSOUND_MOC kwmsound.moc)

add_executable(kwmsound ${KWMSOUND_SRCS} ${MOC_FILES})
target_link_libraries(kwmsound
    ${OSIRIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_MEDIATOOL}
    )

install(TARGETS kwmsound RUNTIME DESTINATION ${KDE1_BINDIR})
