set(
  SCREENCONTROLLER_TEST_SOURCES
  screensmodel_test.cpp
  ${MIRAL_QT_SOURCE_DIR}/src/common/debughelpers.cpp
  # to be moc-ed
  stub_screen.h
  testable_screensmodel.h
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
  ${MIRRENDERERGLDEV_INCLUDE_DIRS}
)

add_executable(ScreensModelTest ${SCREENCONTROLLER_TEST_SOURCES})

target_link_libraries(
  ScreensModelTest
  qpa-mirserver

  -L${CMAKE_BINARY_DIR}/miral-qt/tests/framework
  qtmir-test-framework-static

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(ScreensModel, ScreensModelTest)
