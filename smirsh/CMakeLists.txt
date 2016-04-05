add_executable(smirsh
    main.cpp
    canonical_window_manager.cpp canonical_window_manager.h
    tiling_window_manager.cpp    tiling_window_manager.h
    canonical_window_management_policy_data.cpp canonical_window_management_policy_data.h
)

target_link_libraries(smirsh miral)
