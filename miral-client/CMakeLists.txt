include_directories(
    ../miral-shell/spinner
    ${CMAKE_CURRENT_BINARY_DIR}/../miral-shell/spinner
    ${GLIB_INCLUDE_DIRS}
    ${GLESv2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)


add_executable(miral-client
    client.cpp
)

target_link_libraries(miral-client
    miral-spinner
)
