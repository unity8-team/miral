add_executable(smirsh
    canonical_window_manager.cpp
    canonical_window_manager.h
    tiling_window_manager.cpp
    tiling_window_manager.h
    window_management.cpp
    main.cpp
)

target_link_libraries(smirsh miral)
