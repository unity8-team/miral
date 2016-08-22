if (MIRTEST_VERSION VERSION_LESS 0.25)
    # Mir used mix struct and class in public test headers (fixed -r 3606 in lp:mir)
    check_cxx_compiler_flag(-Wno-mismatched-tags MIRAL_COMPILE_WITH_W_NO_MISMATCHED_TAGS)
    if (MIRAL_COMPILE_WITH_W_NO_MISMATCHED_TAGS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-mismatched-tags")
    endif()
endif()
check_cxx_compiler_flag(-Wno-return-type-c-linkage MIRAL_COMPILE_WITH_W_RETURN_TYPE_C_LINKAGE)
if (MIRAL_COMPILE_WITH_W_RETURN_TYPE_C_LINKAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type-c-linkage")
endif()

include_directories(
        ${MIRTEST_INCLUDE_DIRS}
        ${GMOCK_INCLUDE_DIR}
        ${GTEST_INCLUDE_DIR}
)

add_executable(miral-test
    mru_window_list.cpp
    active_outputs.cpp
    persistent_surface_store.cpp
    runner.cpp
    test_server.cpp         test_server.h
)

target_link_libraries(miral-test
    ${MIRTEST_LDFLAGS}
    ${GTEST_BOTH_LIBRARIES}
    ${GMOCK_LIBRARIES}
    miral
    miral-internal
)

add_dependencies(miral-test
    ${GTEST_BOTH_LIBRARIES}
    ${GMOCK_LIBRARIES}
)

add_test(NAME miral-test WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COMMAND miral-test)
