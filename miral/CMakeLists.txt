add_library(miral SHARED
    basic_window_manager.cpp          ${CMAKE_SOURCE_DIR}/include/mir/al/basic_window_manager.h
    surface_info.cpp                  ${CMAKE_SOURCE_DIR}/include/miral/surface_info.h
    surface.cpp                       ${CMAKE_SOURCE_DIR}/include/miral/surface.h
    session.cpp                       ${CMAKE_SOURCE_DIR}/include/miral/session.h
    runner.cpp                        ${CMAKE_SOURCE_DIR}/include/miral/runner.h
    display_configuration_option.cpp  ${CMAKE_SOURCE_DIR}/include/miral/display_configuration_option.h
    input_event_filter.cpp            input_event_filter.h
    ${CMAKE_SOURCE_DIR}/include/miral/session_info.h
    ${CMAKE_SOURCE_DIR}/include/miral/window_management_policy.h
    ${CMAKE_SOURCE_DIR}/include/miral/window_manager_tools.h)

target_link_libraries(miral
    ${MIRSERVER_LDFLAGS}
    ${MIRCLIENT_LDFLAGS}
)