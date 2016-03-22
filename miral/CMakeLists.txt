add_library(miral SHARED
        basic_window_manager.cpp  window_management_info.cpp
        ${CMAKE_SOURCE_DIR}/include/mir/al/basic_window_manager.h
        ${CMAKE_SOURCE_DIR}/include/mir/al/window_management_info.h
        surface.cpp)

target_link_libraries(miral
        ${MIRSERVER_LDFLAGS}
        ${MIRCLIENT_LDFLAGS}
)