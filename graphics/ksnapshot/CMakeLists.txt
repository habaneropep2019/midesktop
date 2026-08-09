include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
)

set(ksnapshot_SRCS
    eps.cpp
    gif.cpp
    main.cpp
    formats.cpp
    ksnapshot.cpp
    preview.cpp
)

set(ksnapshot_MOC
    ksnapshot.moc
    preview.moc
)

OSIRIS_WRAP_HEADER(MOC_FILES ${ksnapshot_MOC})

add_executable(ksnapshot1 ${ksnapshot_SRCS} ${MOC_FILES})

target_compile_definitions(ksnapshot1 PRIVATE KSNAPVERSION=\"0.2.7\")
target_link_libraries(ksnapshot1
    ${OSIRIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KIMGIO}
)

install(TARGETS ksnapshot1 RUNTIME DESTINATION ${KDE1_BINDIR})
install(FILES ksnapshot.kdelnk DESTINATION ${KDE1_APPSDIR}/Graphics)
install_icon(ksnapshot.xpm)

install(FILES
    doc/cs/index.html
    doc/cs/ksnapshot.gif
    DESTINATION ${KDE1_HTMLDIR}/cs/ksnapshot)

install(FILES
    doc/en/index.html
    doc/en/ksnapshot.gif
    DESTINATION ${KDE1_HTMLDIR}/en/ksnapshot)
