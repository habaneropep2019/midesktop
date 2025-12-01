include_directories(
    ${OSIRIS_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(KFM_SRCS
main.cpp htmlopts.cpp miscopts.cpp  kproxydlg.cpp khttpoptdlg.cpp useragentdlg.cpp kcookiesdlg.cpp rootopts.cpp
)

set(KFM_MOC
htmlopts.moc miscopts.moc  kproxydlg.moc khttpoptdlg.moc useragentdlg.moc kcookiesdlg.moc rootopts.moc
)

add_executable(kcmkfm ${KFM_SRCS} ${MOC_FILES})
target_link_libraries(kcmkfm
    ${OSIRIS_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS kcmkfm RUNTIME DESTINATION ${KDE1_BINDIR})
