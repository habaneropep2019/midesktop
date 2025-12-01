include_directories(
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(KEYS_SRCS
main.cpp keyconfig.cpp savescm.cpp
)

set(KEYS_MOC
keyconfig.moc savescm.moc  
)

add_executable(kcmkeys ${KEYS_SRCS} ${MOC_FILES})
target_link_libraries(kcmkeys
    ${OSIRIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS kcmkeys RUNTIME DESTINATION ${KDE1_BINDIR})
