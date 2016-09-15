set(
  SCREEN_TEST_SOURCES
  screen_test.cpp
  ${MIRAL_QT_SOURCE_DIR}/src/common/debughelpers.cpp
)

include_directories(
  ${MIRAL_QT_SOURCE_DIR}/tests/framework
  ${MIRAL_QT_SOURCE_DIR}/src/platforms/mirserver
  ${MIRAL_QT_SOURCE_DIR}/src/common
)

include_directories(
  SYSTEM
  ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
  ${MIRSERVER_INCLUDE_DIRS}
)

add_executable(ScreenTest ${SCREEN_TEST_SOURCES})

target_link_libraries(
  ScreenTest
  qpa-mirserver

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(Screen, ScreenTest)
