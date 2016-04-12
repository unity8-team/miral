add_subdirectory(spinner)
add_subdirectory(desktop)

add_custom_target(miral-run ALL
    cp ${CMAKE_CURRENT_SOURCE_DIR}/miral-run.sh ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/miral-run
)

# TODO remove dependency on libmirserver-dev
pkg_check_modules(MIRSERVER mirserver>=0.20 REQUIRED)
include_directories(include SYSTEM ${MIRSERVER_INCLUDE_DIRS})

add_executable(miral-shell
    main.cpp
    canonical_window_manager.cpp canonical_window_manager.h
    tiling_window_manager.cpp    tiling_window_manager.h
    canonical_window_management_policy_data.cpp canonical_window_management_policy_data.h
)

target_link_libraries(miral-shell miral miral-spinner)

install(TARGETS miral-shell
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
