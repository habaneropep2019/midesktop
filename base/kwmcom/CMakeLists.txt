include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(KWMCCOM_SRCS kwmcom.C)

add_executable(kwmcom ${KWMCCOM_SRCS} ${MOC_FILES})
target_link_libraries(kwmcom
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    )

install(TARGETS kwmcom RUNTIME DESTINATION ${KDE1_BINDIR})
