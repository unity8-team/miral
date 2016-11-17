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

add_custom_target(miral-screencast ALL
    cp ${CMAKE_CURRENT_SOURCE_DIR}/miral-screencast.sh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/miral-screencast
)

install(PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/miral-screencast
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

add_custom_target(miral-desktop ALL
    cp ${CMAKE_CURRENT_SOURCE_DIR}/miral-desktop.sh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/miral-desktop
)

install(PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/miral-desktop
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

add_executable(miral-shell
    shell_main.cpp
    tiling_window_manager.cpp   tiling_window_manager.h
    titlebar_window_manager.cpp titlebar_window_manager.h
    titlebar_provider.cpp       titlebar_provider.h
    titlebar_config.cpp         titlebar_config.h
)

pkg_check_modules(FREETYPE freetype2 REQUIRED)
target_include_directories(miral-shell PUBLIC ${FREETYPE_INCLUDE_DIRS})
target_compile_definitions(miral-shell PUBLIC -DTYPO_SUPPORTS_FREETYPE)
target_link_libraries(miral-shell ${FREETYPE_LIBRARIES})

target_link_libraries(miral-shell
    miral-spinner
    miral
)

install(TARGETS miral-shell
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
