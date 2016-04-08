add_subdirectory(spinner)

add_executable(miral-shell
    main.cpp
    canonical_window_manager.cpp canonical_window_manager.h
    tiling_window_manager.cpp    tiling_window_manager.h
    canonical_window_management_policy_data.cpp canonical_window_management_policy_data.h
        spinner/splash.h
)

target_link_libraries(miral-shell miral miral-spinner)
