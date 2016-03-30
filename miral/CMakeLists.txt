add_library(miral SHARED
        basic_window_manager.cpp  window_management_info.cpp
        ${CMAKE_SOURCE_DIR}/include/mir/al/basic_window_manager.h
        ${CMAKE_SOURCE_DIR}/include/mir/al/window_management_info.h
        surface.cpp ${CMAKE_SOURCE_DIR}/include/mir/al/surface.h
        runner.cpp ${CMAKE_SOURCE_DIR}/include/mir/al/runner.h
        input_event_filter.cpp input_event_filter.h
        )

target_link_libraries(miral
        ${MIRSERVER_LDFLAGS}
        ${MIRCLIENT_LDFLAGS}
)