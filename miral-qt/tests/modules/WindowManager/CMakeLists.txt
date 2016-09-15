set(
  MIR_WINDOW_MANAGER_TEST_SOURCES
#  mirsurfaceitem_test.cpp #FIXME - reinstate these tests when functionality there
  mirsurface_test.cpp
  windowmodel_test.cpp
  ${MIRAL_QT_SOURCE_DIR}/src/common/debughelpers.cpp
)

include_directories(
  ${MIRAL_QT_SOURCE_DIR}/src/common
  ${MIRAL_QT_SOURCE_DIR}/src/modules
  ${MIRAL_QT_SOURCE_DIR}/src/platforms/mirserver
  ${MIRAL_QT_SOURCE_DIR}/tests/framework
  ${CMAKE_SOURCE_DIR}/include/miral   #FIXME
)

include_directories(
  SYSTEM
  ${MIRTEST_INCLUDE_DIRS}
  ${MIRSERVER_INCLUDE_DIRS}
  ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
)

add_executable(windowmanager_test ${MIR_WINDOW_MANAGER_TEST_SOURCES})

target_link_libraries(
  windowmanager_test

  unityapplicationplugin

  Qt5::Test

  -L${CMAKE_BINARY_DIR}/miral-qt/tests/framework
  qtmir-test-framework-static

  ${MIRTEST_LDFLAGS}
  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(WindowManager windowmanager_test)
