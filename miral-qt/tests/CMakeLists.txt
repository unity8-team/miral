# if (NOT GMOCK_FOUND) doesn't work, dunno why - alan_g
if (NOT GMOCK_LIBRARIES)
    find_package(GMock)
endif()

pkg_check_modules(MIRTEST mirtest>=0.24 REQUIRED)

unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY) #stop miral copying built tests into bin dir

add_subdirectory(framework)
add_subdirectory(mirserver)
add_subdirectory(modules)

add_test(
    NAME convert-to-gles
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/convert-to-gles.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
