set(
  MIR_SURFACE_MANAGER_TEST_SOURCES
  mirsurfaceitem_test.cpp
  mirsurface_test.cpp
  ${MIRAL_QT_SOURCE_DIR}/src/common/debughelpers.cpp
)

include_directories(
  ${MIRAL_QT_SOURCE_DIR}/src/common
  ${MIRAL_QT_SOURCE_DIR}/src/modules
  ${MIRAL_QT_SOURCE_DIR}/src/platforms/mirserver
  ${MIRAL_QT_SOURCE_DIR}/tests/framework
  ${MIRSERVER_INCLUDE_DIRS}
  ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
)

add_executable(surfacemanager_test ${MIR_SURFACE_MANAGER_TEST_SOURCES})

target_link_libraries(
  surfacemanager_test

  unityapplicationplugin

  Qt5::Test

  -L${CMAKE_BINARY_DIR}/miral-qt/tests/framework
  qtmir-test-framework-static

  ${GTEST_BOTH_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(SurfaceManager surfacemanager_test)
