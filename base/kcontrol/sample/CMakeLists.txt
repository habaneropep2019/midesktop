include_directories(
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(SAMPLE_SRCS
    main.cpp bell.cpp kslidercontrol.cpp
)

set(SAMPLE_MOC
    bell.moc kslidercontrol.moc
)

OSIRIS_WRAP_HEADER(MOC_FILES ${SAMPLE_MOC})

add_executable(kcmbell ${SAMPLE_SRCS} ${MOC_FILES})
target_link_libraries(kcmbell
    ${OSIRIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS kcmbell RUNTIME DESTINATION ${KDE1_BINDIR})

install_icon(sample.xpm "")
