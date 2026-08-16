add_subdirectory(pics)

include_directories(
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(MIPANEL_SRCS  mipanel.C props.C layout.C tools.C main.C events.C pmenu.C pfilemenu.C)

set(MIPANEL_MOC mipanel.moc pmenu.moc)

OSIRIS_WRAP_HEADER(MOC_FILES ${MIPANEL_MOC})

add_executable(mipanel ${MIPANEL_SRCS} ${MOC_FILES})
target_link_libraries(mipanel
    ${OSIRIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KFM}
    )

install(TARGETS mipanel RUNTIME DESTINATION ${KDE1_BINDIR})

install(FILES personal.kdelnk DESTINATION ${KDE1_DATADIR}/mipanel/default RENAME personal_directory)
install(FILES Home.kdelnk Desktop.kdelnk DESTINATION ${KDE1_DATADIR}/mipanel/default)
