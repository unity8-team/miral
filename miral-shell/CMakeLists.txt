add_subdirectory(spinner)

add_executable(miral-shell
    main.cpp
    canonical_window_manager.cpp canonical_window_manager.h
    tiling_window_manager.cpp    tiling_window_manager.h
    canonical_window_management_policy_data.cpp canonical_window_management_policy_data.h
)

target_link_libraries(miral-shell miral miral-spinner)

add_executable(miral-gnome-terminal
    gnome_terminal.cpp
    canonical_window_manager.cpp canonical_window_manager.h
    tiling_window_manager.cpp    tiling_window_manager.h
    canonical_window_management_policy_data.cpp canonical_window_management_policy_data.h
)

target_link_libraries(miral-gnome-terminal miral)
