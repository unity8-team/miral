find_package(GtestGmock REQUIRED)
pkg_check_modules(MIRTEST REQUIRED mirtest)
include_directories(
        ${MIRTEST_INCLUDE_DIRS}
        ${GMOCK_INCLUDE_DIR}
        ${GTEST_INCLUDE_DIR}
)

add_executable(miral-test
    mru_window_list.cpp
)

message(STATUS "MIRTEST_LDFLAGS=${MIRTEST_LDFLAGS}")
message(STATUS "MIRTEST_STATIC_LDFLAGS=${MIRTEST_STATIC_LDFLAGS}")

target_link_libraries(miral-test
    ${MIRTEST_LDFLAGS}
    ${GTEST_BOTH_LIBRARIES}
    ${GMOCK_LIBRARY}
    ${GMOCK_MAIN_LIBRARY}
    miral
)
