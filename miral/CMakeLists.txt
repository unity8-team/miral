pkg_check_modules(MIRSERVER mirserver>=0.20 REQUIRED)

set(MIRAL_ABI 1)
set(symbol_map ${CMAKE_CURRENT_SOURCE_DIR}/symbols.map)

include_directories(include SYSTEM ${MIRSERVER_INCLUDE_DIRS})

add_library(miral-internal STATIC
    basic_window_manager.cpp            basic_window_manager.h window_manager_tools_implementation.h
    mru_window_list.cpp                 mru_window_list.h
)

add_library(miral SHARED
    active_outputs.cpp                  ${CMAKE_SOURCE_DIR}/include/miral/active_outputs.h
    application.cpp                     ${CMAKE_SOURCE_DIR}/include/miral/application.h
    application_authorizer.cpp          ${CMAKE_SOURCE_DIR}/include/miral/application_authorizer.h
    application_info.cpp                ${CMAKE_SOURCE_DIR}/include/miral/application_info.h
    canonical_window_manager.cpp        ${CMAKE_SOURCE_DIR}/include/miral/canonical_window_manager.h
    runner.cpp                          ${CMAKE_SOURCE_DIR}/include/miral/runner.h
    display_configuration_option.cpp    ${CMAKE_SOURCE_DIR}/include/miral/display_configuration_option.h
    output.cpp                          ${CMAKE_SOURCE_DIR}/include/miral/output.h
    quit_on_ctrl_alt_bksp.cpp           ${CMAKE_SOURCE_DIR}/include/miral/quit_on_ctrl_alt_bksp.h
    window.cpp                          ${CMAKE_SOURCE_DIR}/include/miral/window.h
    window_info.cpp                     ${CMAKE_SOURCE_DIR}/include/miral/window_info.h
    window_management_options.cpp       ${CMAKE_SOURCE_DIR}/include/miral/window_management_options.h
    window_specification.cpp            ${CMAKE_SOURCE_DIR}/include/miral/window_specification.h
    internal_client.cpp                 ${CMAKE_SOURCE_DIR}/include/miral/internal_client.h
    set_window_managment_policy.cpp     ${CMAKE_SOURCE_DIR}/include/miral/set_window_managment_policy.h
    window_management_policy.cpp        ${CMAKE_SOURCE_DIR}/include/miral/window_management_policy.h
    window_manager_tools.cpp            ${CMAKE_SOURCE_DIR}/include/miral/window_manager_tools.h
                                        ${CMAKE_SOURCE_DIR}/include/miral/stream_specification.h
                                        ${CMAKE_SOURCE_DIR}/include/miral/toolkit/surface_spec.h
                                        ${CMAKE_SOURCE_DIR}/include/miral/toolkit/connection.h
)

target_link_libraries(miral
    PUBLIC
        ${MIRCLIENT_LDFLAGS}
        ${MIRCOMMON_LDFLAGS}
    PRIVATE
        miral-internal
        ${MIRSERVER_LDFLAGS}
)

set_target_properties(miral
        PROPERTIES
        SOVERSION ${MIRAL_ABI}
        LINK_FLAGS "-Wl,--exclude-libs=ALL -Wl,--version-script,${symbol_map}"
)

set(LIBDIR "${CMAKE_INSTALL_PREFIX}/lib")
set(INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include/miral")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/miral.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/miral.pc
    @ONLY
)

install(TARGETS     miral                           LIBRARY DESTINATION "${LIBDIR}")
install(DIRECTORY   ${CMAKE_SOURCE_DIR}/include/            DESTINATION "${INCLUDEDIR}")
install(FILES       ${CMAKE_CURRENT_BINARY_DIR}/miral.pc    DESTINATION "${LIBDIR}/pkgconfig")
