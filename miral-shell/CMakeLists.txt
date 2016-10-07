add_subdirectory(spinner)
add_subdirectory(desktop)

add_custom_target(miral-run ALL
    cp ${CMAKE_CURRENT_SOURCE_DIR}/miral-run.sh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/miral-run
)

install(PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/miral-run
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

add_custom_target(miral-xrun ALL
    cp ${CMAKE_CURRENT_SOURCE_DIR}/miral-xrun.sh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/miral-xrun
)

install(PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/miral-xrun
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

add_executable(miral-shell
    shell_main.cpp
    tiling_window_manager.cpp   tiling_window_manager.h
    titlebar_window_manager.cpp titlebar_window_manager.h
    titlebar_provider.cpp       titlebar_provider.h
)

target_link_libraries(miral-shell
    miral-spinner
    miral
)

install(TARGETS miral-shell
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
