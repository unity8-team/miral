pkg_check_modules(MIRSERVER mirserver>=0.20 REQUIRED)

if(${CMAKE_COMPILER_IS_GNUCXX})
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -flto")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto")
    set(CMAKE_AR "gcc-ar")
    set(CMAKE_NM "gcc-nm")
    set(CMAKE_RANLIB "gcc-ranlib")
endif()

set(MIRAL_ABI 2)
set(symbol_map ${CMAKE_CURRENT_SOURCE_DIR}/symbols.map)

include_directories(include SYSTEM ${MIRSERVER_INCLUDE_DIRS})

add_library(miral-internal STATIC
    basic_window_manager.cpp            basic_window_manager.h window_manager_tools_implementation.h
    coordinate_translator.cpp           coordinate_translator.h
    mru_window_list.cpp                 mru_window_list.h
    window_management_trace.cpp         window_management_trace.h
    xcursor_loader.cpp                  xcursor_loader.h
    xcursor.c                           xcursor.h
    both_versions.h
)

set_source_files_properties(xcursor.c PROPERTIES COMPILE_DEFINITIONS _GNU_SOURCE)

add_library(miral SHARED
    active_outputs.cpp                  ${CMAKE_SOURCE_DIR}/include/miral/active_outputs.h
    add_init_callback.cpp               ${CMAKE_SOURCE_DIR}/include/miral/add_init_callback.h
    application.cpp                     ${CMAKE_SOURCE_DIR}/include/miral/application.h
    application_authorizer.cpp          ${CMAKE_SOURCE_DIR}/include/miral/application_authorizer.h
    application_info.cpp                ${CMAKE_SOURCE_DIR}/include/miral/application_info.h
    canonical_window_manager.cpp        ${CMAKE_SOURCE_DIR}/include/miral/canonical_window_manager.h
    command_line_option.cpp             ${CMAKE_SOURCE_DIR}/include/miral/command_line_option.h
    cursor_theme.cpp                    ${CMAKE_SOURCE_DIR}/include/miral/cursor_theme.h
    debug_extension.cpp                 ${CMAKE_SOURCE_DIR}/include/miral/debug_extension.h
    keymap.cpp                          ${CMAKE_SOURCE_DIR}/include/miral/keymap.h
    runner.cpp                          ${CMAKE_SOURCE_DIR}/include/miral/runner.h
    display_configuration_option.cpp    ${CMAKE_SOURCE_DIR}/include/miral/display_configuration_option.h
    output.cpp                          ${CMAKE_SOURCE_DIR}/include/miral/output.h
    append_event_filter.cpp             ${CMAKE_SOURCE_DIR}/include/miral/append_event_filter.h
    window.cpp                          ${CMAKE_SOURCE_DIR}/include/miral/window.h
    window_info.cpp                     ${CMAKE_SOURCE_DIR}/include/miral/window_info.h
    window_management_options.cpp       ${CMAKE_SOURCE_DIR}/include/miral/window_management_options.h
    window_specification.cpp            ${CMAKE_SOURCE_DIR}/include/miral/window_specification.h
    internal_client.cpp                 ${CMAKE_SOURCE_DIR}/include/miral/internal_client.h
    set_command_line_hander.cpp         ${CMAKE_SOURCE_DIR}/include/miral/set_command_line_hander.h
    set_terminator.cpp                  ${CMAKE_SOURCE_DIR}/include/miral/set_terminator.h
    set_window_managment_policy.cpp     ${CMAKE_SOURCE_DIR}/include/miral/set_window_managment_policy.h
    window_management_policy.cpp        ${CMAKE_SOURCE_DIR}/include/miral/window_management_policy.h
    window_manager_tools.cpp            ${CMAKE_SOURCE_DIR}/include/miral/window_manager_tools.h
                                        ${CMAKE_SOURCE_DIR}/include/miral/workspace_policy.h
                                        ${CMAKE_SOURCE_DIR}/include/mir/client/window_spec.h
                                        ${CMAKE_SOURCE_DIR}/include/mir/client/window_id.h
                                        ${CMAKE_SOURCE_DIR}/include/mir/client/connection.h
                                        ${CMAKE_SOURCE_DIR}/include/mir/client/window.h
                                        ${CMAKE_SOURCE_DIR}/include/mir/client/detail/mir_forward_compatibility.h
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
        LINK_DEPENDS ${symbol_map}
)

add_custom_target(check-symbols ALL
        DEPENDS miral ${PROJECT_SOURCE_DIR}/debian/libmiral${MIRAL_ABI}.symbols
        COMMAND dpkg-gensymbols -e${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libmiral.so.${MIRAL_ABI} -plibmiral${MIRAL_ABI} | scripts/filter_symbols_diff.sh
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        VERBATIM
)

set(LIBDIR "${CMAKE_INSTALL_FULL_LIBDIR}")
set(INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include/miral")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/miral.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/miral.pc
    @ONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mirclientcpp.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/mirclientcpp.pc
    @ONLY
)

if (MIRSERVER_VERSION VERSION_LESS 0.24)
    set(MIR_POINTER_CONFINEMENT 0)
else()
    set(MIR_POINTER_CONFINEMENT 1)
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/mir_features.h.in
        ${PROJECT_SOURCE_DIR}/include/mir/client/detail/mir_features.h
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
  ${PROJECT_SOURCE_DIR}/include/miral/version.h
)

install(TARGETS     miral                           LIBRARY         DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}")
install(DIRECTORY   ${CMAKE_SOURCE_DIR}/include/                    DESTINATION "${INCLUDEDIR}")
install(FILES       ${CMAKE_CURRENT_BINARY_DIR}/miral.pc
                    ${CMAKE_CURRENT_BINARY_DIR}/mirclientcpp.pc    DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")
