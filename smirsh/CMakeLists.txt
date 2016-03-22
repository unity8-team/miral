add_executable(smirsh
    canonical_window_manager.cpp
    canonical_window_manager.h
    input_event_filter.cpp
    input_event_filter.h
    tiling_window_manager.cpp
    tiling_window_manager.h
    window_management.cpp
    main.cpp
)

target_link_libraries(smirsh miral)
