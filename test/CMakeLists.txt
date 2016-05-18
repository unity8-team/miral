find_package(GtestGmock REQUIRED)
include_directories(${GMOCK_INCLUDE_DIR} ${GTEST_INCLUDE_DIR})

add_executable(miral-test
    mru_window_list.cpp
)

target_link_libraries(miral-test
    miral
    ${GTEST_BOTH_LIBRARIES}
    ${GMOCK_LIBRARY}
    ${GMOCK_MAIN_LIBRARY}
)
