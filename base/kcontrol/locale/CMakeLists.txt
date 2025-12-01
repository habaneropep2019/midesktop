add_subdirectory(pics)

include_directories(
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(LOCALE_SRCS
main.cpp locale.cpp klangcombo.cpp
)

set(LOCALE_MOC
locale.moc klangcombo.moc
)

OSIRIS_WRAP_HEADER(MOC_FILES ${LOCALE_MOC})

add_executable(kcmlocale ${LOCALE_SRCS} ${MOC_FILES})
target_link_libraries(kcmlocale
    ${OSIRIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS kcmlocale RUNTIME DESTINATION ${KDE1_BINDIR})

install_icon(locale.xpm "")
