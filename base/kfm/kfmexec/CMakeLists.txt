include_directories(
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(KFMWARN_SRCS main.cpp)

OSIRIS_WRAP_MOC(MOC_FILES main.moc)

add_executable(kfmexec ${KFMWARN_SRCS} ${MOC_FILES})
target_link_libraries(kfmexec
    ${OSIRIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KFM}
    )

install(TARGETS kfmexec RUNTIME DESTINATION ${KDE1_BINDIR})

