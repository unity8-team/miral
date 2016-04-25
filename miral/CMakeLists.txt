pkg_check_modules(MIRSERVER mirserver>=0.20 REQUIRED)

include_directories(include SYSTEM ${MIRSERVER_INCLUDE_DIRS})

add_library(miral SHARED
    application.cpp                   ${CMAKE_SOURCE_DIR}/include/miral/application.h
    runner.cpp                        ${CMAKE_SOURCE_DIR}/include/miral/runner.h
    display_configuration_option.cpp  ${CMAKE_SOURCE_DIR}/include/miral/display_configuration_option.h
    quit_on_ctrl_alt_bksp.cpp         ${CMAKE_SOURCE_DIR}/include/miral/quit_on_ctrl_alt_bksp.h
    window.cpp                        ${CMAKE_SOURCE_DIR}/include/miral/window.h
    window_info.cpp                   ${CMAKE_SOURCE_DIR}/include/miral/window_info.h
    window_management_options.cpp     ${CMAKE_SOURCE_DIR}/include/miral/window_management_options.h
    window_specification.cpp          ${CMAKE_SOURCE_DIR}/include/miral/window_specification.h
    startup_internal_client.cpp       ${CMAKE_SOURCE_DIR}/include/miral/startup_internal_client.h
    basic_window_manager.cpp          basic_window_manager.h
    ${CMAKE_SOURCE_DIR}/include/miral/application_info.h
    ${CMAKE_SOURCE_DIR}/include/miral/window_management_policy.h
    ${CMAKE_SOURCE_DIR}/include/miral/window_manager_tools.h
    ${CMAKE_SOURCE_DIR}/include/miral/stream_specification.h
)

target_link_libraries(miral
    PUBLIC
        ${MIRCLIENT_LDFLAGS}
        ${MIRCOMMON_LDFLAGS}
    PRIVATE
        ${MIRSERVER_LDFLAGS}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/miral.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/miral.pc
    @ONLY
)

install(TARGETS     miral                                   DESTINATION "lib")
install(DIRECTORY   ${CMAKE_SOURCE_DIR}/include/            DESTINATION "include/mircommon")
install(FILES       ${CMAKE_CURRENT_BINARY_DIR}/miral.pc    DESTINATION "lib/pkgconfig")
