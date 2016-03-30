add_executable(smirsh
    main.cpp
    canonical_window_manager.cpp canonical_window_manager.h
    tiling_window_manager.cpp    tiling_window_manager.h
    window_management.cpp        window_management.h
    timeout_option.cpp           timeout_option.h
)

target_link_libraries(smirsh miral)
