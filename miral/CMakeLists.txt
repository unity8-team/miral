pkg_check_modules(MIRSERVER mirserver>=0.20 REQUIRED)

include_directories(include SYSTEM ${MIRSERVER_INCLUDE_DIRS})

add_library(miral SHARED
    surface_info.cpp                  ${CMAKE_SOURCE_DIR}/include/miral/surface_info.h
    surface.cpp                       ${CMAKE_SOURCE_DIR}/include/miral/surface.h
    session.cpp                       ${CMAKE_SOURCE_DIR}/include/miral/session.h
    runner.cpp                        ${CMAKE_SOURCE_DIR}/include/miral/runner.h
    display_configuration_option.cpp  ${CMAKE_SOURCE_DIR}/include/miral/display_configuration_option.h
    quit_on_ctrl_alt_bksp.cpp         ${CMAKE_SOURCE_DIR}/include/miral/quit_on_ctrl_alt_bksp.h
    window_management_options.cpp     ${CMAKE_SOURCE_DIR}/include/miral/window_management_options.h
    internal_client.cpp               ${CMAKE_SOURCE_DIR}/include/miral/internal_client.h
    basic_window_manager.cpp          basic_window_manager.h
    ${CMAKE_SOURCE_DIR}/include/miral/session_info.h
    ${CMAKE_SOURCE_DIR}/include/miral/window_management_policy.h
    ${CMAKE_SOURCE_DIR}/include/miral/window_manager_tools.h
)

target_link_libraries(miral
    PUBLIC
        ${MIRCLIENT_LDFLAGS}
#    PRIVATE TODO complete wrapping of libmirserver
        ${MIRSERVER_LDFLAGS}
)