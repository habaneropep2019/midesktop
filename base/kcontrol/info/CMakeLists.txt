add_subdirectory(pics)

include_directories(
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(INFO_SRCS
    main.cpp memory.cpp
)

set(INFO_MOC
    memory.moc info.moc
)

add_executable(kcminfo ${INFO_SRCS} ${MOC_FILES})
target_link_libraries(kcminfo
    ${OSIRIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS kcminfo RUNTIME DESTINATION ${KDE1_BINDIR})
