include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(KROOTWM_SRCS krootwm.C)

set(KROOTWM_MOC krootwm.moc)

add_executable(krootwm ${KROOTWM_SRCS} ${MOC_FILES})
target_link_libraries(krootwm
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KFM}
    )

install(TARGETS krootwm RUNTIME DESTINATION ${KDE1_BINDIR})
